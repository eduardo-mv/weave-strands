/*
Title: "Multi Reader Single Writer Spin Lock"
File: Spinlock.h
Author(s): Eduardo Martínez Vidal

Abstract:
	Implements a spinlock that allows multiple readers to access a resource and a single writer at a time.
	Uses two atomics, one to control the resource access and the other to keep count of the amount of current readers.
	The style is done immitating the STL.
	A RAII container is supplied to lock the spinlock on either mode.
	On windows platform, the function cpu_relax is defined as YieldProcessor, which is another macro that translates to _mm_pause on x64
Update Log:

*/
#pragma once

#include <atomic>
#include <thread>

namespace weave {

//Calls a platform dependent equivalent of yield
void cpu_relax();

//Calls a platform dependent functionality assigning the current thread to the specified core number
void thread_core(uint64_t ideal, uint64_t coreMask);

//Calls a platform dependent functionality assigning the current thread the specified priority
void thread_priority(int priority);

class mrsw_spinlock {
	//Lock that controls write access
	std::atomic_flag lock = ATOMIC_FLAG_INIT;
	//Counter of the amount of readers currently reading the resource
	std::atomic_int readerCount {0};

public:
	void acquire_read() {
		//Wait for resource flag and claim it. Readers and Writers set this flag, which protects the atomic counter
		//The flag is released once the counter is increased
		while(lock.test_and_set(std::memory_order_acquire)) { cpu_relax(); }
		readerCount.fetch_add(1, std::memory_order_relaxed);
		lock.clear(std::memory_order_release);
	}

	void release_read() {
		//Decrement the reader counter, effectively indicating a reader has finished their job
		readerCount.fetch_sub(1, std::memory_order_relaxed);
	}

	void acquire_write() {
		//Wait for the resoruce flag to claim it and then wait for all readers to finish their jobs
		//No new readers can enter after the resource acquisition. We don't release the resource until explicitly told so
		while (lock.test_and_set(std::memory_order_relaxed)) { cpu_relax();	}
		while(readerCount.load(std::memory_order_acquire) > 0) { cpu_relax(); }
	}

	void release_write() {
		//Release the resource
		lock.clear(std::memory_order_release);
	}
};

class mrsw_readlock {
private:
	mrsw_spinlock &lock;
public:
	mrsw_readlock(mrsw_spinlock &lock) : lock(lock) {
		lock.acquire_read();
	}

	~mrsw_readlock() {
		lock.release_read();
	}
};

class mrsw_writelock {
private:
	mrsw_spinlock &lock;
public:
	mrsw_writelock(mrsw_spinlock &lock) : lock(lock) {
		lock.acquire_write();
	}

	~mrsw_writelock() {
		lock.release_write();
	}
};
	
}
