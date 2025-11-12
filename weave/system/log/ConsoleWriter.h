/*
Title: "Weave WLog Console Writer"
File: ConsoleWriter.h
Author(s): Eduardo Martínez Vidal

Abstract:
	WLog writer that outputs to std::cout
Update Log:
	31 July 2017:
	File creation

*/
#pragma once

#include "WLogWriter.h"
#include "DefaultFormater.h"
#include <iostream>

namespace weave {

template<typename Formater = DefaultFormater>
class ConsoleWriter : public WLogWriter {
private:
	std::string outBuffer;
public:
	virtual ~ConsoleWriter() override = default;

	//Calles when a new record is started
	virtual void BeginRecord(std::chrono::time_point<std::chrono::system_clock> const &startTime, uint64_t channel,
							 std::string const &fileDef, std::string const &lineDef, std::string const &funcLongDef, std::string const &funcShortDef, std::string const &dateDef, std::string const &timeDef) override {

		std::cout << Formater::BeginRecord(startTime, channel, fileDef, lineDef, funcLongDef, funcShortDef, dateDef, timeDef, outBuffer);
	}
	//Called when text is passed for the current record
	virtual void Append(std::string const &text, uint64_t channel) override {
		std::cout << Formater::Append(text, channel, outBuffer);
	}

};

}
