#include "GeometryAtlas.h"

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <limits>

using namespace weave;
using namespace weave::graphics;
using namespace weave::graphics::gl::gpu;
using namespace weave::opengl;

namespace {
size_t SafeByteSize(size_t count, size_t stride) {
	if (stride == 0 || count == 0)
		return 0;

	const size_t maxCount = std::numeric_limits<size_t>::max() / stride;
	if (count > maxCount) {
		return std::numeric_limits<size_t>::max();
	}
	return count * stride;
}
}

GeometryAtlas::~GeometryAtlas() = default;

void GeometryAtlas::ReserveBytes(size_t vertexBytes, size_t indexBytes) {
	if (vertexBytes > vertexBuffer.GetBufferByteSize()) {
		vertexBuffer.Create(nullptr, vertexBytes, BufferUsage::Write);
		vertexAllocator = weave::memory::RangeAllocator(vertexBytes);
	}

	if (indexBytes > indexBuffer.GetBufferByteSize()) {
		indexBuffer.Create(nullptr, indexBytes, BufferUsage::Write);
		indexAllocator = weave::memory::RangeAllocator(indexBytes);
	}
}

void GeometryAtlas::ReserveGeometry(size_t vertexCount, size_t indexCount, size_t vertexStrideBytes) {
	const size_t vertexBytes = SafeByteSize(vertexCount, vertexStrideBytes);
	// Default to 32-bit indices when estimating.
	constexpr size_t kIndexStride = sizeof(uint32_t);
	const size_t indexBytes = SafeByteSize(indexCount, kIndexStride);

	ReserveBytes(vertexBytes, indexBytes);
}

GeometryAtlas::StreamingTicket GeometryAtlas::StreamMesh(std::shared_ptr<const MeshData> meshData) {
	if (!meshData) {
		return {};
	}

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
			std::move(meshData),
			std::move(promise)
		});
	}

	streamingCv.notify_one();
	return StreamingTicket{
		handle,
		std::move(future)
	};
}

GeometryAtlas::StreamStatus GeometryAtlas::QueryStreamingStatus(Handle handle) const {
	return AccessEntryRecord(handle, [](EntryRecord const* entry) {
		return entry ? entry->status : StreamStatus::Unknown;
	});
}

std::optional<GeometryAtlas::EntryInfo> GeometryAtlas::QueryEntry(Handle handle) const {
	return AccessEntryRecord(handle, [](EntryRecord const* entry) {
		if(entry && entry->status == StreamStatus::Ready) {
			return std::optional{entry->info};
		}

		return std::optional<GeometryAtlas::EntryInfo>{};
	});
	
}

GeometryAtlas::StreamingTicket GeometryAtlas::RemoveMesh(Handle handle) {
	
	StreamingTicket ticket = AccessEntryRecord(handle, [&](EntryRecord *entry) {
		if(entry && entry->status != StreamStatus::Removing) {
			std::promise<StreamingResult> promise;
			auto future = promise.get_future();
			
			if(entry->status == StreamStatus::Pending || entry->status == StreamStatus::Failed) {
				std::erase_if(streamingQueue, [&](StreamingTask const& task) {
					return task.handle == handle;
				});

				*entry = EntryRecord{};
				freeSlots.push_back(handle.Slot());

				promise.set_value(StreamingResult {
					true, 
					""
				});
			}
			else {
				streamingQueue.emplace_back(StreamingTask{
					TaskType::Remove,
					handle,
					{},
					std::move(promise)
				});
			}

			entry->status = StreamStatus::Removing;

			return StreamingTicket{
				handle,
				std::move(future)
			};
		}

		return StreamingTicket{};
	});

	if(ticket.Valid()) {
		streamingCv.notify_one();
	}

	return ticket;
}

namespace {
template<typename Queue>
bool WaitForTask(std::unique_lock<std::mutex>& lock,
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
}

GeometryAtlas::StreamingTask GeometryAtlas::ClaimStreamingTask(std::chrono::milliseconds blockTimeout) {
	while(true) {
		std::unique_lock lock(streamingMutex);
		if (!WaitForTask(lock, streamingCv, streamingQueue, blockTimeout)) {
			return {};
		}

		StreamingTask task = std::move(streamingQueue.front());
		streamingQueue.pop_front();


		auto invalidHandleError = [&]{
			StreamingResult result;
			result.success = false;
			result.errorMessage = "Invalid target handle";
			try {
				task.promise.set_value(std::move(result));
			}
			catch (...) {
				// Promise might already be satisfied/destroyed; ignore.
			}
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

bool GeometryAtlas::ProcessUploadTask(GeometryAtlas::StreamingTask& task) {
	StreamingResult result;
	if (!task.meshData) {
		result.success = false;
		result.errorMessage = "Mesh data expired";
	} else {
		// TODO: actual GPU upload work.
		result.success = true;
	}

	bool ok = result.success;
	try {
		task.promise.set_value(std::move(result));
	}
	catch (...) {
		// Promise might already be satisfied/destroyed; ignore.
	}

	return ok;
}

bool GeometryAtlas::ProcessRemoveTask(GeometryAtlas::StreamingTask& task) {
	StreamingResult result;
	result.success = true;
	// TODO actual GPU removal work
	try {
		task.promise.set_value(std::move(result));
	}
	catch (...) {
		// Promise might already be satisfied/destroyed; ignore.
	}

	return true;
}


size_t GeometryAtlas::ProcessStreamingQueue(size_t maxJobs, std::chrono::milliseconds blockTimeout) {
	size_t jobsProcessed = 0;

	while (maxJobs == 0 || jobsProcessed < maxJobs) {
		StreamingTask task = ClaimStreamingTask(blockTimeout);
		if(!task.handle.IsValid()) {
			break;
		}
		
		bool resultOk{};

		if (task.type == TaskType::Upload) {
			resultOk = ProcessUploadTask(task);
		}
		else if (task.type == TaskType::Remove) {
			resultOk = ProcessRemoveTask(task);
		}

		AccessEntryRecord(task.handle, [&](GeometryAtlas::EntryRecord *entry){
			if(entry) {
				if(task.type == TaskType::Upload) {
					entry->status = resultOk ? StreamStatus::Ready : StreamStatus::Failed;
				}
				else if(task.type == TaskType::Remove){
					*entry = EntryRecord{};
					freeSlots.push_back(task.handle.Slot());
				}
			}
		});

		++jobsProcessed;
	}

	return jobsProcessed;
}

size_t GeometryAtlas::PeekStreamingQueue() const {
	std::lock_guard lock(streamingMutex);
	return streamingQueue.size();
}
