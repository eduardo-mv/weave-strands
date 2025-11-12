#include "Spinlock.h"
#include <thread>

#ifdef _MSC_FULL_VER
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

void weave::cpu_relax() {
#ifdef _MSC_FULL_VER
	YieldProcessor();
#else
	std::this_thread::yield();
#endif
}

//Calls a platform dependent functionality assigning the current thread to the specified core number
void weave::thread_core(uint64_t ideal, uint64_t coreMask) {
#ifdef _MSC_FULL_VER
	SetThreadAffinityMask(GetCurrentThread(), coreMask);
	SetThreadIdealProcessor(GetCurrentThread(), uint32_t(ideal));
#elif NN_NINTENDO_SDK
	nn::os::SetThreadCoreMask(nn::os::GetCurrentThread(), uint32_t(ideal % 3), coreMask & nn::os::GetThreadAvailableCoreMask());
#else
	(void)ideal;
	(void)coreMask;
#endif
}

//Calls a platform dependent functionality assigning the current thread the specified priority
void weave::thread_priority(int priority) {
#ifdef _MSC_FULL_VER
	SetThreadPriority(GetCurrentThread(), priority);
#elif NN_NINTENDO_SDK
	nn::os::ChangeThreadPriority(nn::os::GetCurrentThread(), tdata->priority);
#else
	(void)priority;
#endif
}
