#include "DefaultFormater.h"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning(disable: 4996) //4996: Deprecated std::localtime (this won't have an alternative till C++20)
#endif

namespace {
	enum Channel {
		None = 0,
		Trivial = 1 << 0,
		Info = 1 << 1,
		Warning = 1 << 2,
		Error = 1 << 3,
		Critical = 1 << 4,
		Dev = 1 << 5,
	};

char const *ChannelName(uint64_t ch) {
	switch(ch) {
	case Info: return "Info";
	case Warning: return "Warning";
	case Error: return "Error";
	case Critical: return "Critical";
	case Dev: return "Dev";
	default: return "Other";
	}
}

}

//Called when a new record is started
std::string const & weave::DefaultFormater::BeginRecord(std::chrono::time_point<std::chrono::system_clock> const & startTime,
												uint64_t channel, 
												std::string const & /*fileDef*/, std::string const & lineDef, std::string const & funcLongDef, std::string const & /*funcShortDef*/, std::string const & /*dateDef*/, std::string const & /*timeDef*/, std::string & out) {

	out = "\n";

	if(channel > Channel::Trivial) {
		char strtime[256];

		std::time_t time = std::chrono::system_clock::to_time_t(startTime);
		std::strftime(strtime, 256, "%Y-%m-%d %H:%M:%S", std::localtime(&time));

		out += "[";
		out += strtime;
		if(channel >= Channel::Error) {
			out += " " + funcLongDef + "@" + lineDef;
		}
		out += "]";
		out += ChannelName(channel);
		out += ": ";
	}

	return out;
}

//Called when text is passed for the current record
std::string const & weave::DefaultFormater::Append(std::string const & text, uint64_t /*channel*/, std::string & /*out*/) {
	//out = text;
	return text;
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif
