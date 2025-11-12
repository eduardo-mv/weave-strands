#include "WLog.h"
#include <cstdarg>
using namespace weave;

weave::WLog weave::wlog;

weave::WLog::WLog() {
	printfBuffer.resize(2047);
}

weave::WLog::~WLog() {
}

void weave::WLog::Initialize(std::initializer_list<std::pair<uint64_t, std::initializer_list<WLogWriter*>>> list) {
	//Go through all entries and add them to the right writer lists
	for(auto const &item : list) {
		for(auto *writer : item.second) {
			std::shared_ptr<WLogWriter> ptr(writer);

			for(uint64_t mask = 1, i = 0; i < 64; mask = mask << 1, ++i) {
				if(item.first & mask) {
					if(i >= writers.size())
						writers.resize(i + 1);
					writers[i].push_back(ptr);
				}
			}
		}
	}
}

WLog & weave::WLog::Preprocessor(char const * file, int line, char const * function, char const * func, char const * date, char const * time, bool condition) {
	if(!condition) {
		std::lock_guard<std::mutex> lock(mutex);

		fileDef = (file ? file : "");
		lineDef = std::to_string(line);
		funcLongDef = (function ? function : "");
		funcShortDef = (func ? func : "");
		dateDef = (date ? date : "");
		timeDef = (time ? time : "");
	}
	return *this;
}

//Executes the log operation only if the asserted condition is true
WLog & weave::WLog::Assert(bool condition, uint64_t channelMaskIn) {
	//Reset mask
	(*this)((!condition ? channelMaskIn : 0));
	return *this;
}

WLog & weave::WLog::Assert(bool condition, uint64_t channelMaskIn, char * format, ...) {
	//Reset mask
	(*this)((!condition ? channelMaskIn : 0));
	//Format string
	if(channelMaskIn) {
		std::lock_guard<std::mutex> lock(mutex);

		va_list args;
		va_start(args, format);
		auto size = std::vsnprintf(&printfBuffer[0], 2047, format, args);
		printfBuffer.resize(size);

		for(uint64_t mask = 1, i = 0; mask <= channelMaskIn && i < writers.size(); mask = mask << 1, ++i) {
			if(mask & channelMaskIn) {
				for(auto const &wr : writers[i]) {
					wr->Append(printfBuffer, mask);
				}
			}
		}
	}
	return *this;
}

WLog & weave::WLog::operator()(uint64_t channelMaskIn) {
	std::lock_guard<std::mutex> lock(mutex);

	channelMask = channelMaskIn;
	if(channelMask) {
		recordTime = std::chrono::system_clock::now();

		for(uint64_t mask = 1, i = 0; mask <= channelMask && i < writers.size(); mask = mask << 1, ++i) {
			if(mask & channelMask) {
				for(auto const &wr : writers[i]) {
					wr->BeginRecord(recordTime, mask, fileDef, lineDef, funcLongDef, funcShortDef, dateDef, timeDef);
				}
			}
		}
	}
	return *this;
}

WLog & weave::WLog::operator()(char const * format, ...) {
	//Format string
	if(channelMask) {
		std::lock_guard<std::mutex> lock(mutex);

		va_list args;
		va_start(args, format);
		printfBuffer.resize(2047);
		auto printfSize = std::vsnprintf(&printfBuffer[0], 2047, format, args);
		printfBuffer.resize(printfSize);

		for(uint64_t mask = 1, i = 0; mask <= channelMask && i < writers.size(); mask = mask << 1, ++i) {
			if(mask & channelMask) {
				for(auto const &wr : writers[i]) {
					wr->Append(printfBuffer, mask);
				}
			}
		}
	}
	return *this;
}

WLog & weave::WLog::operator()(uint64_t channelMaskIn, char const * format, ...) {
	//Reset mask
	(*this)(channelMaskIn);
	
	if(channelMask) {
		std::lock_guard<std::mutex> lock(mutex);

		//Format string
		va_list args;
		va_start(args, format);
		printfBuffer.resize(2047);
		auto printfSize = std::vsnprintf(&printfBuffer[0], 2047, format, args);
		printfBuffer.resize(printfSize);

		for(uint64_t mask = 1, i = 0; mask <= channelMask && i < writers.size(); mask = mask << 1, ++i) {
			if(mask & channelMask) {
				for(auto const &wr : writers[i]) {
					wr->Append(printfBuffer, mask);
				}
			}
		}
	}
	return *this;
}
