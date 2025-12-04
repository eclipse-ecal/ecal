/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/

#include <ecal/ecal.h>

// Per-publisher / per-subscriber configuration (transport layers etc.)
#include <ecal/config/publisher.h>
#include <ecal/config/subscriber.h>

#include <benchmark/benchmark.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

// ------------------------------------------------------------------------
// Benchmark parameters
// ------------------------------------------------------------------------

constexpr int registration_delay_ms = 2000;

constexpr int range_multiplier = 1 << 3;
constexpr int range_start = 1;
constexpr int range_limit = 1 << 21;

// 2nd benchmark argument: transport layer
enum class TransportLayer : int
{
    Shm = 0,
    Udp = 1,
    Tcp = 2,
};

inline const char* TransportLayerName(TransportLayer layer)
{
    switch (layer)
    {
    case TransportLayer::Shm: return "shm";
    case TransportLayer::Udp: return "udp";
    case TransportLayer::Tcp: return "tcp";
    }
    return "unknown";
}

// Generate (payload_size, transport_layer) argument tuples.
static void TransportAndSizeArgs(benchmark::internal::Benchmark* b)
{
    for (int layer = static_cast<int>(TransportLayer::Shm);
        layer <= static_cast<int>(TransportLayer::Tcp);
        ++layer)
    {
        for (int size = range_start; size <= range_limit; size *= range_multiplier)
        {
            b->Args({ size, layer });
        }
    }
}

// ------------------------------------------------------------------------
// Payload generator (deterministic for reproducibility)
// ------------------------------------------------------------------------

char gen()
{
    // Fixed seed -> deterministic content
    static std::mt19937 engine(0x12345678u);
    static std::uniform_int_distribution<int> distr(0, 255);
    return static_cast<char>(distr(engine));
}

// ------------------------------------------------------------------------
// Helper: Create fixed eCAL process configuration
// ------------------------------------------------------------------------
// Uses a local eCAL::Configuration object, so the run does not depend on
// any external ecal.yaml files / environment.
inline eCAL::Configuration MakeFixedEcalConfiguration()
{
    eCAL::Configuration config;
    config.registration.local.transport_type = eCAL::Registration::Local::eTransportType::shm;
    // NOTE: We intentionally *do not* call config.InitFromConfig()
    // so we stay with eCAL's compiled-in defaults only.
    return config;
}

// ------------------------------------------------------------------------
// Helper: Create per-publisher / per-subscriber transport config
// ------------------------------------------------------------------------

struct PubSubConfig
{
    eCAL::Publisher::Configuration  pub_cfg;
    eCAL::Subscriber::Configuration sub_cfg;
};

inline PubSubConfig MakeTransportConfig(TransportLayer layer)
{
    PubSubConfig cfg;

    // Start with all layers disabled and selectively enable one.
    cfg.pub_cfg.layer.shm.enable = false;
    cfg.pub_cfg.layer.udp.enable = false;
    cfg.pub_cfg.layer.tcp.enable = false;

    cfg.sub_cfg.layer.shm.enable = false;
    cfg.sub_cfg.layer.udp.enable = false;
    cfg.sub_cfg.layer.tcp.enable = false;

    switch (layer)
    {
    case TransportLayer::Shm:
        cfg.pub_cfg.layer.shm.enable = true;
        cfg.sub_cfg.layer.shm.enable = true;
        break;
    case TransportLayer::Udp:
        cfg.pub_cfg.layer.udp.enable = true;
        cfg.sub_cfg.layer.udp.enable = true;
        break;
    case TransportLayer::Tcp:
        cfg.pub_cfg.layer.tcp.enable = true;
        cfg.sub_cfg.layer.tcp.enable = true;
        break;
    }

    return cfg;
}

// ------------------------------------------------------------------------
// SEND-ONLY BENCHMARK
// ------------------------------------------------------------------------
namespace Send {

