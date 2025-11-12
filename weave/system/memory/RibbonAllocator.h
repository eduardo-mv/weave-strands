/*
Round-ribbon aligned-memory allocator

Round ribbon allocator. Memory is allocated from a fixed sized pool that wraps around when it reaches the end.
This allocator must only be used under controlled circumstances where it's guaranteed by the implementation that memory blocks will not be held
for too long and released before the ribbon wraps around and overwrites a given block.
Memory from the ribbon cannot be freed.
Intended to be used on a single thread. This is not safe for multithraded.

*/
#pragma once

#include <memory>

namespace weave {

class RibbonAllocator {
private:
	//The memory block starting address
	uintptr_t memory;
	//The first free point
	uintptr_t marker;
	//The last valid point on the ribbon (size = last - memory)
	uintptr_t last;

public:
	RibbonAllocator(size_t stackByteSize);
	~RibbonAllocator();

	//Reserve aligned bytes from from stack. Alignment must be a power of 2. Returns a pointer to the start of the block
	void* Allocate(size_t byteSize, size_t align = 1);
	//Allocate with a template function supplying the optional ctor arguments
	template<typename T, size_t align = 1, typename ...Args>
	T* AllocateNew(Args ...args);
	//Allocate multiple instances with a template function supplying the optional ctor arguments
	template<typename T, size_t align = 1, typename ...Args>
	T* AllocateMany(size_t n, Args ...args);

	//Returns the size of the stack
	size_t MemSize() const;
};

template<typename T, size_t align, typename ...Args>
inline T * RibbonAllocator::AllocateNew(Args ...args) {
	return new (Allocate(sizeof(T), align)) T (args...);
}

template<typename T, size_t align, typename ...Args>
inline T * RibbonAllocator::AllocateMany(size_t n, Args ...args) {
	T *addr = reinterpret_cast<T*>(Allocate(sizeof(T) * n, align));
	for(size_t i = 0; i < n; ++i) {
		new (&addr[i]) T(args...);
	}
	
	return addr;
}

}
