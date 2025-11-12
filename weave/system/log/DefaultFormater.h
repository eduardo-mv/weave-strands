/*
Title: "Weave WLog Default Formater"
File: DefaultFormater.h
Author(s): Eduardo Martínez Vidal

Abstract:
	WLog output Formater that generates default tags on BeginRecord with current time, function and line
	Append text is pass through
Update Log:
	31 July 2017:
	File creation

*/
#pragma once

#include <string>
#include <chrono>
#include <ctime>

namespace weave {

class DefaultFormater {
public:
	//Called when a new record is started
	static std::string const& BeginRecord(std::chrono::time_point<std::chrono::system_clock> const &startTime, uint64_t channel,
							std::string const &fileDef, std::string const &lineDef, std::string const &funcLongDef, std::string const &funcShortDef, std::string const &dateDef, std::string const &timeDef,
							std::string &out);
	//Called when text is passed for the current record
	static std::string const& Append(std::string const &text, uint64_t channel, std::string &out);

};

}
