#include "tests/TestEntryPoints.h"

#include "weave/system/memory/StreamingForge.h"

#include <chrono>

namespace weave::tests::system::memory {

namespace wsm = weave::system::memory;

class DummyStreamingAtlasBackend {
public:
	struct Entry {
		int storedValue = 0;
	};

	struct Payload {
		int value = 0;
	};

	std::pair<bool, std::string> UploadEntry(Entry& entry, Payload&& payload) {
		entry.storedValue = payload.value;
		++uploadCount;
		return { true, {} };
	}

	std::pair<bool, std::string> RemoveEntry(Entry& entry) {
		entry.storedValue = -1;
		++removeCount;
		return { true, {} };
	}

	int uploadCount = 0;
	int removeCount = 0;
};

using DummyStreamingAtlas = wsm::StreamingForge<DummyStreamingAtlasBackend>;


TestReport TestStreamingForge() {
	TestReport report;
	DummyStreamingAtlasBackend atlasBackend;
	DummyStreamingAtlas backend(atlasBackend);

	auto ticket = backend.StreamPayload({42});
	report.Expect(ticket.Valid(), "Streaming ticket should be valid");
	report.Expect(backend.QueryStreamingStatus(ticket.handle) == DummyStreamingAtlas::StreamStatus::Pending,
		"Handle should report pending prior to processing");

	backend.ProcessStreamingQueue(0, std::chrono::milliseconds::zero());

	auto uploadResult = ticket.future.get();
	report.Expect(uploadResult.success, "Upload result should be successful");
	report.Expect(atlasBackend.uploadCount == 1, "Upload callback should have fired exactly once");

	auto entry = backend.QueryEntry(ticket.handle);
	report.Expect(entry.has_value(), "Entry should be available after upload");
	report.Expect(entry->storedValue == 42, "Entry should reflect uploaded payload");

	auto removeTicket = backend.RemovePayload(ticket.handle);
	report.Expect(removeTicket.Valid(), "Remove ticket should be valid");

	backend.ProcessStreamingQueue(0, std::chrono::milliseconds::zero());

	auto removeResult = removeTicket.future.get();
	report.Expect(removeResult.success, "Removal result should be successful");
	report.Expect(atlasBackend.removeCount == 1, "Remove callback should have fired exactly once");

	auto removedEntry = backend.QueryEntry(ticket.handle);
	report.Expect(!removedEntry.has_value(), "Entry should no longer exist after removal");

	return report;
}

} // namespace weave::tests::system::memory

