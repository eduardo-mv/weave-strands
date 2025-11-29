#pragma once

#include <cstddef>
#include <vector>

namespace weave::memory {

class RangeAllocator {
public:
    struct Range {
        size_t offset = 0;
        size_t size = 0;
    };

    RangeAllocator() = default;
    explicit RangeAllocator(size_t initialSize);

    size_t Allocate(size_t size);
    void Free(size_t offset, size_t size);
    void Grow(size_t additionalBytes);

    size_t Capacity() const { return capacity; }
    size_t FreeBytes() const;

private:
    void InsertFreeRange(Range range);
    void Coalesce();

    size_t capacity = 0;
    std::vector<Range> freeList;
};

}
