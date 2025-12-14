#pragma once

#include <tuple>
#include <typeinfo>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <vector>
#include <utility>
#include <limits>
#include <cstddef>
#include <cassert>
#include <span>
#include <iterator>
#include <functional>
#include <algorithm>
#include <array>
#include "TupleTraits.h"


namespace weave::blender {

struct BlenderNodeBase;
	
using FlowId = uint32_t;
inline constexpr FlowId kInvalidFlowId = std::numeric_limits<FlowId>::max();

class FlowInterface {
public:
	class SmallByteVector {
	public:
		static constexpr size_t kInlineCapacity = 64;

		SmallByteVector() = default;

		size_t size() const { return size_; }

		bool empty() const { return size_ == 0; }

		std::byte* data() {
			return usingHeap_ ? heap_.data() : inlineStorage_.data();
		}

		std::byte const* data() const {
			return usingHeap_ ? heap_.data() : inlineStorage_.data();
		}

		void clear() {
			if (usingHeap_) {
				heap_.clear();
			}
			size_ = 0;
		}

		void resize(size_t newSize) {
			if (!usingHeap_ && newSize <= kInlineCapacity) {
				if (newSize > size_) {
					std::fill_n(inlineStorage_.data() + size_, newSize - size_, std::byte{});
				}
				size_ = newSize;
				return;
			}

			EnsureHeap();
			heap_.resize(newSize);
			size_ = newSize;
		}

	private:
		void EnsureHeap() {
			if (usingHeap_) {
				return;
			}
			heap_.assign(inlineStorage_.begin(), inlineStorage_.begin() + size_);
			usingHeap_ = true;
		}

		size_t size_{};
		bool usingHeap_{};
		std::vector<std::byte> heap_;
		std::array<std::byte, kInlineCapacity> inlineStorage_{};
	};

	struct FlowDataContainer {
		std::type_index typeId;
		SmallByteVector data;
		size_t fence{};
		std::function<void(void*, size_t)> destructor;
		
		~FlowDataContainer() {
			clear();
		}

		void clear() {
			if (!data.empty()) {
				destructor(data.data(), data.size());
			}
			data.clear();
			fence = 0;
		}

		template<typename T>
		void push_back(T&& value) {
			using ValueType = std::decay_t<T>;
			assert(typeId == typeid(ValueType));
			
			auto const offset = data.size();
			data.resize(offset + sizeof(T));
			void* ptr = data.data() + offset;
			::new(ptr) ValueType(std::forward<T>(value));
		}

		void flush() {
			fence = data.size();
		}

		template<typename T>
		auto make_span() {
			assert(typeId == typeid(T));
			assert(fence <= data.size());
			return std::span<T>{ reinterpret_cast<T*>(data.data() + fence), (data.size() - fence) / sizeof(T) };
		}

		bool is_fenced() const {
			return fence > 0;
		}
	};

	struct FlowBlock {
		FlowId id{ kInvalidFlowId };
		std::unordered_map<std::type_index, std::unique_ptr<FlowDataContainer>> dataTypeMap; // Type-erased storage and memory offsets describing the "start" of the storage buffer, acting as a fence to virtually remove data from a flow
		std::vector<FlowId> ancestry;

		template<typename ...Types>
		void PrepareData() {
			(MakeData<Types>(), ...);
		}

		template<typename T>
		FlowDataContainer& GetContainer() {
			PrepareData<T>();
			std::type_index typeIndex = typeid(T);
			return *dataTypeMap[typeIndex].get();
		}

		template<typename T>
		FlowDataContainer* TryContainer() {
			std::type_index typeIndex = typeid(T);
			
			if (auto it = dataTypeMap.find(typeIndex); it != dataTypeMap.end()) {
				return it->second.get();

			}
			return nullptr;
		}

