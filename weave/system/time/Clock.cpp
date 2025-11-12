#include "Clock.h"
#include <iostream>
#include <thread>
using namespace weave::time;

void Clock::ResetCounters() {
	time = {};
	timeDelta = {};
	tickTime = {};
	tickCount = 0;
	tickDelta = 0;
}

void Clock::Increment(std::chrono::nanoseconds deltaIn) {
	//Linear time control
	timeDelta = !frozen ? deltaIn : std::chrono::nanoseconds{};
	if (timeRate != 1.0) {
		auto deltaDouble = std::chrono::duration_cast<std::chrono::duration<double, std::nano>>(timeDelta);
		timeDelta = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaDouble * timeRate);
	}
	time += timeDelta;

	//Tick time control
	tickDelta = 0;
	while (time > tickTime + tickRate) {
		tickTime += tickRate;
		++tickDelta;
		++tickCount;
	}
}

void Clock::SetTickRateHz(double hz) {
	tickRate = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(1.0 / hz));
	if (tickRate.count() == 0) {
		tickRate = std::chrono::seconds(1);
	}
}


void IntervalTracker::Start() {
	timeOrigin = std::chrono::steady_clock::now();
	intervalStart = timeOrigin;
	elapsedTime = std::chrono::nanoseconds(0);
}

std::chrono::nanoseconds IntervalTracker::Snapshot() {
	deltaTime = std::chrono::steady_clock::now() - intervalStart;
	if (deltaTime.count() < 0)
		deltaTime = std::chrono::nanoseconds(0);
	
	elapsedTime += deltaTime;
	
	intervalStart = std::chrono::steady_clock::now();

	return deltaTime;
}

void FrameTracker::SetFrametimeCap(std::chrono::nanoseconds cap) {
	deltaFrameTimeCap = cap;
	sleepPrecision = {};
}


void FrameTracker::SetFramerateCap(double cap) {
	if (cap > 0.0) {
		auto capTime = std::chrono::duration<double>(1.0 / cap);
		SetFrametimeCap(std::chrono::duration_cast<std::chrono::nanoseconds>(capTime));
	}
	else {
		SetFrametimeCap(std::chrono::nanoseconds{ 0 });
	}
}

void FrameTracker::ProgramStart() {
	intervalTracker.Start();
	globalFrameCount = 0;
}

void FrameTracker::FrameStart() {
	auto delta = intervalTracker.Snapshot();

	if (delta < deltaFrameTimeCap) {
		//Sleep a big chunk of the difference
		auto sleepTime = (deltaFrameTimeCap - delta);
		std::this_thread::sleep_for(sleepTime - sleepPrecision);
		
		auto realSleepTime = intervalTracker.Snapshot();
		delta += realSleepTime;

		if (realSleepTime > sleepTime) {
			sleepPrecision += std::chrono::milliseconds(1);
		}
		else if (sleepPrecision > std::chrono::nanoseconds(0)) {
			sleepPrecision -= std::chrono::microseconds(10);
		}


		//Yield the rest of the time until we match the frame rate we want
		delta += intervalTracker.Snapshot();

		while (delta < deltaFrameTimeCap) {
			std::this_thread::yield();
			delta += intervalTracker.Snapshot();
		}
	}

	//Protection against very long loops, which often happen when debugging
	if (delta > deltaFrameTimeFloor && deltaFrameTimeFloor > deltaFrameTimeCap) {
		delta = deltaFrameTimeFloor;
	}

	clock.Increment(delta);

	//Convert to seconds
	deltaFrameTime = std::chrono::duration_cast<std::chrono::duration<double>>(delta);

	//Track the average frame time
	deltaTrackIndex = (deltaTrackIndex + 1) % frameTrackingAvg;
	deltaTrack[deltaTrackIndex] = delta;
	std::chrono::nanoseconds avgTime{};
	std::chrono::nanoseconds maxT{ delta }, minT{ delta };
	for (uint64_t i = 0; i < frameTrackingAvg; ++i) {
		avgTime += deltaTrack[i];
		if (deltaTrack[i] < minT) {
			minT = deltaTrack[i];
		}
		if (deltaTrack[i] > maxT) {
			maxT = deltaTrack[i];
		}
	}

	avgTime /= frameTrackingAvg;

	avgJitter = std::chrono::duration_cast<std::chrono::duration<double>>(maxT - minT);
	avgFrameTime = std::chrono::duration_cast<std::chrono::duration<double>>(avgTime);

	globalFrameCount++;
}
