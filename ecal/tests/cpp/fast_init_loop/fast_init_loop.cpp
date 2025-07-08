#include <gtest/gtest.h>

#include "ecal/ecal.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <future>
#include <random>

namespace {
class ECALRaiiInitializer {
public:
  ECALRaiiInitializer() {
    if (!eCAL::Initialize())
      throw std::runtime_error("failed to initialize eCAL");
  }

  ECALRaiiInitializer(const ECALRaiiInitializer &) = delete;
  ECALRaiiInitializer &operator=(const ECALRaiiInitializer &) = delete;
  ECALRaiiInitializer(ECALRaiiInitializer &&) = delete;
  ECALRaiiInitializer &operator=(ECALRaiiInitializer &&) = delete;

  ~ECALRaiiInitializer() {
    if (!eCAL::Finalize()) {
      std::cerr << "failed to finalize eCAL" << std::endl;
      std::terminate();
    }
  }
};

class ReceiverChecker {
public:
  explicit ReceiverChecker(const std::string &topic) : subscriber_(topic) {
    subscriber_.SetReceiveCallback(
        [this](const auto &, const auto &,
               const eCAL::SReceiveCallbackData &message) {
          this->onReceive(message);
        });
  }

  bool hasReceivedData() const { return hasReceivedData_; }

private:
  void onReceive(const eCAL::SReceiveCallbackData &message) {
    if (message.buffer_size > 0)
      hasReceivedData_.store(true);
    else
      throw std::runtime_error(
          "Received message with zero size, which is unexpected.");
  }

  eCAL::CSubscriber subscriber_;
  std::atomic<bool> hasReceivedData_{false};
};

std::string create_topic_name() {
  static std::random_device device;
  static std::mt19937 generator(device());
  static std::uniform_int_distribution<size_t> distribution(
      0, std::numeric_limits<size_t>::max());
  const size_t unique_id = distribution(generator);
  return std::string("test_topic_" + std::to_string(unique_id));
}

template <typename Message> class EcalLoopPublisher {
public:
  explicit EcalLoopPublisher(
      const std::vector<std::string> &topic_names,
      const std::chrono::milliseconds wait_until_ecal_is_ready)
      : wait_until_ecal_is_ready_(wait_until_ecal_is_ready) {
    std::transform(
        topic_names.begin(), topic_names.end(), std::back_inserter(publishers_),
        [](const std::string &topic) { return eCAL::CPublisher(topic); });
  }

  EcalLoopPublisher &operator=(const EcalLoopPublisher &publisher) = delete;
  EcalLoopPublisher(const EcalLoopPublisher &publisher) = delete;
  EcalLoopPublisher &operator=(EcalLoopPublisher &&publisher) = delete;
  EcalLoopPublisher(EcalLoopPublisher &&publisher) = delete;

  void start_publishing() {
    if (is_publishing_)
      throw std::logic_error("Publisher is already running.");
    is_publishing_ = true;
    publisher_future_ = std::async(
        std::launch::async, &EcalLoopPublisher::send_messages_in_a_loop, this);
  }

  void stop_publishing() {
    is_publishing_ = false;
    if (publisher_future_.valid())
      publisher_future_.wait();
  }

  ~EcalLoopPublisher() { stop_publishing(); }

private:
  void send_messages_in_a_loop() {
    char value = 0;
    Message data{};
    std::this_thread::sleep_for(wait_until_ecal_is_ready_);
    while (is_publishing_) {
      std::fill(data.begin(), data.end(), value);
      std::for_each(publishers_.begin(), publishers_.end(),
                    [&data](eCAL::CPublisher &publisher) {
                      publisher.Send(&data, sizeof(data));
                    });
      ++value;
    }
  }

  std::atomic<bool> is_publishing_{false};
  std::vector<eCAL::CPublisher> publishers_;
  std::future<void> publisher_future_;
  std::chrono::milliseconds wait_until_ecal_is_ready_{0};
};

class PublisherSubscriberSet {
public:
  explicit PublisherSubscriberSet(
      const size_t number_of_topics,
      const std::chrono::milliseconds &wait_until_ecal_is_ready) {
    std::vector<std::string> topic_names;
    std::generate_n(std::back_inserter(topic_names), number_of_topics,
                    create_topic_name);
    publishers = std::make_unique<EcalLoopPublisher<Message>>(
        topic_names, wait_until_ecal_is_ready);
    publishers->start_publishing();

    std::transform(topic_names.begin(), topic_names.end(),
                   std::back_inserter(subscribers),
                   [](const std::string &topic) {
                     return std::make_unique<ReceiverChecker>(topic);
                   });
  }

  ECALRaiiInitializer ecal_context_;
  using Message = std::array<char, 1000>;

  std::vector<std::unique_ptr<ReceiverChecker>> subscribers;
  std::unique_ptr<EcalLoopPublisher<Message>> publishers;
};

bool has_received(const std::unique_ptr<ReceiverChecker> &subscriber) {
  return subscriber->hasReceivedData();
};

void wait_until_one_ecal_topic_received(
    std::vector<std::unique_ptr<ReceiverChecker>> &subscribers,
    const std::chrono::milliseconds timeout) {
  using std::chrono::steady_clock;
  const auto wait_until = steady_clock::now() + timeout;

  while (steady_clock::now() < wait_until) {
    const bool any_topic_received =
        std::any_of(subscribers.begin(), subscribers.end(), has_received);
    if (any_topic_received)
      return;
  }
  throw std::runtime_error(
      "Timeout: No topic received data within the specified duration.");
}

void wait_until_all_ecal_topics_received(
    std::vector<std::unique_ptr<ReceiverChecker>> &subscribers,
    const std::chrono::milliseconds timeout) {
  using std::chrono::steady_clock;
  const auto start_time = steady_clock::now();
  const auto wait_until = start_time + timeout;

  while (steady_clock::now() < wait_until) {
    const bool all_topics_received =
        std::all_of(subscribers.begin(), subscribers.end(), has_received);
    if (all_topics_received) {
      using std::chrono::duration_cast;
      using std::chrono::milliseconds;
      const auto time_delta =
          duration_cast<milliseconds>(steady_clock::now() - start_time);
      std::cout << "  it took " << std::to_string(time_delta.count())
                << " milliseconds to receive all topics." << std::endl;
      return;
    }
  }

  const auto number_of_received_topics = std::count_if(
      std::begin(subscribers), std::end(subscribers), has_received);
  const auto number_of_missing_topics =
      subscribers.size() - number_of_received_topics;

  throw std::runtime_error(
      "Timeout: " + std::to_string(number_of_missing_topics) +
      " topic did not receive data within the specified duration.");
}
} // namespace

TEST(FastInitLoop, fifty_topics_one_hundret_iterations) {
  constexpr size_t iterations = 100;

  using std::chrono::milliseconds;
  const milliseconds wait_until_ecal_is_ready(2000);
  const milliseconds connect_timeout(3000);
  const milliseconds timeout(1000);

  for (size_t iteration = 0; iteration < iterations; ++iteration) {
    constexpr size_t number_of_topics = 500;
    PublisherSubscriberSet test_context{number_of_topics,
                                        wait_until_ecal_is_ready};
    wait_until_one_ecal_topic_received(test_context.subscribers,
                                       connect_timeout);
    EXPECT_NO_THROW(
        wait_until_all_ecal_topics_received(test_context.subscribers, timeout))
        << "Not all topics received data in loop " << iteration << ".";
  }
}