	private:
		template<typename T>
		void MakeData() {
			 const auto key = std::type_index(typeid(T));
			if (dataTypeMap.find(key) != dataTypeMap.end()) {
				return;
			}

			auto container = 
			std::make_unique<FlowDataContainer>(
				key,
				SmallByteVector{},
				size_t{0},
				[](void* mem, size_t bytes) {
					if (!mem || bytes == 0) {
						return;
					}
					auto* items = static_cast<T*>(mem);
					size_t count = bytes / sizeof(T);
					for (size_t i = 0; i < count; ++i) {
						items[i].~T();
					}
				}
			);

			dataTypeMap.emplace(key, std::move(container));
		}

	};

	FlowInterface() = default;

	void Reset() {
		flowBlockStorage.clear();
		flowIdCounter = 0;
	}

	FlowId CreateFlowId() {
		return ++flowIdCounter;
	}

	FlowBlock& GetBlock(FlowId id) {
		auto& block = flowBlockStorage[id];
		if(!block) {
			block = std::make_unique<FlowBlock>();
			block->id = id;
		}
		
		return *block;
	}

	FlowBlock* TryBlock(FlowId id) {
		if (auto it = flowBlockStorage.find(id); it != flowBlockStorage.end()) {
			return it->second.get();
		}
		return nullptr;
	}

	FlowBlock const* TryBlock(FlowId id) const {
		if (auto it = flowBlockStorage.find(id); it != flowBlockStorage.end()) {
			return it->second.get();
		}
		return nullptr;
	}

	void MergeAncestry(FlowId targetId, std::vector<FlowId> const& parentFlows) {
		auto& block = GetBlock(targetId);
		block.ancestry.clear();

		std::unordered_set<FlowId> uniqueIds;
		for (auto parentId : parentFlows) {
			if (parentId == kInvalidFlowId) {
				continue;
			}

			if (uniqueIds.insert(parentId).second) {
				block.ancestry.push_back(parentId);
			}

			if (auto const* parentBlock = TryBlock(parentId)) {
				for (auto ancestorId : parentBlock->ancestry) {
					if (uniqueIds.insert(ancestorId).second) {
						block.ancestry.push_back(ancestorId);
					}
				}
			}
		}
	}

	void FlushFlow() {
		for (auto& [id, blockPtr] : flowBlockStorage) {
			(void)id;
			if (!blockPtr) {
				continue;
			}
			for (auto& [type, container] : blockPtr->dataTypeMap) {
				container->clear();
			}
		}
	}

private:
	std::unordered_map<FlowId, std::unique_ptr<FlowBlock>> flowBlockStorage;
	FlowId flowIdCounter { 0 };
};

template<typename T>
struct FlowContainerView {
	using FlowDataContainer = FlowInterface::FlowDataContainer;
	std::vector<FlowDataContainer*> ancestry;

	void push_back(T const& value) {
		assert(!ancestry.empty());
		ancestry.front()->push_back(value);
	}

	void flush() {
		assert(!ancestry.empty());
		ancestry.front()->flush();
	}

	static FlowContainerView Make(FlowId flowId, FlowInterface &flowInterface) {
		auto& block = flowInterface.GetBlock(flowId);
		auto& container = block.GetContainer<T>();
		
		FlowContainerView fcv;
		fcv.ancestry.clear();
		fcv.ancestry.push_back(&container);

		for (auto it = block.ancestry.rbegin(); it != block.ancestry.rend(); ++it) {
			auto& ancestorBlock = flowInterface.GetBlock(*it);
			if (auto ancestorContainer = ancestorBlock.TryContainer<T>()) {
				fcv.ancestry.push_back(ancestorContainer);
			}
		}

		return fcv;
	}

	struct Range {
		struct iterator {
			using iterator_category = std::input_iterator_tag;
			using value_type = T;
			using difference_type = std::ptrdiff_t;
			using pointer = T*;
			using reference = T&;

			std::vector<FlowDataContainer*>* ancestors{};
			size_t ancestorIndex{};
			std::span<T> data{};
			size_t dataIndex{};
			bool isFenced{};
			
			iterator() = default;

			explicit iterator(std::vector<FlowDataContainer*>* view)
				: ancestors(view)
			{
				SetupForAncestor(0);
			}

			reference operator*() const {
				return data[dataIndex - 1];
			}

			pointer operator->() const {
				return &(**this);
			}

			iterator& operator++() {
				Advance();
				return *this;
			}

			iterator operator++(int) {
				auto tmp = *this;
				Advance();
				return tmp;
			}

			bool operator==(iterator const& other) const {
				if (!ancestors && !other.ancestors) {
					return true;
				}
				return ancestors == other.ancestors &&
					ancestorIndex == other.ancestorIndex &&
					dataIndex == other.dataIndex;
			}

			bool operator!=(iterator const& other) const {
				return !(*this == other);
			}

		private:
			void SetupForAncestor(size_t index) {
				if(!ancestors)
					return;
				
				if(isFenced || index >= ancestors->size()){
					ancestors = nullptr;
					return;
				}

				ancestorIndex = index;

				auto& container = *(*ancestors)[ancestorIndex];
				isFenced = container.is_fenced();
				data = container.make_span<T>();
				dataIndex = data.size();
				
				while (dataIndex == 0 && ancestors) {
					SetupForAncestor(ancestorIndex + 1);
				}
			}

			void Advance() {
				if (!ancestors) {
					return;
				}
				if (dataIndex > 0) {
					--dataIndex;
				}
				if(dataIndex == 0) {
					SetupForAncestor(ancestorIndex + 1);
				}
			}
		};

		FlowContainerView& view{};

		iterator begin() {
			return iterator(&view.ancestry);
		}

		iterator end() {
			return iterator(nullptr);
		}
	};
};

class DynamicInterface {
private:
	std::unordered_map<std::type_index, std::shared_ptr<void>> uniforms;

	std::vector<std::pair<std::type_index, std::shared_ptr<void>>> inputs;
	std::vector<std::pair<std::type_index, std::shared_ptr<void>>> outputs;
	
public:
	template<typename T, typename ...Args>
	std::shared_ptr<T> GetUniformSharedPtr(Args&&... args) {
		if (auto it = uniforms.find(typeid(T)); it != std::end(uniforms)) {
			return std::reinterpret_pointer_cast<T>(it->second);
		}
		else {
			auto ptr = std::make_shared<T>(std::forward<Args>(args)...);
			uniforms.insert(std::pair{ std::type_index{ typeid(T) }, ptr });
			return ptr;
		}
	}

	template<typename ...Types, typename ...TypeValues>
	std::tuple<std::shared_ptr<Types>...> GetUniformsTuple(TypeValues&&... initValues) {
		if constexpr (sizeof...(TypeValues) == sizeof...(Types)) {
			return std::tuple<std::shared_ptr<Types>...>{
				([&] {return GetUniformSharedPtr<Types>(std::forward<TypeValues>(initValues)); }())...
			};
		}
		else {
			return std::tuple<std::shared_ptr<Types>...>{
				([&] {return GetUniformSharedPtr<Types>(); }())...
			};
		}
	}

	template<typename T = void>
	std::shared_ptr<T> GetOutputSharedPtr(size_t index) const {
		return GetPtr<T>(outputs, index);
	}

	template<typename T = void>
	std::shared_ptr<T> GetInputSharedPtr(size_t index) const {
		return GetPtr<T>(inputs, index);
	}

	template<typename T>
	size_t RegisterOutput(std::shared_ptr<T> ptr) {
		return RegisterOutput(std::pair{ std::type_index{typeid(T)}, ptr });
	}

	size_t RegisterOutput(std::pair<std::type_index, std::shared_ptr<void>> ptr) {
		return InsertPtr(outputs, std::move(ptr));
	}

	bool RemoveOutput(size_t index) {
		return RemovePtr(outputs, index);
	}


	template<typename T>
	size_t RegisterInput(std::shared_ptr<T> ptr) {
		return RegisterInput(std::pair{ std::type_index{typeid(T)}, ptr });
	}

	size_t RegisterInput(std::pair<std::type_index, std::shared_ptr<void>> ptr) {
		return InsertPtr(inputs, std::move(ptr));
	}

	bool RemoveInput(size_t index) {
		return RemovePtr(inputs, index);
	}
	

private:

