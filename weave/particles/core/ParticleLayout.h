#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>

#include <weave/system/math/VectorMath.h>

namespace weave::particles {

namespace layout {

struct Position {
	Vector3 pos;
};

struct Velocity {
	Vector3 vel;
};

struct LifeTime {
	float lifeTime;
};

struct MaxLifeTime {
	float maxLifeTime;
};

struct Force {
	Vector3 force;
};

struct Mass {
	float mass;
};

struct Target {
	Vector3 targetPosition;
	float time;
};

struct StdParticle
	: Position
	, LifeTime
	, Velocity
	, MaxLifeTime
	, Force
	, Mass
	, Target {};

} // namespace layout

template<typename... Types>
using ParticleOffsetArray = std::array<size_t, sizeof...(Types)>;

struct ParticleLayout {
	static constexpr size_t kInvalidOffset = std::numeric_limits<size_t>::max();

	size_t particleByteSize{ 0 };

private:
	std::unordered_map<uint64_t, size_t> typedByteOffsets;

	template<typename T>
	static constexpr uint64_t TypeHash() {
		return static_cast<uint64_t>(typeid(T).hash_code());
	}

public:
	template<typename T>
	void SetOffset(size_t byteOffset) {
		typedByteOffsets[TypeHash<T>()] = byteOffset;
		const size_t endOffset = byteOffset + sizeof(T);
		if (endOffset > particleByteSize) {
			particleByteSize = endOffset;
		}
	}

	template<typename ...T, typename... Offsets>
	requires (sizeof...(T) == sizeof...(Offsets) && sizeof...(T) > 0)
	void SetOffsets(Offsets... offsets) {
		(SetOffset<T>(offsets), ...);
	}

	template<typename T>
	size_t GetOffset() const {
		if (auto it = typedByteOffsets.find(TypeHash<T>()); it != typedByteOffsets.end()) {
			return it->second;
		}
		return kInvalidOffset;
	}

	template<typename... Types>
	ParticleOffsetArray<Types...> GetOffsets() const {
		return { GetOffset<Types>()... };
	}

	template<std::size_t N>
	static constexpr bool HasInvalidOffsets(std::array<size_t, N> const& offsets) {
		return std::any_of(offsets.begin(), offsets.end(), [](size_t offset) {
			return offset == kInvalidOffset;
		});
	}

	static ParticleLayout BuildStdParticleLayout() {
		using namespace weave::particles::layout;

		ParticleLayout layout;
		layout.particleByteSize = sizeof(StdParticle);

		layout.SetOffsets<StdParticle, Position, Velocity, LifeTime, MaxLifeTime, Force, Mass, Target>(
			size_t(0),
			OffsetOfBase<StdParticle, Position>(),
			OffsetOfBase<StdParticle, Velocity>(),
			OffsetOfBase<StdParticle, LifeTime>(),
			OffsetOfBase<StdParticle, MaxLifeTime>(),
			OffsetOfBase<StdParticle, Force>(),
			OffsetOfBase<StdParticle, Mass>(),
			OffsetOfBase<StdParticle, Target>());

		return layout;
	}

private:
	template<typename Derived, typename Base>
	static size_t OffsetOfBase() {
		static_assert(std::is_base_of_v<Base, Derived>,
			"OffsetOfBase requires Base to be a base of Derived");

		Derived instance{};
		auto derivedPtr = reinterpret_cast<std::byte*>(&instance);
		auto basePtr = reinterpret_cast<std::byte*>(static_cast<Base*>(&instance));
		return static_cast<size_t>(basePtr - derivedPtr);
	}
};

} // namespace weave::particles
