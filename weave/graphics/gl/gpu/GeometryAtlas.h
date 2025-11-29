#pragma once

#include "weave/graphics/core/MeshData.h"
#include "weave/graphics/gl/resources/Buffer.h"
#include "weave/system/memory/RangeAllocator.h"

#include <chrono>
#include <future>
#include <limits>
#include <memory>
#include <optional>
#include <deque>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <condition_variable>
#include <mutex>

namespace weave::graphics::gl::gpu {

class GeometryAtlas {
public:
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

    struct EntryInfo {
        // Placeholder for offsets/size metadata; will be defined later.
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

private:
    struct EntryRecord {
        StreamStatus status = StreamStatus::Unknown;
        uint32_t generation = 0;
        // Placeholder for offsets and sizes once uploaded.
        EntryInfo info{};
    };

    enum class TaskType {
        Upload,
        Remove
    };

    struct StreamingTask {
        TaskType type = TaskType::Upload;
        Handle handle;
        std::shared_ptr<const MeshData> meshData;
        std::promise<StreamingResult> promise;
    };

    GeometryAtlas() = default;
    GeometryAtlas(GeometryAtlas const&) = delete;
    GeometryAtlas(GeometryAtlas&&) noexcept = delete;
    GeometryAtlas& operator=(GeometryAtlas const&) = delete;
    GeometryAtlas& operator=(GeometryAtlas&&) noexcept = delete;
    ~GeometryAtlas();

    StreamingTicket StreamMesh(std::shared_ptr<const MeshData> meshData);
    StreamStatus QueryStreamingStatus(Handle handle) const;
    StreamingTicket RemoveMesh(Handle handle);
    
    std::optional<EntryInfo> QueryEntry(Handle handle) const;
    
    void ReserveBytes(size_t vertexBytes, size_t indexBytes);
    void ReserveGeometry(size_t vertexCount, size_t indexCount, size_t vertexStrideBytes);
    
    // Processes queued streaming work.
    // maxJobs == 0 means process all pending.
    // blockTimeout == duration::zero() -> non-blocking, infinite duration -> wait indefinitely.
    size_t ProcessStreamingQueue(size_t maxJobs, std::chrono::milliseconds blockTimeout);
    
    size_t PeekStreamingQueue() const;

private:
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
    
    StreamingTask ClaimStreamingTask(std::chrono::milliseconds blockTimeout);
    bool ProcessUploadTask(StreamingTask& task);
    bool ProcessRemoveTask(StreamingTask& task);

    weave::opengl::Buffer vertexBuffer;
    weave::opengl::Buffer indexBuffer;
    weave::memory::RangeAllocator vertexAllocator;
    weave::memory::RangeAllocator indexAllocator;

    std::vector<EntryRecord> entries;
    std::vector<uint32_t> freeSlots;

    mutable std::mutex streamingMutex;
    std::condition_variable streamingCv;
    std::deque<StreamingTask> streamingQueue;
};

}
