/*
Title: "Weave WLog File Stream Writer"
File: LogfileWriter.h
Author(s): Eduardo Martínez Vidal

Abstract:
	WLog writer that outputs to an std::fstream. 
	File size limit can be defined and the amount of log files kept.
	When a file is full, the next file in the round ribbon is created until the max amount of files is reached
	at which point the first file is overwriten.
	Filenames are automatically appended with a timestamp and an extension.
Update Log:
	31 July 2017:
	File creation

*/
#pragma once

#include "WLogWriter.h"
#include "DefaultFormater.h"
#include <fstream>
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning(disable: 4996) //4996: Deprecated std::localtime (this won't have an alternative till C++20)
#endif

namespace weave {

template<typename Formater = DefaultFormater>
class LogfileWriter : public WLogWriter {
private:
	std::string outBuffer;
	std::ofstream file;

	std::string baseFileName;
	uint64_t currentFile = 0;
	uint64_t maxFileSize = 0;
	uint64_t maxFiles = 1;
	bool useTimestamp = false;

public:
	LogfileWriter(std::string const &fileName, bool timestamp, uint64_t maxFileSize = 0, uint64_t maxFiles = 1) :
		baseFileName(fileName), currentFile(0), maxFileSize(maxFileSize), maxFiles(maxFiles), useTimestamp(timestamp)
	{}
	virtual ~LogfileWriter() {}

	//Called when a new record is started
	virtual void BeginRecord(std::chrono::time_point<std::chrono::system_clock> const &startTime, uint64_t channel,
							 std::string const &fileDef, std::string const &lineDef, std::string const &funcLongDef, std::string const &funcShortDef, std::string const &dateDef, std::string const &timeDef) override {

		CheckFile();
		
		file << Formater::BeginRecord(startTime, channel, fileDef, lineDef, funcLongDef, funcShortDef, dateDef, timeDef, outBuffer);
		
		file.flush();
	}
	//Called when text is passed for the current record
	virtual void Append(std::string const &text, uint64_t channel) override {
		file << Formater::Append(text, channel, outBuffer);

		file.flush();
	}

	//Checks the current file size and creates a new file if conditions are met
	void CheckFile() {
		if(!file.is_open()) {
			//Open the first file
			currentFile = 0;
			auto fname = GenFilename();
			file.open(fname);
			file << fname;
		}
		else if(maxFileSize > 0 && file.tellp() >= int64_t(maxFileSize)) {
			//Open a new file
			currentFile = (currentFile + 1) % maxFiles;
			file.close();
			auto fname = GenFilename();
			file.open(fname);
			file << fname;
		}
	}

	//Generates a filename given the current state
	std::string GenFilename() const {
		std::string fname = baseFileName + std::to_string(currentFile);
		if(useTimestamp) {
			char strtime[256];
			std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			std::strftime(strtime, 256, ".[%F_%H-%M-%S]", std::localtime(&time));

			fname += strtime;
		}
		fname += ".log";

		return fname;
	}

};

}

#ifdef _MSC_VER
#pragma warning( pop )
#endif