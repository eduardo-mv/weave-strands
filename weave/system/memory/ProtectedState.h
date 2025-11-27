#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <utility>
#include "BufferedCache.h"

namespace weave {
	template<typename StateType>
	struct ProtectedState {
		StateType state{};
		mutable std::mutex stateMutex;
		mutable std::atomic_uint64_t stateRevision { 1 };
		mutable BufferedCache<StateType> stateCache;

		template<typename TouchLambda>
		uint64_t Touch(TouchLambda const& touch) {
			std::lock_guard<std::mutex> lock(stateMutex);
			touch(state);
			return stateRevision.fetch_add(1, std::memory_order_release);
		}

		uint64_t Revision() const {
			return stateRevision.load(std::memory_order_acquire);
		}

		auto Snapshot() const {
			UpdateCache();
			return stateCache.GetCache();
		}

		auto SnapshotAndRevision() const {
			while(true) {
				auto revision = stateRevision.load(std::memory_order_acquire);
				auto snapshot = Snapshot();
				auto snapshotRevision = stateRevision.load(std::memory_order_acquire);
				if(revision == snapshotRevision) {
					return std::make_pair(snapshot, revision);
				}
			}
		}

		template<typename FilterLambda>
		auto Snapshot(FilterLambda&& filter) const {
			UpdateCache();
			return stateCache.template GetCache(std::forward<FilterLambda>(filter));
		}

		template<typename FilterLambda>
		auto SnapshotAndRevision(FilterLambda&& filter) const {
			while(true) {
				auto revision = stateRevision.load(std::memory_order_acquire);
				auto snapshot = Snapshot(std::forward<FilterLambda>(filter));
				auto snapshotRevision = stateRevision.load(std::memory_order_acquire);
				if(revision == snapshotRevision) {
					return std::make_pair(snapshot, revision);
				}
			}
		}

		void UpdateCache() const {
			stateCache.UpdateCache(Revision(), [&](auto& target, auto& revision){ 
				std::lock_guard lock(stateMutex);
				target = state;
				revision = Revision();
			});
		}
	};
};
