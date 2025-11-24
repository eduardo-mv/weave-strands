#include "ParticleEmitter.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "weave/system/math/Random.h"

#include "weave/particles/ParticleBuffer.h"

namespace weave::particles {

ParticleEmitter::ParticleEmitter() {
	this->input.SetDefaultValues(
		0.0f,                               // min emit
		100.0f,                             // max emit
		1.0f,                               // rate multiplier
		0.0f,                               // min period (seconds)
		0.0f,                               // max period (seconds)
		std::numeric_limits<double>::max(), // max runtime
		std::numeric_limits<uint64_t>::max(), // max particles
		0u                                  // reset signal
	);
}

void ParticleEmitter::Reset() {
	accumTime = 0.0f;
	nextFreq = 0.0f;
	runTime = 0.0;
	fractionalCarry = 0.0;
	emittedTotal = 0;
}

float ParticleEmitter::RandomFrequency(float minFreq, float maxFreq) const {
	if (minFreq <= 0.0f && maxFreq <= 0.0f) {
		return 0.0f;
	}

	const float low = (minFreq <= 0.0f) ? maxFreq : std::min(minFreq, maxFreq);
	const float high = (maxFreq <= 0.0f) ? minFreq : std::max(minFreq, maxFreq);

	if (low <= 0.0f) {
		return std::max(high, 0.0f);
	}

	return weave::rng::uniform<float>() * (high - low) + low;
}

void ParticleEmitter::ExecuteNode() {
	auto& context = GetContext();
	if (context.buffers.empty()) {
		return;
	}

	auto& minEmitInput = this->input.template Ref<MinEmit>();
	auto& maxEmitInput = this->input.template Ref<MaxEmit>();
	auto& rate = this->input.template Ref<Rate>();
	auto& minFreq = this->input.template Ref<MinFrequency>();
	auto& maxFreq = this->input.template Ref<MaxFrequency>();
	auto maxRuntime = this->input.template Ref<MaxRuntime>();
	auto& maxParticles = this->input.template Ref<MaxParticles>();
	auto& resetSig = this->input.template Ref<ResetSignal>();

	if (lastResetValue != resetSig) {
		Reset();
		lastResetValue = resetSig;
	}

	if (maxRuntime < 0.0) {
		maxRuntime = std::numeric_limits<double>::max();
	}

	if (maxParticles == 0u) {
		return;
	}

	if (nextFreq <= 0.0f && (minFreq > 0.0f || maxFreq > 0.0f)) {
		nextFreq = RandomFrequency(minFreq, maxFreq);
	}

	const float deltaTime = std::max(0.0f, context.sampling.deltaTime);
	const bool isVisible = context.sampling.isVisible;

	runTime = std::min(runTime + static_cast<double>(deltaTime), maxRuntime);

	if (emittedTotal >= maxParticles || runTime >= maxRuntime) {
		return;
	}

	bool shouldEmit = false;

	if (nextFreq <= 0.0f) {
		shouldEmit = isVisible;
	}
	else {
		accumTime += deltaTime;
		if (accumTime >= nextFreq) {
			accumTime = 0.0f;
			shouldEmit = isVisible;
			nextFreq = RandomFrequency(minFreq, maxFreq);
		}
	}

	if (!shouldEmit) {
		return;
	}

	const float emitMin = std::min(minEmitInput, maxEmitInput);
	const float emitMax = std::max(minEmitInput, maxEmitInput);
	const float rateMultiplier = std::max(rate, 0.0f);

	double countd = (weave::rng::uniform<double>() * (emitMax - emitMin) + emitMin) * rateMultiplier;
	if (std::isnan(countd) || std::isinf(countd) || countd < 0.0) {
		countd = 0.0;
	}

	if (nextFreq <= 0.0f) {
		const double remainingRuntime = std::max(0.0, maxRuntime - runTime);
		countd *= std::min<double>(deltaTime, remainingRuntime);
	}

	fractionalCarry += countd;
	const auto integralPart = static_cast<uint64_t>(std::floor(fractionalCarry));
	fractionalCarry = std::max(0.0, fractionalCarry - integralPart);

	uint64_t particlesToEmit = integralPart;
	if (particlesToEmit == 0) {
		return;
	}

	const uint64_t remaining = maxParticles - emittedTotal;
	if (particlesToEmit > remaining) {
		particlesToEmit = remaining;
	}

	if (particlesToEmit == 0) {
		return;
	}

	for (auto* buffer : context.buffers) {
		if (buffer) {
			buffer->AddEmissionParticles(particlesToEmit);
		}
	}

	emittedTotal += particlesToEmit;
}

} // namespace weave::particles