	template <typename T>
	std::pair<std::type_index, std::shared_ptr<void>> CreateInputStorage() {
		return std::pair{ typeid(T), std::make_shared<T>() };
	}

	template<typename Holder>
	size_t InsertPtr(Holder&& holder, std::pair<std::type_index, std::shared_ptr<void>> ptr) {
		for (size_t i = 0; i < holder.size(); ++i) {
			if (holder[i].first == typeid(void) && !holder[i].second) {
				holder[i] = ptr;
				return i;
			}
		}

		holder.push_back(ptr);
		return holder.size() - 1;
	}

	template<typename Holder>
	size_t RemovePtr(Holder&& holder, size_t index) {
		if (index < holder.size()) {
			holder[index] = std::pair<std::type_index, std::shared_ptr<void>>{ typeid(void), {} };
			return true;
		}

		return false;
	}

	template<typename T, typename Holder>
	std::shared_ptr<T> GetPtr(Holder&& holder, size_t index) const {
		if (index < holder.size()) {
			auto [type, ptrvoid] = holder[index];
			if (type == typeid(T)) {
				return std::reinterpret_pointer_cast<T>(ptrvoid);
			}
		}

		return {};
	}

};


template<typename ...Types>
struct In {
	using ValueTuple = std::tuple<Types...>;
private:
	// Single input represented by a node and a linked shared_ptr from that node
	template<typename T>
	struct Input {
		std::shared_ptr<T> data{ std::make_shared<T>() };
		BlenderNodeBase* nodeLink{};

		bool IsConnected() const {
			return data.use_count() > 1;
		}
	};

	template<typename T>
	auto const& CachedInput(T& input) {
		if (input.nodeLink) {
			input.nodeLink->Execute(executionStamp, 0);
		}

		return *input.data;
	}

	template<typename T, typename OutType>
	void BindNode(T& input, OutType&& out, size_t nodeOutput) {
		// Determine the type of the object we're binding for compatibility and extract the related shared_ptr
		std::pair<std::type_index, std::shared_ptr<void>> ptrPair{ typeid(void), nullptr };
		if constexpr (weave::is_specialization_of_v<std::pair, std::decay_t<OutType>>) {
			(void)nodeOutput;
			ptrPair = out;
		}
		else {
			ptrPair = out.GetSharedPtrOutput(nodeOutput);
		}

		// Assign the input to extracted shared_ptr
		using PtrDataType = std::decay_t<decltype(*input.data)>;
		auto [type, voidPtr] = ptrPair;
		if (voidPtr && type == typeid(PtrDataType)) {
			input.data = std::reinterpret_pointer_cast<PtrDataType>(voidPtr);
		}
		else {
			input.data = std::make_shared<PtrDataType>();
		}

		// Reset the node and bind it if provided
		input.nodeLink = nullptr;
		if constexpr (std::is_base_of_v<BlenderNodeBase, std::decay_t<OutType>>) {
			input.nodeLink = &out;
		}
	}

	template<size_t N = 0, typename Tuple, typename NodeOrOutOrPtr>
	void Connect(Tuple&& tup, size_t inputNum, NodeOrOutOrPtr&& n, size_t nodeOutput) {
		if (inputNum >= std::tuple_size_v<std::decay_t<Tuple>>) {
			return;
		}

		if (inputNum == 0) {
			BindNode(std::get<N>(tup), std::forward<NodeOrOutOrPtr>(n), nodeOutput);
		}
		else {
			if constexpr (N + 1 < std::tuple_size_v<std::decay_t<Tuple>>) {
				return Connect<N + 1>(std::forward<Tuple>(tup), inputNum - 1, std::forward<NodeOrOutOrPtr>(n), nodeOutput);
			}
		}
	}

	template<size_t N = 0, typename Tuple, typename... ValueType>
	void Disconnect(Tuple&& tup, size_t inputNum, ValueType&&... value) {
		if (inputNum >= std::tuple_size_v<std::decay_t<Tuple>>) {
			return;
		}

		if (inputNum == 0) {
			auto& input = std::get<N>(tup);
			input.nodeLink = nullptr;
			
			using PtrDataType = std::decay_t<decltype(*input.data)>;
			if constexpr (sizeof...(ValueType) > 0) {
				input.data = std::make_shared<PtrDataType>(std::forward<ValueType>(value)...);
			}
			else {
				input.data = std::make_shared<PtrDataType>(*input.data);
			}
		}
		else {
			if constexpr (N + 1 < std::tuple_size_v<std::decay_t<Tuple>>) {
				return Disconnect<N + 1>(std::forward<Tuple>(tup), inputNum - 1, std::forward<ValueType>(value)...);
			}
		}
	}

	template<size_t N = 0, typename Tuple>
	bool IsConnectedTuple(Tuple const& tup, size_t inputNum) const {
		if (inputNum >= std::tuple_size_v<std::decay_t<Tuple>>) {
			return false;
		}

		if constexpr (N >= std::tuple_size_v<std::decay_t<Tuple>>) {
			return false;
		}
		else if (inputNum == 0) {
			return std::get<N>(tup).IsConnected();
		}
		else {
			return IsConnectedTuple<N + 1>(tup, inputNum - 1);
		}
	}

private:
	struct EmptyType {
		EmptyType() {}
		EmptyType(int64_t&) {}
		constexpr operator int64_t() const { return 0; }
	};

	using StampType = std::conditional_t<(sizeof...(Types) > 0), int64_t&, EmptyType>;
	StampType executionStamp; // From the owner BlenderNodeBase
public:
	std::tuple<Input<Types>...> inputTuple;

	In(int64_t& executionStamp) : executionStamp(executionStamp) {}

	In(In const& other) = default;
	In(In&& other) noexcept = default;
	In& operator=(In const& other) = default;
	In& operator=(In&& other) noexcept = default;

	template<size_t n>
	auto SharedPtr() {
		return std::get<n>(inputTuple).data;
	}

	template<typename T>
	auto SharedPtr() {
		return std::get<Input<T>>(inputTuple).data;
	}

	template<size_t n>
	auto const& Ref() {
		return CachedInput(std::get<n>(inputTuple));
	}

	template<typename T>
	auto const& Ref() {
		return CachedInput(std::get<Input<T>>(inputTuple));
	}

	template<size_t n>
	auto Value() {
		return CachedInput(std::get<n>(inputTuple));
	}

	template<typename T>
	auto Value() {
		return CachedInput(std::get<Input<T>>(inputTuple));
	}

	template<size_t n, typename T>
	void SetDefaultValue(T &&value) {
		auto& ref = std::get<n>(inputTuple);
		if (!ref.IsConnected()) {
			*ref.data = std::forward<T>(value);
		}
	}

	template<typename T>
	void SetDefaultValue(T&& value) {
		auto& ref = std::get<Input<T>>(inputTuple);
		if (!ref.IsConnected()) {
			*ref.data = std::forward<T>(value);
		}
	}

	template<typename ...ValueTypes>
	void SetDefaultValues(ValueTypes&&... values) {
		SetDefaultValueTuple<0>(std::make_tuple(std::forward<ValueTypes>(values)...));
	}

	template<size_t n>
	bool IsConnected() const {
		static_assert(n < GetInputCount());
		return std::get<n>(inputTuple).IsConnected();
	}

	template<typename T>
	bool IsConnected() const {
		return std::get<Input<T>>(inputTuple).IsConnected();
	}

	bool IsConnected(size_t inputNum) const {
		if constexpr (sizeof...(Types) == 0) {
			return false;
		}
		else {
			return IsConnectedTuple(inputTuple, inputNum);
		}
	}

	template<size_t N = 0>
	void SetDefaultValueTuple(ValueTuple const& values) {
		if constexpr (N < GetInputCount()) {
			SetDefaultValue<N>(std::get<N>(values));
			SetDefaultValueTuple<N + 1>(values);
		}
	}

	void Connect(size_t inputNum, std::pair<std::type_index, std::shared_ptr<void>> typedPtr) {
		if constexpr (sizeof...(Types) > 0) {
			Connect(inputTuple, inputNum, typedPtr, 0);
		}
	}

