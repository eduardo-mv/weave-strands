/*
Title: "Weave WLog NX Console Writer"
File: NxConsoleWriter.h
Author(s): Eduardo Martínez Vidal

Abstract:
	WLog writer that outputs to NN_LOG
Update Log:
	05 March 2018:
	File creation

*/
#pragma once

#include "WLogWriter.h"
#include "DefaultFormater.h"
#include <nn/nn_Log.h>

namespace weave {

template<typename Formater = DefaultFormater>
class NxConsoleWriter : public WLogWriter {
private:
	std::string outBuffer;
public:
	NxConsoleWriter() {}
	virtual ~NxConsoleWriter() {}

	//Calles when a new record is started
	virtual void BeginRecord(std::chrono::time_point<std::chrono::system_clock> const &startTime, uint64_t channel,
							 std::string const &fileDef, std::string const &lineDef, std::string const &funcLongDef, std::string const &funcShortDef, std::string const &dateDef, std::string const &timeDef) override {

		NN_LOG(Formater::BeginRecord(startTime, channel, fileDef, lineDef, funcLongDef, funcShortDef, dateDef, timeDef, outBuffer).c_str());
	}
	//Called when text is passed for the current record
	virtual void Append(std::string const &text, uint64_t channel) override {
		NN_LOG(Formater::Append(text, channel, outBuffer).c_str());
	}

};

}
