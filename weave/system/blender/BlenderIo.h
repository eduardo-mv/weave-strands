#pragma once

#include <tuple>
#include <typeinfo>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <memory>
#include <vector>
#include <utility>
#include "TupleTraits.h"

namespace weave::blender {

struct BlenderNodeBase;

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
			input.nodeLink->Execute(executionStamp);
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

}