	void Connect(size_t inputNum, BlenderNodeBase* n, size_t nodeOutputNum) {
		if constexpr (sizeof...(Types) > 0) {
			Connect(inputTuple, inputNum, *n, nodeOutputNum);
		}
	}

	template<typename OutType>
	void Connect(size_t inputNum, OutType& out, size_t nodeOutputNum) {
		if constexpr (sizeof...(Types) > 0) {
			Connect(inputTuple, inputNum, out, nodeOutputNum);
		}
	}

	void Disconnect(size_t inputNum) {
		if constexpr (sizeof...(Types) > 0) {
			Disconnect(inputTuple, inputNum);
		}
	}

	template<typename ...ValueType>
	void DisconnectWithValue(size_t inputNum, ValueType&&... value) {
		if constexpr (sizeof...(Types) > 0) {
			Disconnect(inputTuple, inputNum, std::forward<ValueType>(value)...);
		}
	}

	static constexpr size_t GetInputCount() {
		return std::tuple_size_v<decltype(inputTuple)>;
	}

	constexpr std::type_index GetInputType(size_t inputNum) const {
		using TupleClean = std::tuple<Types...>;
		return weave::getTypeByIndex<TupleClean>(inputNum);
	}
};

template<typename ...Types>
struct Out {
	using ValueTuple = std::tuple<Types...>;

	std::tuple<std::shared_ptr<Types>...> outputTuple{ std::make_shared<Types>()... };

	Out() = default;
	Out(Out const& other) = default;
	Out(Out&& other) noexcept = default;
	Out& operator=(Out const& other) = default;
	Out& operator=(Out&& other) noexcept = default;

	template<typename... Args>
	Out(Args... args) : outputTuple(std::make_shared<Types>(args)...) {}

	template<size_t n>
	auto SharedPtr() {
		return std::get<n>(outputTuple);
	}

	template<typename T>
	auto SharedPtr() {
		return std::get<std::shared_ptr<T>>(outputTuple);
	}

	template<size_t n>
	auto& Ref() {
		return *std::get<n>(outputTuple);
	}

	template<typename T>
	auto& Ref() {
		return *std::get<std::shared_ptr<T>>(outputTuple);
	}

	template<size_t n, typename T>
	void Set(T&& value) {
		*std::get<n>(outputTuple) = std::forward<T>(value);
	}

	template<typename T, typename U = T>
	void Set(U&& value) {
		*std::get<std::shared_ptr<T>>(outputTuple) = std::forward<U>(value);
	}

	template<typename ...U>
	void Set(U&& ...value) {
		((*std::get<std::shared_ptr<std::decay_t<U>>>(outputTuple) = std::forward<U>(value)), ...);
	}

	template<typename ...U>
	void SetTuple(U&& ...value) {
		outputTuple = std::make_tuple(std::make_shared<std::decay_t<U>>(std::forward<U>(value))...);
	}

	template<typename U>
	Out& operator=(U&& value) {
		Set(value);
		return *this;
	}

	auto operator->() {
		return &Ref<0>();
	}

	std::pair<std::type_index, std::shared_ptr<void>> GetSharedPtrOutput(size_t outputNum) {
		return weave::getTypeAndSharedPtrByIndex(outputTuple, outputNum);
	}

	static constexpr size_t GetOutputCount() {
		return std::tuple_size_v<decltype(outputTuple)>;
	}

	constexpr std::type_index GetOutputType(size_t outputNum) const {
		using TupleClean = std::tuple<Types...>;
		return weave::getTypeByIndex<TupleClean>(outputNum);
	}
};

template<typename ...Types>
struct Uniform {
	using ValueTuple = std::tuple<Types...>;

private:
	std::tuple<std::shared_ptr<Types>...> uniformTuple{};

public:

	template<typename Tuple>
	Uniform(Tuple&& tuple) : uniformTuple(std::forward<Tuple>(tuple)) {}

	Uniform() = default;
	Uniform(Uniform const& other) = default;
	Uniform(Uniform&& other) noexcept = default;
	Uniform& operator=(Uniform const& other) = default;
	Uniform& operator=(Uniform&& other) noexcept = default;

