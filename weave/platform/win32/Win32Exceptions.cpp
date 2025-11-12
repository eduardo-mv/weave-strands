#ifdef _WIN32
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning(disable: 4996) //4996: Deprecated wstring_convert in C++17
#endif

#include "Win32Exceptions.h"
#include <sstream>
#include <thread>
#include <future>

#include <dbghelp.h>
#ifdef _MSC_VER
#pragma comment(lib,"DbgHelp.lib")
#endif
#include "weave/system/utf/Utf.h"

//Minidump global configuration
namespace {
	int dumpMode = 0;
	std::function<LONG(_EXCEPTION_POINTERS* exInfo)> userCallback;
}

//Default exception handler
LONG Win32FaultHandler(_EXCEPTION_POINTERS *  exInfo) {
	//Use the mini dump facilities and create two dumps
	//auto callerID = GetCurrentThreadId();
	bool createMiniDump = (dumpMode >= 0);
	bool createFullDump = (dumpMode >= 1);

	if(dumpMode >= 2) {
		std::stringstream ss;
		switch(exInfo->ExceptionRecord->ExceptionCode) {
		case EXCEPTION_ACCESS_VIOLATION:
			ss << "Access violation";
			break;
		case EXCEPTION_DATATYPE_MISALIGNMENT:
			ss << "Data missalignment";
			break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			ss << "Integer divide by 0";
			break;
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			ss << "Float divide by 0";
			break;
		case EXCEPTION_STACK_OVERFLOW:
			ss << "Stack overflow";
			break;
		default:
			ss << "Unknown";
			break;
		}

		ss << " at address " << exInfo->ExceptionRecord->ExceptionAddress;

		ss << ".\n\nA snapshot of the program state can be generated for debugging.\n\nYes: Create full dump\nNo: Create mini dump\nCancel: Ignore this error and hope it never happens again";

		ChangeDisplaySettings(nullptr, 0);

		//Message prompt the user about the exception
		auto ret = MessageBoxW(NULL, weave::utf::Utf8toUtf16(ss.str()).c_str(), L"Application Exception", MB_YESNOCANCEL | MB_TOPMOST | MB_SYSTEMMODAL | MB_ICONEXCLAMATION);
		createMiniDump = (ret == 6 || ret == 7);
		createFullDump = (ret == 6);
	}

	weave::win32::CreateMinidump(createMiniDump, createFullDump);

	/*if(want to continue)
	{
	exInfo->ContextRecord->Eip++;
	return EXCEPTION_CONTINUE_EXECUTION;
	}
	*/

	if (userCallback)
		return userCallback(exInfo);
	else
		return EXCEPTION_EXECUTE_HANDLER;

	//return EXCEPTION_CONTINUE_SEARCH;
}

//Minidump creation
void weave::win32::CreateMinidump(bool createMiniDump, bool createFullDump) {
	//This is done in a secondary thread because minidump cannot handle its own thread correctly.
	auto futureHandle = std::async(std::launch::async, [=] {
		//MiniDumpNormal
		if (createFullDump) {
			auto flag = MINIDUMP_TYPE(MiniDumpWithDataSegs | MiniDumpWithFullMemory);
			HANDLE winf = CreateFileW(L"fulldump.dmp", GENERIC_READ | GENERIC_WRITE, 0, nullptr, 2, FILE_ATTRIBUTE_NORMAL, nullptr);
			MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), winf, flag, nullptr, nullptr, nullptr);
			CloseHandle(winf);
		}

		if (createMiniDump) {
			HANDLE winf = CreateFileW(L"minidump.dmp", GENERIC_READ | GENERIC_WRITE, 0, nullptr, 2, FILE_ATTRIBUTE_NORMAL, nullptr);
			MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), winf, MiniDumpNormal, nullptr, nullptr, nullptr);
			CloseHandle(winf);
		}
	});

	futureHandle.wait();
}

void weave::win32::InstallExceptionHandler(int dumpModeIn, std::function<LONG(struct _EXCEPTION_POINTERS* exInfo)> callback) {
	//Set the dump mode configuration used by the exception handler to determine what dumps to create
	dumpMode = dumpModeIn;
	userCallback = callback;
	//Debugging mini dump handler for exceptions
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)Win32FaultHandler);
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
