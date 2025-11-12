/*
Title: "Weave WLog Broadcaster"
File: LogBroadcaster.h
Author(s): Eduardo Martínez Vidal

Abstract:
	WLog writer that broadcasts the formatted text into a message that can be captured by any other module.
	Message sent is:
	
	Weave.WLog.Broadcast
*/
#pragma once

#include "WLogWriter.h"
#include "DefaultFormater.h"
#include "system/messaging/MessageSystem.h"
#include <fstream>
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning(disable: 4996) //4996: Deprecated std::localtime (this won't have an alternative till C++20)
#endif

namespace weave {

template<typename Formater = DefaultFormater>
class LogBroadcaster : public WLogWriter {
private:
	std::string outBuffer;
	std::vector<std::pair<uint64_t, std::string>> logQueue; //Used to record pending messages, usually to track initial log entries that cannot be broadcasted due to the msg system not being ready
public:
	//Calles when a new record is started
	void BeginRecord(std::chrono::time_point<std::chrono::system_clock> const& startTime, uint64_t channel,
		std::string const& fileDef, std::string const& lineDef, std::string const& funcLongDef, std::string const& funcShortDef, std::string const& dateDef, std::string const& timeDef) override {

		Formater::BeginRecord(startTime, channel, fileDef, lineDef, funcLongDef, funcShortDef, dateDef, timeDef, outBuffer);
		SendMessage(channel);
	}
	//Called when text is passed for the current record
	void Append(std::string const& text, uint64_t channel) override {
		outBuffer = Formater::Append(text, channel, outBuffer);
		SendMessage(~0ull);
	}

	void SendMessage(uint64_t channel) {
		if (Service<MessageSystem>()) {
			if (!logQueue.empty()) {
				for (auto& entry : logQueue) {
					SendMessage(entry.first, entry.second);
				}
				logQueue.clear();
			}

			SendMessage(channel, outBuffer);
		}
		else {
			logQueue.emplace_back(std::pair{ channel, outBuffer });
		}
	}

	void SendMessage(uint64_t channel, std::string const& buffer) const {
		static uint64_t msgName = weave::HashString("Weave.WLog.Broadcast");
		weave::Message msg(msgName);
		auto strCpy = msg.AllocatePackage<char>(buffer.size() + 1 + sizeof(channel));
		std::memcpy(strCpy, &channel, sizeof(channel));
		std::memcpy(strCpy + sizeof(channel), buffer.c_str(), buffer.size() + 1);
		msg.Queue();
	}
};

}

#ifdef _MSC_VER
#pragma warning( pop )
#endif