#pragma once

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

namespace weave::system::memory {

/**
 * Lightweight alternative to StreamingForge that only guarantees a thread-safe
 * producer/consumer queue. Producers enqueue payloads from any thread; a single
 * consumer drains the queue and forwards each payload to the backend.
 */
template<typename Backend>
concept QueueBackend = requires(
    Backend& backend,
    typename Backend::Payload payload) {
    typename Backend::Payload;
    { backend.UploadEntry(std::move(payload)) } -> std::convertible_to<std::pair<bool, std::string>>;
};

template<QueueBackend Backend>
class StreamingQueue {
public:
    using Payload = typename Backend::Payload;
    using TaskResult = std::pair<bool, std::string>;

    explicit StreamingQueue(Backend& backend)
        : backend(backend) {}

    StreamingQueue(StreamingQueue const&) = delete;
    StreamingQueue& operator=(StreamingQueue const&) = delete;

    StreamingQueue(StreamingQueue&&) noexcept = delete;
    StreamingQueue& operator=(StreamingQueue&&) noexcept = delete;

    void Enqueue(Payload payload) {
        {
            std::lock_guard lock(queueMutex);
            queue.emplace_back(std::move(payload));
        }
        queueCv.notify_one();
    }

    size_t ProcessStreamingQueue(size_t maxTasks,
                         std::chrono::milliseconds blockTimeout = std::chrono::milliseconds::zero()) {
        size_t processed = 0;
        while (maxTasks == 0 || processed < maxTasks) {
            auto task = ClaimStreamingTask(blockTimeout);
            if (!task.has_value()) {
                break;
            }

            [[maybe_unused]] auto result = backend.UploadEntry(std::move(*task));
            ++processed;
        }

        return processed;
    }

    size_t PeekQueuedCount() const {
        std::lock_guard lock(queueMutex);
        return queue.size();
    }

private:
    std::optional<Payload> ClaimStreamingTask(std::chrono::milliseconds blockTimeout) {
        std::unique_lock lock(queueMutex);
        if (!WaitForTask(lock, blockTimeout)) {
            return std::nullopt;
        }

        Payload payload = std::move(queue.front());
        queue.pop_front();
        return std::optional<Payload>(std::in_place, std::move(payload));
    }

    bool WaitForTask(std::unique_lock<std::mutex>& lock, std::chrono::milliseconds timeout) {
        if (!queue.empty()) {
            return true;
        }

        if (timeout == std::chrono::milliseconds::zero()) {
            return false;
        }

        if (timeout == std::chrono::milliseconds::max()) {
            queueCv.wait(lock, [&]() { return !queue.empty(); });
            return true;
        }

        return queueCv.wait_for(lock, timeout, [&]() { return !queue.empty(); });
    }

    Backend& backend;
    mutable std::mutex queueMutex;
    std::condition_variable queueCv;
    std::deque<Payload> queue;
};

} // namespace weave::system::memory
