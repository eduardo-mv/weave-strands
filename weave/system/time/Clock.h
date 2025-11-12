/*
* Clocks and interval timing tools based on std::chrono
*/
#pragma once

#include <chrono>
#include <vector>

namespace weave::time {

struct Clock {
	std::chrono::nanoseconds time{}; //Current time, always increasing
	std::chrono::nanoseconds timeDelta{}; //Last delta used to update this timer. It can be 0 if the timer was updated while being paused
	double timeRate = 1.0; //Incremental multiplier used to modify the delta on each update

	//Tick time tracks time in specific time increments determined by the tickRate.
	uint64_t tickCount = 0; //Amount of total ticks generated
	uint64_t tickDelta = 0; //Amount of generated ticks on this iteration
	std::chrono::nanoseconds tickTime{}; //Current time, increased on a per tick basis
	std::chrono::nanoseconds tickRate{ std::chrono::seconds(1) }; //Tick deltarate at which the tickTime increases

	bool frozen = false;

	void ResetCounters();

	void ToggleFreeze() { frozen = !frozen; }

	void Increment(std::chrono::nanoseconds deltaIn);

	//Sets the tick rate in Hertz
	void SetTickRateHz(double hz);

	template<typename Return = float, typename Unit = std::chrono::seconds>
	Return CurrentTime() const { return std::chrono::duration_cast<std::chrono::duration<Return, typename Unit::period>>(time).count(); }

	template<typename Return = float, typename Unit = std::chrono::seconds>
	Return CurrentDelta() const { return std::chrono::duration_cast<std::chrono::duration<Return, typename Unit::period>>(timeDelta).count(); }

	template<typename Return = float, typename Unit = std::chrono::seconds>
	Return CurrentTickTime() const { return std::chrono::duration_cast<std::chrono::duration<Return, typename Unit::period>>(tickTime).count(); }
};

class IntervalTracker {
private:
	std::chrono::steady_clock::time_point timeOrigin;
	std::chrono::steady_clock::time_point intervalStart;
	std::chrono::nanoseconds elapsedTime, deltaTime;

public:

	IntervalTracker() { Start(); }

	//Signals the beginning of the tracking. Sets the initial time to calculate the global clocks
	void Start();

	//Signals the start of a new snapshot in time.
	std::chrono::nanoseconds Snapshot();

	std::chrono::nanoseconds LastSnapshot() const {
		return deltaTime;
	}

	std::chrono::nanoseconds Elapsed() const {
		return elapsedTime;
	}
};

class FrameTracker {
public:
	static constexpr uint64_t frameTrackingAvg = 40;
	
	Clock clock;

	std::chrono::duration<double> deltaFrameTime{ 0.0 };
	std::chrono::duration<double> avgFrameTime{ 1.0 / 60.0 };
	std::chrono::duration<double> avgJitter{ 0.0 };
	std::chrono::nanoseconds deltaFrameTimeCap{}; //Cap applied to the frame time. 0 indicates no cap
	std::chrono::nanoseconds deltaFrameTimeFloor{ std::chrono::milliseconds(42) }; //Minimun allowed frame time (usually set for debugging purposes)

	uint64_t globalFrameCount = 0;


private:
	IntervalTracker intervalTracker;
	//Frame values are kept in record and averaged for statistics purposes
	std::chrono::nanoseconds deltaTrack[frameTrackingAvg] = {};
	uint64_t deltaTrackIndex = 0;

	std::chrono::nanoseconds sleepPrecision{ std::chrono::milliseconds(1) };
public:
	FrameTracker() {
		ProgramStart();
	}
	
	void SetFrametimeCap(std::chrono::nanoseconds cap);
	void SetFramerateCap(double cap);

	void ProgramStart();
	void FrameStart();
};
}