    void BM_eCAL_Send(benchmark::State& state)
    {
        const size_t payload_size = static_cast<size_t>(state.range(0));
        const auto   layer = static_cast<TransportLayer>(state.range(1));

        // Label row in benchmark output with transport
        state.SetLabel(TransportLayerName(layer));

        // Create payload
        std::vector<char> content_vector(payload_size);
        std::generate(content_vector.begin(), content_vector.end(), gen);
        const char* content_addr = content_vector.data();

        // Fixed process configuration (no external ecal.yaml)
        auto process_config = MakeFixedEcalConfiguration();
        eCAL::Initialize(process_config, "Benchmark_Send", eCAL::Init::Default);

        // Per-publisher / per-subscriber configuration for this transport
        auto pubsub_cfg = MakeTransportConfig(layer);

        // Publisher
        eCAL::CPublisher publisher("benchmark_topic", {}, pubsub_cfg.pub_cfg);

        // Receiver thread (dummy subscriber that just keeps the connection alive)
        std::atomic<bool> stop_receiver{ false };
        std::thread receiver_thread([&]() {
            eCAL::CSubscriber subscriber("benchmark_topic", {}, pubsub_cfg.sub_cfg);

            subscriber.SetReceiveCallback(
                [](const eCAL::STopicId&,
                    const eCAL::SDataTypeInformation&,
                    const eCAL::SReceiveCallbackData&) {
                        // Intentionally empty: we only need a subscription
                });

            while (!stop_receiver.load(std::memory_order_relaxed))
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            });

        // Wait for registration / matching
        std::this_thread::sleep_for(std::chrono::milliseconds(registration_delay_ms));
        while (!publisher.GetSubscriberCount() > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

        // Benchmarked section: Send data
        for (auto _ : state)
        {
            publisher.Send(content_addr, payload_size);
        }

        // Report bytes processed (for bytes/second metrics)
        state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) *
            static_cast<int64_t>(payload_size));

        // Shutdown subscriber thread & eCAL
        stop_receiver.store(true, std::memory_order_relaxed);
        receiver_thread.join();

        eCAL::Finalize();
    }

    // Register benchmark
    BENCHMARK(BM_eCAL_Send)
        ->Apply(TransportAndSizeArgs)
        ->UseRealTime();

} // namespace Send

// ------------------------------------------------------------------------
// SEND + RECEIVE BENCHMARK
// ------------------------------------------------------------------------
namespace Send_and_Receive {

    std::mutex              mtx;
    std::condition_variable convar;
    bool                    msg_received = false;

    void BM_eCAL_Send_and_Receive(benchmark::State& state)
    {
        const size_t payload_size = static_cast<size_t>(state.range(0));
        const auto   layer = static_cast<TransportLayer>(state.range(1));

        state.SetLabel(TransportLayerName(layer));

        std::vector<char> content_vector(payload_size);
        std::generate(content_vector.begin(), content_vector.end(), gen);
        const char* content_addr = content_vector.data();

        auto process_config = MakeFixedEcalConfiguration();
        eCAL::Initialize(process_config, "Benchmark_SendRecv", eCAL::Init::Default);

        auto pubsub_cfg = MakeTransportConfig(layer);

        eCAL::CPublisher publisher("benchmark_topic", {}, pubsub_cfg.pub_cfg);

        std::atomic<bool> stop_receiver{ false };
        std::thread receiver_thread([&stop_receiver, &pubsub_cfg]() {
            eCAL::CSubscriber subscriber("benchmark_topic", {}, pubsub_cfg.sub_cfg);

            subscriber.SetReceiveCallback(
                [](const eCAL::STopicId&,
                    const eCAL::SDataTypeInformation&,
                    const eCAL::SReceiveCallbackData&) {
                        {
                            std::lock_guard<std::mutex> lock(mtx);
                            msg_received = true;
                        }
                        convar.notify_one();
                });

            while (!stop_receiver.load(std::memory_order_relaxed))
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            });

        std::this_thread::sleep_for(std::chrono::milliseconds(registration_delay_ms));
        while (!publisher.GetSubscriberCount() > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        auto timeout = std::chrono::seconds(2);
        for (auto _ : state)
        {
            {
                std::lock_guard<std::mutex> lock(mtx);
                msg_received = false;
            }

            publisher.Send(content_addr, payload_size);

            std::unique_lock<std::mutex> lock(mtx);
            if (!convar.wait_for(lock, timeout, [] { return msg_received; }))
            {
                state.SkipWithError("Timeout waiting for message");
                break;
            }
        }

        state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) *
            static_cast<int64_t>(payload_size));

        stop_receiver.store(true, std::memory_order_relaxed);
        receiver_thread.join();

        eCAL::Finalize();
    }

    BENCHMARK(BM_eCAL_Send_and_Receive)
        ->Apply(TransportAndSizeArgs)
        ->UseRealTime();

} // namespace Send_and_Receive

// ------------------------------------------------------------------------
// Benchmark execution
// ------------------------------------------------------------------------

BENCHMARK_MAIN();
