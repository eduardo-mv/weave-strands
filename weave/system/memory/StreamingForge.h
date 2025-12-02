#pragma once

#include <algorithm>
#include <chrono>
#include <concepts>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <future>
#include <limits>
#include <mutex>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace weave::system::memory {

/**
 * StreamingForge encapsulates the generic mechanics behind streaming payloads into
 * long-lived slots managed by an atlas-like object. The Atlas template parameter
 * must provide:
 *   using Entry;    // per-slot metadata structure
 *   using Payload;  // object queued for upload
 *
 * and the following member functions (invoked by the forge):
 *   StreamingResult UploadEntry(Entry& entry, Payload&& payload);
 *   StreamingResult RemoveEntry(Entry& entry);
 */
template<typename AtlasType>
concept Atlas = requires(
    AtlasType& atlas,
    typename AtlasType::Entry& entry,
    typename AtlasType::Payload& payload) {
    
    typename AtlasType::Entry;
    typename AtlasType::Payload;

    { atlas.UploadEntry(entry, std::move(payload)) } -> std::convertible_to<std::pair<bool, std::string>>;
    { atlas.RemoveEntry(entry) } -> std::convertible_to<std::pair<bool, std::string>>;
};

template<Atlas AtlasType>
class StreamingForge {
public:
    using Entry = typename AtlasType::Entry;
    using Payload = typename AtlasType::Payload;

    struct Handle {
        uint64_t value = InvalidValue;

        static constexpr uint64_t InvalidValue = std::numeric_limits<uint64_t>::max();

        static Handle FromParts(uint32_t generation, uint32_t slot) {
            Handle h{};
            h.value = (static_cast<uint64_t>(generation) << 32) | slot;
            return h;
        }

        uint32_t Generation() const {
            return static_cast<uint32_t>(value >> 32);
        }

        uint32_t Slot() const {
            return static_cast<uint32_t>(value & 0xffffffffu);
        }

        bool IsValid() const { return value != InvalidValue; }

        bool operator==(Handle const& other) const { return value == other.value; }
    };

    struct StreamingResult {
        bool success = false;
        std::string errorMessage;
    };

    struct StreamingTicket {
        Handle handle{};
        std::future<StreamingResult> future;

        bool Valid() const { return handle.IsValid(); }
    };

    enum class StreamStatus {
        Pending,
        Uploading,
        Ready,
        Failed,
        Removing,
        Unknown
    };

    StreamingForge(AtlasType &atlas)
        :atlas(atlas) {
    }

    ~StreamingForge() = default;

    StreamingTicket StreamPayload(Payload payload) {
        std::promise<StreamingResult> promise;
        auto future = promise.get_future();

        Handle handle;
        {
            std::lock_guard lock(streamingMutex);

            uint32_t slot = 0;
            if (!freeSlots.empty()) {
                slot = freeSlots.back();
                freeSlots.pop_back();
                entries[slot].generation++;
            } else {
                slot = static_cast<uint32_t>(entries.size());
                entries.emplace_back();
                entries[slot].generation = 1;
            }

            auto& entry = entries[slot];
            entry.status = StreamStatus::Pending;
            handle = Handle::FromParts(entry.generation, slot);

            streamingQueue.emplace_back(StreamingTask{
                TaskType::Upload,
                handle,
                std::optional<Payload>(std::in_place, std::move(payload)),
                std::move(promise)
            });
        }

        streamingCv.notify_one();
        return StreamingTicket{handle, std::move(future)};
    }

