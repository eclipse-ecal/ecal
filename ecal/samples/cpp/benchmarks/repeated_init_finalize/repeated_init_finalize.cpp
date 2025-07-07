// COMPANY:   Continental Automotive
// COMPONENT: Eco

#include <gtest/gtest.h>

#include "ecal/ecal.h"

#include <algorithm>
#include <chrono>
#include <future>

struct ECALRaiiInitializer {
  ECALRaiiInitializer() {
    if (!eCAL::Initialize())
      throw std::runtime_error("failed to initialize eCAL");
  }

  ~ECALRaiiInitializer() {
    if (!eCAL::Finalize()) {
      std::cerr << "failed to finalize eCAL" << std::endl;
      std::terminate();
    }
  }
};


class eCALLogSubscriber {
public:
  explicit eCALLogSubscriber(const std::string &topic) : subscriber_(topic) {
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

std::vector<std::string> createTopicNames(const size_t numberOfTopics) {
  std::vector<std::string> topics;
  topics.reserve(numberOfTopics);
  static size_t index = 0;
  std::generate_n(std::back_inserter(topics), numberOfTopics, []() mutable {
    index += 1;
    return std::string("test_topic_" + std::to_string(index));
  });
  return topics;
}

template <typename Message> class EcalLoopPublisher {
public:
  explicit EcalLoopPublisher(const std::vector<std::string> &topicNames) {
    std::transform(
        topicNames.begin(), topicNames.end(), std::back_inserter(publishers_),
        [](const std::string &topic) { return eCAL::CPublisher(topic); });
  }

  void startPublishing() {
    if (isPublishing_)
      throw std::logic_error("Publisher is already running.");
    isPublishing_ = true;
    publisherFuture_ = std::async(
        std::launch::async, &EcalLoopPublisher::sendMessagesInALoop, this);
  }

  void stopPublishing() {
    isPublishing_ = false;
    if (publisherFuture_.valid())
      publisherFuture_.wait();
  }

  ~EcalLoopPublisher() { stopPublishing(); }

private:
  void sendMessagesInALoop() {
    char value = 0;
    Message data{};
    while (isPublishing_) {
      std::fill(data.begin(), data.end(), value);
      std::for_each(publishers_.begin(), publishers_.end(),
                    [&data](eCAL::CPublisher &publisher) {
                      publisher.Send(&data, sizeof(data));
                    });
      ++value;
    }
  }

  std::atomic<bool> isPublishing_{false};
  std::vector<eCAL::CPublisher> publishers_;
  std::future<void> publisherFuture_;
};


class PublisherSubscriberSet {
public:
  explicit PublisherSubscriberSet(const size_t numberOfTopics) {
    const auto topics = createTopicNames(numberOfTopics);
    publishers_ = std::make_unique<EcalLoopPublisher<Message>>(topics);
    publishers_->startPublishing();

    std::transform(topics.begin(), topics.end(),
                   std::back_inserter(subscribers_),
                   [](const std::string &topic) {
                     return std::make_unique<eCALLogSubscriber>(topic);
                   });
  }

  ECALRaiiInitializer ecal_context_{};
  using Message = std::array<char, 1000>;

  std::vector<std::unique_ptr<eCALLogSubscriber>> subscribers_;
  std::unique_ptr<EcalLoopPublisher<Message>> publishers_;
};

void waitUntilOneEcalTopicReceived(
    std::vector<std::unique_ptr<eCALLogSubscriber>> &subscribers,
    const std::chrono::milliseconds timeout) {
  using std::chrono::steady_clock;
  const auto wait_until = steady_clock::now() + timeout;
  while (steady_clock::now() < wait_until) {
    const bool anyTopicReceived = std::any_of(
        subscribers.begin(), subscribers.end(),
        [](auto &subscriber) { return subscriber->hasReceivedData(); });
    if (anyTopicReceived)
      return;
  }
  throw std::runtime_error(
      "Timeout: No topic received data within the specified duration.");
}

void waitUntilAllEcalTopicsReceived(
    std::vector<std::unique_ptr<eCALLogSubscriber>> &subscribers,
    const std::chrono::milliseconds timeout) {
  using std::chrono::steady_clock;
  const auto waitUntil = steady_clock::now() + timeout;

  while (steady_clock::now() < waitUntil) {
    const bool allTopicsReceived = std::all_of(
        subscribers.begin(), subscribers.end(),
        [](auto &subscriber) { return subscriber->hasReceivedData(); });
    if (allTopicsReceived)
      return;
  }
  throw std::runtime_error(
      "Timeout: Not all topics received data within the specified duration.");
}

TEST(MultipleSubscriberTest,
     SWT_ECAL_SUBSCRIBER_CHECKMULTIPLESUBSCRIBERv1__each) {
  constexpr size_t iterations = 100;
  const std::chrono::milliseconds connect_timeout(3000);
  const std::chrono::milliseconds timeout(1000);
  for (size_t iteration = 0; iteration < iterations; ++iteration) {
    constexpr size_t numberOfTopics = 50;
    PublisherSubscriberSet testContext{numberOfTopics};
    waitUntilOneEcalTopicReceived(testContext.subscribers_, connect_timeout);
    EXPECT_NO_THROW(
        waitUntilAllEcalTopicsReceived(testContext.subscribers_, timeout))
        << "Not all topics received data in loop " << iteration << ".";
  }
}