/*
Title: "Weave Logger Writer"
File: WLogWriter.h
Author(s): Eduardo Martínez Vidal

Abstract:
	Base class for WLog writers
Update Log:
	31 July 2017:
	File creation

*/
#pragma once

#include <string>
#include <sstream>
#include <cstdio>
#include <thread>
#include <chrono>
#include <ctime>
#include <vector>

namespace weave {

class WLogWriter {
public:
	virtual ~WLogWriter() = default;

	//Calles when a new record is started
	virtual void BeginRecord(std::chrono::time_point<std::chrono::system_clock> const &startTime, uint64_t channel,
							 std::string const &fileDef, std::string const &lineDef, std::string const &funcLongDef, std::string const &funcShortDef, std::string const &dateDef, std::string const &timeDef) = 0;
	//Called when text is passed for the current record
	virtual void Append(std::string const &text, uint64_t channel) = 0;

};

}
