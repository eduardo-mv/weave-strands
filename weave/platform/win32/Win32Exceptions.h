/*
Win32 Exceptions handler
Win32 Support module to handle exceptions and create minidumps
*/


#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <string>
#include <vector>
#include <functional>

namespace weave {
namespace win32 {

	//Installs a Win32 exception handler that will dump minidump data. The dumpMode can be specified as:
	//0 = auto mini dump
	//1 = auto full dump
	//2 = Windows prompt user selected dump
	void InstallExceptionHandler(int dumpMode, std::function<LONG(_EXCEPTION_POINTERS* exInfo)> callback = nullptr);
	//Generates a memory dump using the Win32 Minidump facilities
	void CreateMinidump(bool createMiniDump, bool createFullDump);
}
}



