#include "RibbonAllocator.h"
#include <cassert>

using namespace weave;

weave::RibbonAllocator::RibbonAllocator(size_t stackByteSize) {
	//Use malloc to allocate the requested memory block
	memory = uintptr_t(std::malloc(stackByteSize));
	//Store the starting marker
	marker = memory;
	//The last memory point
	last = memory + stackByteSize;
}

weave::RibbonAllocator::~RibbonAllocator() {
	std::free(reinterpret_cast<void*>(memory));
}

//Reserve bytes from from stack. Returns a pointer to the start of the block
void * weave::RibbonAllocator::Allocate(size_t byteSize, size_t align) {
	//Assert power of two alignment and enough size
	assert((align & (align - 1)) == 0);
	assert(byteSize + align < size_t(last - memory));
	//Wrap the marker around if we cannot provide a contiguous block from the current position
	if(marker + byteSize + align >= last)
		marker = memory;

	//Find the alignment difference and adjustment needed
	uintptr_t excess = (marker & (align - 1));
	uintptr_t adjust = align - excess;
	//Adjust the target address
	void *addr = reinterpret_cast<void*>(marker + adjust);
	//Store the adjustment on the previous byte
	reinterpret_cast<uint8_t*>(addr)[-1] = static_cast<uint8_t>(adjust);
	//Reserve enough space to keep the block aligned with the metadata 
	marker += byteSize + align;

	return addr;
}

//Returns the size of the stack
size_t weave::RibbonAllocator::MemSize() const {
	return size_t(last - memory);
}