    StreamingTicket RemovePayload(Handle handle) {
        StreamingTicket ticket = AccessEntryRecord(handle, [&](EntryRecord* entry) {
            if (entry && entry->status != StreamStatus::Removing) {
                std::promise<StreamingResult> promise;
                auto future = promise.get_future();

                if (entry->status == StreamStatus::Pending || entry->status == StreamStatus::Failed) {
                    std::erase_if(streamingQueue, [&](StreamingTask const& task) {
                        return task.handle == handle;
                    });

                    ReleaseEntry(handle.Slot());

                    promise.set_value(StreamingResult{true, {}});
                } else {
                    streamingQueue.emplace_back(StreamingTask{
                        TaskType::Remove,
                        handle,
                        std::nullopt,
                        std::move(promise)
                    });
                }

                entry->status = StreamStatus::Removing;

                return StreamingTicket{handle, std::move(future)};
            }

            return StreamingTicket{};
        });

        if (ticket.Valid()) {
            streamingCv.notify_one();
        }

        return ticket;
    }

    StreamStatus QueryStreamingStatus(Handle handle) const {
        return AccessEntryRecord(handle, [](EntryRecord const* entry) {
            return entry ? entry->status : StreamStatus::Unknown;
        });
    }

    std::optional<Entry> QueryEntry(Handle handle) const {
        return AccessEntryRecord(handle, [](EntryRecord const* entry) -> std::optional<Entry> {
            if (entry && entry->status == StreamStatus::Ready) {
                return entry->info;
            }
            return std::nullopt;
        });
    }

    size_t ProcessStreamingQueue(size_t maxJobs, std::chrono::milliseconds blockTimeout) {
        size_t jobsProcessed = 0;

        while (maxJobs == 0 || jobsProcessed < maxJobs) {
            StreamingTask task = ClaimStreamingTask(blockTimeout);
            if (!task.handle.IsValid()) {
                break;
            }

            bool taskResult = false;

            if (task.type == TaskType::Upload) {
                taskResult = ProcessUploadTask(task);
            } else if (task.type == TaskType::Remove) {
                taskResult = ProcessRemoveTask(task);
            }

            AccessEntryRecord(task.handle, [&](EntryRecord* entry) {
                if (!entry) {
                    return;
                }

                if (task.type == TaskType::Upload) {
                    entry->status = taskResult ? StreamStatus::Ready : StreamStatus::Failed;
                } else if (task.type == TaskType::Remove) {
                    ReleaseEntry(task.handle.Slot());
                }
            });

            ++jobsProcessed;
        }

        return jobsProcessed;
    }

    size_t PeekStreamingQueue() const {
        std::lock_guard lock(streamingMutex);
        return streamingQueue.size();
    }

private:
    enum class TaskType {
        Upload,
        Remove
    };

    struct EntryRecord {
        StreamStatus status = StreamStatus::Unknown;
        uint32_t generation = 0;
        Entry info{};
    };

    struct StreamingTask {
        TaskType type = TaskType::Upload;
        Handle handle;
        std::optional<Payload> payload;
        std::promise<StreamingResult> promise;
    };

    template<typename Visitor>
    auto AccessEntryRecord(Handle handle, Visitor&& visitor) {
        return AccessEntryRecordImpl(*this, handle, std::forward<Visitor>(visitor));
    }

    template<typename Visitor>
    auto AccessEntryRecord(Handle handle, Visitor&& visitor) const {
        return AccessEntryRecordImpl(*this, handle, std::forward<Visitor>(visitor));
    }

    template<typename Self, typename Visitor>
    static auto AccessEntryRecordImpl(Self& self, Handle handle, Visitor&& visitor) {
        using EntryPtr = std::conditional_t<std::is_const_v<Self>, EntryRecord const*, EntryRecord*>;

        if (!handle.IsValid()) {
            return std::forward<Visitor>(visitor)(static_cast<EntryPtr>(nullptr));
        }

        std::lock_guard lock(self.streamingMutex);
        const auto slot = handle.Slot();
        if (slot >= self.entries.size()) {
            return std::forward<Visitor>(visitor)(static_cast<EntryPtr>(nullptr));
        }

        auto* entry = &self.entries[slot];
        if (entry->generation == 0 || entry->generation != handle.Generation()) {
            return std::forward<Visitor>(visitor)(static_cast<EntryPtr>(nullptr));
        }

        return std::forward<Visitor>(visitor)(static_cast<EntryPtr>(entry));
    }

