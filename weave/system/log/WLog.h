/*
Title: "Weave Logger"
File: WLog.h
Author(s): Eduardo Martínez Vidal

Abstract:
	Implements a logger class and singleton with support macros.
	The logger is extendable with output systems and formatters.
Update Log:
	31 July 2017:
	File creation

*/
#pragma once

#include "WLogWriter.h"
#include <mutex>
#include <memory>
#include <initializer_list>

namespace weave {
	
class WLog {
public:
	enum Channel {
		None = 0,
		Trivial = 1 << 0,
		Info = 1 << 1,
		Warning = 1 << 2,
		Error = 1 << 3,
		Critical = 1 << 4,
		Dev = 1 << 5,
	};

private:
	//The output channels and the associated output writers
	std::vector<std::vector<std::shared_ptr<WLogWriter>>> writers;
	//The current output mask
	uint64_t channelMask;
	//The time point of the current record
	std::chrono::time_point<std::chrono::system_clock> recordTime;
	//Static buffer to output printf style commands
	std::string printfBuffer;
	//Dynamic stream for streamed output
	std::stringstream streamBuffer;

	//Strings holding the various preprocessor generated strings which can be set with specific functions
	//These are usually set using a macro automatically but can be set manually
	std::string fileDef, lineDef, funcLongDef, funcShortDef, dateDef, timeDef;

	//The control mutex
	std::mutex mutex;

public:
	WLog();
	~WLog();
	
	//Initializes the logger with a list of writers 
	//Usage: Initialize({{channelsMask, {new WLogWriter(), new WLogWriter()}}, {..}});
	void Initialize(std::initializer_list<std::pair<uint64_t, std::initializer_list<WLogWriter*>>> list);

	//Sets preprocessor strings
	WLog& Preprocessor(char const *file, int line, char const * function, char const *func, char const *date, char const *time, bool condition = false);

	//Executes the log operation only if the asserted condition is true
	WLog& Assert(bool condition, uint64_t channelMask);
	WLog& Assert(bool condition, uint64_t channelMask, char *format, ...);

	//Sets the current output channel mask and resets the current record entry
	WLog& operator()(uint64_t channelMask);
	//Performs stream-like output operations on the current record
	template<typename Type>
	WLog& operator<<(Type const &in);
	//Performs printf-like operations on the current record
	WLog& operator()(char const *format, ...);
	//Performs printf-like output and resets the current record entry
	WLog& operator()(uint64_t channelMask, char const *format, ...);

};

#define WLOG(channel) weave::wlog.Preprocessor(__FILE__, __LINE__, __FUNCTION__, __func__, __DATE__, __TIME__)(channel)
#define WLOGASSERT(condition, channel) weave::wlog.Preprocessor(__FILE__, __LINE__, __FUNCTION__, __func__, __DATE__, __TIME__, (condition)).Assert((condition), channel)
#define WLOGA weave::wlog
#define WLOGINIT(params) weave::wlog.Initialize(params);
extern WLog wlog;

template<typename Type>
inline WLog & WLog::operator<<(Type const & in) {
	if(channelMask) {
		std::lock_guard<std::mutex> lock(mutex);

		//Format string
		streamBuffer.clear();
		streamBuffer.str("");
		streamBuffer << in;

		for(uint64_t mask = 1, i = 0; mask <= channelMask && i < writers.size(); mask = mask << 1, ++i) {
			if(mask & channelMask) {
				for(auto const &wr : writers[i]) {
					wr->Append(streamBuffer.str().c_str(), mask);
				}
			}
		}
	}

	return *this;
}

}
