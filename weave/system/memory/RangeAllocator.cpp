#include "RangeAllocator.h"

#include <algorithm>

using namespace weave::memory;

RangeAllocator::RangeAllocator(size_t initialSize) : capacity(initialSize) {
    if (initialSize > 0) {
        freeList.push_back({0, initialSize});
    }
}

size_t RangeAllocator::Allocate(size_t size) {
    if (size == 0) {
        return static_cast<size_t>(-1);
    }

    for (auto it = freeList.begin(); it != freeList.end(); ++it) {
        if (it->size >= size) {
            const size_t offset = it->offset;
            it->offset += size;
            it->size -= size;
            if (it->size == 0) {
                freeList.erase(it);
            }
            return offset;
        }
    }

    return static_cast<size_t>(-1);
}

void RangeAllocator::Free(size_t offset, size_t size) {
    if (size == 0) {
        return;
    }

    InsertFreeRange({offset, size});
    Coalesce();
}

void RangeAllocator::Grow(size_t additionalBytes) {
    if (additionalBytes == 0) {
        return;
    }

    InsertFreeRange({capacity, additionalBytes});
    capacity += additionalBytes;
    Coalesce();
}

size_t RangeAllocator::FreeBytes() const {
    size_t total = 0;
    for (auto const& range : freeList) {
        total += range.size;
    }
    return total;
}

void RangeAllocator::InsertFreeRange(Range range) {
    auto it = std::lower_bound(freeList.begin(), freeList.end(), range,
        [](Range const& a, Range const& b) {
            return a.offset < b.offset;
        });
    freeList.insert(it, range);
}

void RangeAllocator::Coalesce() {
    if (freeList.empty()) {
        return;
    }

    std::vector<Range> merged;
    merged.reserve(freeList.size());

    Range current = freeList.front();
    for (size_t i = 1; i < freeList.size(); ++i) {
        Range const& next = freeList[i];
        if (current.offset + current.size == next.offset) {
            current.size += next.size;
        } else {
            merged.push_back(current);
            current = next;
        }
    }

    merged.push_back(current);
    freeList.swap(merged);
}