    StreamingTask ClaimStreamingTask(std::chrono::milliseconds blockTimeout) {
        while (true) {
            std::unique_lock lock(streamingMutex);
            if (!WaitForTask(lock, streamingCv, streamingQueue, blockTimeout)) {
                return {};
            }

            StreamingTask task = std::move(streamingQueue.front());
            streamingQueue.pop_front();

            auto invalidHandleError = [&] {
                StreamingResult result;
                result.success = false;
                result.errorMessage = "Invalid target handle";
                FulfillPromise(task.promise, std::move(result));
            };

            const auto slot = task.handle.Slot();
            if (slot >= entries.size()) {
                invalidHandleError();
                continue;
            }

            auto& entry = entries[slot];
            if (entry.generation == 0 || entry.generation != task.handle.Generation()) {
                invalidHandleError();
                continue;
            }

            if (task.type == TaskType::Upload) {
                entry.status = StreamStatus::Uploading;
            } else if (task.type == TaskType::Remove) {
                entry.status = StreamStatus::Removing;
            }

            return task;
        }
    }

    bool ProcessUploadTask(StreamingTask& task) {
        StreamingResult result;
        auto entryPtr = AccessEntryRecord(task.handle, [](EntryRecord* entry) { return entry; });
        if (!entryPtr) {
            result.success = false;
            result.errorMessage = "Entry expired";
            FulfillPromise(task.promise, std::move(result));
            return false;
        }

        if (!task.payload) {
            result.success = false;
            result.errorMessage = "Missing payload";
            FulfillPromise(task.promise, std::move(result));
            return false;
        }

        auto payload = std::move(*task.payload);
        task.payload.reset();

        std::tie(result.success, result.errorMessage) = atlas.UploadEntry(entryPtr->info, std::move(payload));
        StreamingResult deliver = result;
        FulfillPromise(task.promise, std::move(deliver));
        return result.success;
    }

    bool ProcessRemoveTask(StreamingTask& task) {
        StreamingResult result;
        auto entryPtr = AccessEntryRecord(task.handle, [](EntryRecord* entry) { return entry; });
        if (!entryPtr) {
            result.success = false;
            result.errorMessage = "Entry expired";
            FulfillPromise(task.promise, std::move(result));
            return false;
        }

        std::tie(result.success, result.errorMessage) = atlas.RemoveEntry(entryPtr->info);
        ReleaseEntry(task.handle.Slot());
        StreamingResult deliver = result;
        FulfillPromise(task.promise, std::move(deliver));
        return result.success;
    }

    void ReleaseEntry(uint32_t slot) {
        if (slot >= entries.size()) {
            return;
        }

        entries[slot] = EntryRecord{};
        freeSlots.push_back(slot);
    }

    static void FulfillPromise(std::promise<StreamingResult>& promise, StreamingResult&& result) {
        try {
            promise.set_value(std::move(result));
        } catch (...) {
            // Promise might already be satisfied/destroyed; ignore.
        }
    }

    template<typename Queue>
    static bool WaitForTask(std::unique_lock<std::mutex>& lock,
                            std::condition_variable& cv,
                            Queue& queue,
                            std::chrono::milliseconds timeout) {
        if (!queue.empty()) {
            return true;
        }

        if (timeout == std::chrono::milliseconds::zero()) {
            return false;
        }

        if (timeout == std::chrono::milliseconds::max()) {
            cv.wait(lock, [&queue]() { return !queue.empty(); });
            return true;
        }

        return cv.wait_for(lock, timeout, [&queue]() { return !queue.empty(); });
    }

    AtlasType& atlas;
    std::vector<EntryRecord> entries;
    std::vector<uint32_t> freeSlots;

    mutable std::mutex streamingMutex;
    std::condition_variable streamingCv;
    std::deque<StreamingTask> streamingQueue;
};

} // namespace weave::system::memory