	template<size_t n>
	auto SharedPtr() {
		return std::get<n>(uniformTuple);
	}

	template<typename T>
	auto SharedPtr() {
		return std::get<std::shared_ptr<T>>(uniformTuple);
	}

	template<size_t n>
	auto& Ref() {
		return *std::get<n>(uniformTuple);
	}

	template<typename T>
	auto& Ref() {
		return *std::get<std::shared_ptr<T>>(uniformTuple);
	}

	template<size_t n, typename T>
	void Set(T&& value) {
		*std::get<n>(uniformTuple) = std::forward<T>(value);
	}

	template<typename T, typename U = T>
	void Set(U&& value) {
		*std::get<std::shared_ptr<T>>(uniformTuple) = std::forward<U>(value);
	}

	template<typename ...U>
	void Set(U&& ...value) {
		((*std::get<std::shared_ptr<std::decay_t<U>>>(uniformTuple) = std::forward<U>(value)), ...);
	}

	template<typename ...U>
	void SetTuple(U&& ...value) {
		uniformTuple = std::make_tuple(std::make_shared<std::decay_t<U>>(std::forward<U>(value))...);
	}

	template<typename U>
	Uniform& operator=(U&& value) {
		Set(std::forward<U>(value));
		return *this;
	}

	auto operator->() {
		return &Ref<0>();
	}

	constexpr size_t GetUniformCount() const {
		return std::tuple_size_v<decltype(uniformTuple)>;
	}

	constexpr std::type_index GetUniformType(size_t outputNum) const {
		using TupleClean = std::tuple<Types...>;
		return weave::getTypeByIndex<TupleClean>(outputNum);
	}

	template<size_t N = 0>
	void LinkUniforms(DynamicInterface& uniformInterface) {
		if constexpr (N < sizeof...(Types)) {
			auto& ptr = std::get<N>(uniformTuple);
			using ElementType = std::decay_t<decltype(ptr)>::element_type;
			ptr = uniformInterface.GetUniformSharedPtr<ElementType>();
			
			LinkUniforms<N + 1>(uniformInterface);
		}
	}
};


template<typename ...Types>
struct Flow {
	friend class Blender;
	using ValueTuple = std::tuple<Types...>;

private:
	std::tuple<FlowContainerView<Types>...> flowTuple{};

public:
	std::vector<uint32_t> inflowStage{};

public:

	template<typename Tuple>
	Flow(Tuple&& tuple) : flowTuple(std::forward<Tuple>(tuple)) {}

	Flow() = default;
	Flow(Flow const& other) = default;
	Flow(Flow&& other) noexcept = default;
	Flow& operator=(Flow const& other) = default;
	Flow& operator=(Flow&& other) noexcept = default;

	template<typename T>
	void Push(T&& value) {
		using ValueType = std::decay_t<T>;
		std::get<FlowContainerView<ValueType>>(flowTuple).push_back(std::forward<T>(value));
	}

	template<typename T>
	void Flush() {
		using ValueType = std::decay_t<T>;
		std::get<FlowContainerView<ValueType>>(flowTuple).flush();
	}

	template<typename T>
	auto Iterate() {
		using ValueType = std::decay_t<T>;
		return typename FlowContainerView<ValueType>::Range{ std::get<FlowContainerView<ValueType>>(flowTuple) };
	}

	void PrepareFlowData(FlowId flowId, FlowInterface &flowInterface) {
		auto& block = flowInterface.GetBlock(flowId);
		block.PrepareData<Types...>();
	}

	template<size_t N = 0>
	void LinkFlow(FlowId flowId, FlowInterface &flowInterface) {
		if constexpr (N < sizeof...(Types)) {
			using T = std::tuple_element_t<N, ValueTuple>;
			auto& tupleElem = std::get<N>(flowTuple);
			tupleElem = FlowContainerView<T>::Make(flowId, flowInterface);

			LinkFlow<N + 1>(flowId, flowInterface);
		}
	}
};


}
