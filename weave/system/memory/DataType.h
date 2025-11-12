/*
DataType enum helper
Enum class and support functions to define common data types in a portable manner
*/
#pragma once

#include <typeinfo>
#include <typeindex>
#include <string>
#include <array>
#include <cstdint>

namespace weave::types {
	enum class DataType : int32_t {
		Void,
		Float,
		Float_2,
		Float_3,
		Float_4,
		Float_2x2,
		Float_3x3,
		Float_4x4,
		Double,
		Double_2,
		Double_3,
		Double_4,
		Double_2x2,
		Double_3x3,
		Double_4x4,
		Int8,
		Int8_2,
		Int8_3,
		Int8_4,
		UInt8,
		UInt8_2,
		UInt8_3,
		UInt8_4,
		Int16,
		Int16_2,
		Int16_3,
		Int16_4,
		UInt16,
		UInt16_2,
		UInt16_3,
		UInt16_4,
		Int32,
		Int32_2,
		Int32_3,
		Int32_4,
		UInt32,
		UInt32_2,
		UInt32_3,
		UInt32_4,
		Int64,
		Int64_2,
		Int64_3,
		Int64_4,
		UInt64,
		UInt64_2,
		UInt64_3,
		UInt64_4,
		Bool,
		Bool_2,
		Bool_3,
		Bool_4,
		UserExtended
	};

	template<DataType>
	struct DataTypeTraits;

	template<typename T>
	struct TypeTraits;

	template<DataType dataType, typename NaturalType>
	struct Traits {
	private:
		template<typename T>
		struct size_of {
			static constexpr size_t value = []() constexpr {
				if constexpr (std::is_void_v<T>) {
					return size_t{0};
				}
				else {
					return sizeof(T);
				}
			}();
		};

		template<typename T>
		struct VectorTraits {
			using UnderlyingType = T;
			static constexpr size_t vectorSize = 1;
		};

		template<typename UType, size_t vsize>
		struct VectorTraits<std::array<UType, vsize>> {
			using UnderlyingType = UType;
			static constexpr size_t vectorSize = vsize;
		};

	public:
		using Type = NaturalType;
		using UnderlyingType = VectorTraits<NaturalType>::UnderlyingType;

		static constexpr DataType dataTypeValue = dataType;
		static constexpr size_t vectorSize = VectorTraits<NaturalType>::vectorSize;
		static constexpr size_t byteSizeUnit = size_of<UnderlyingType>::value;
		static constexpr size_t byteSizeVector = byteSizeUnit * vectorSize;
		static constexpr bool isPrimitiveType = std::is_fundamental_v<Type>;

		static std::type_index type_index() { return std::type_index(typeid(Type)); }
		static std::type_index underlying_type_index() { return std::type_index(typeid(UnderlyingType)); }
	};

	template<DataType dataType>
	struct DataTypeTraits : Traits<dataType, void> {
	};

	template<typename T>
	struct TypeTraits : Traits<DataType::Void, T> {
	};

	// Float types
	template<>
	struct DataTypeTraits<DataType::Float>
		: Traits<DataType::Float, float> {
	};

	template<>
	struct TypeTraits<float>
		: Traits<DataType::Float, float> {
	};

	template<>
	struct DataTypeTraits<DataType::Float_2>
		: Traits<DataType::Float_2, std::array<float, 2>> {
	};

	template<>
	struct TypeTraits<std::array<float, 2>>
		: Traits<DataType::Float_2, std::array<float, 2>> {
	};

	template<>
	struct DataTypeTraits<DataType::Float_3>
		: Traits<DataType::Float_3, std::array<float, 3>> {
	};

	template<>
	struct TypeTraits<std::array<float, 3>>
		: Traits<DataType::Float_3, std::array<float, 3>> {
	};

	template<>
	struct DataTypeTraits<DataType::Float_4>
		: Traits<DataType::Float_4, std::array<float, 4>> {
	};

	template<>
	struct TypeTraits<std::array<float, 4>>
		: Traits<DataType::Float_4, std::array<float, 4>> {
	};

	// Float matrix types
	template<>
	struct DataTypeTraits<DataType::Float_2x2>
		: Traits<DataType::Float_2x2, std::array<float, 4>> {
	};

	template<>
	struct DataTypeTraits<DataType::Float_3x3>
		: Traits<DataType::Float_3x3, std::array<float, 9>> {
	};

	template<>
	struct TypeTraits<std::array<float, 9>>
		: Traits<DataType::Float_3x3, std::array<float, 9>> {
	};

	template<>
	struct DataTypeTraits<DataType::Float_4x4>
		: Traits<DataType::Float_4x4, std::array<float, 16>> {
	};

	template<>
	struct TypeTraits<std::array<float, 16>>
		: Traits<DataType::Float_4x4, std::array<float, 16>> {
	};

	// Double types
	template<>
	struct DataTypeTraits<DataType::Double>
		: Traits<DataType::Double, double> {
	};

	template<>
	struct TypeTraits<double>
		: Traits<DataType::Double, double> {
	};

	template<>
	struct DataTypeTraits<DataType::Double_2>
		: Traits<DataType::Double_2, std::array<double, 2>> {
	};

	template<>
	struct TypeTraits<std::array<double, 2>>
		: Traits<DataType::Double_2, std::array<double, 2>> {
	};

	template<>
	struct DataTypeTraits<DataType::Double_3>
		: Traits<DataType::Double_3, std::array<double, 3>> {
	};

	template<>
	struct TypeTraits<std::array<double, 3>>
		: Traits<DataType::Double_3, std::array<double, 3>> {
	};

	template<>
	struct DataTypeTraits<DataType::Double_4>
		: Traits<DataType::Double_4, std::array<double, 4>> {
	};

	template<>
	struct TypeTraits<std::array<double, 4>>
		: Traits<DataType::Double_4, std::array<double, 4>> {
	};

	// Double matrix types
	template<>
	struct DataTypeTraits<DataType::Double_2x2>
		: Traits<DataType::Double_2x2, std::array<double, 4>> {
	};

	template<>
	struct DataTypeTraits<DataType::Double_3x3>
		: Traits<DataType::Double_3x3, std::array<double, 9>> {
	};

	template<>
	struct TypeTraits<std::array<double, 9>>
		: Traits<DataType::Double_3x3, std::array<double, 9>> {
	};

	template<>
	struct DataTypeTraits<DataType::Double_4x4>
		: Traits<DataType::Double_4x4, std::array<double, 16>> {
	};

	template<>
	struct TypeTraits<std::array<double, 16>>
		: Traits<DataType::Double_4x4, std::array<double, 16>> {
	};

	// Int8 types
	template<>
	struct DataTypeTraits<DataType::Int8>
		: Traits<DataType::Int8, int8_t> {
	};

	template<>
	struct TypeTraits<int8_t>
		: Traits<DataType::Int8, int8_t> {
	};

	template<>
	struct DataTypeTraits<DataType::Int8_2>
		: Traits<DataType::Int8_2, std::array<int8_t, 2>> {
	};

	template<>
	struct TypeTraits<std::array<int8_t, 2>>
		: Traits<DataType::Int8_2, std::array<int8_t, 2>> {
	};

	template<>
	struct DataTypeTraits<DataType::Int8_3>
		: Traits<DataType::Int8_3, std::array<int8_t, 3>> {
	};

	template<>
	struct TypeTraits<std::array<int8_t, 3>>
		: Traits<DataType::Int8_3, std::array<int8_t, 3>> {
	};

	template<>
	struct DataTypeTraits<DataType::Int8_4>
		: Traits<DataType::Int8_4, std::array<int8_t, 4>> {
	};

	template<>
	struct TypeTraits<std::array<int8_t, 4>>
		: Traits<DataType::Int8_4, std::array<int8_t, 4>> {
	};

	// UInt8 types
	template<>
	struct DataTypeTraits<DataType::UInt8>
		: Traits<DataType::UInt8, uint8_t> {
	};

	template<>
	struct TypeTraits<uint8_t>
		: Traits<DataType::UInt8, uint8_t> {
	};

	template<>
	struct DataTypeTraits<DataType::UInt8_2>
		: Traits<DataType::UInt8_2, std::array<uint8_t, 2>> {
	};

	template<>
	struct TypeTraits<std::array<uint8_t, 2>>
		: Traits<DataType::UInt8_2, std::array<uint8_t, 2>> {
	};

	template<>
	struct DataTypeTraits<DataType::UInt8_3>
		: Traits<DataType::UInt8_3, std::array<uint8_t, 3>> {
	};

	template<>
	struct TypeTraits<std::array<uint8_t, 3>>
		: Traits<DataType::UInt8_3, std::array<uint8_t, 3>> {
	};

	template<>
	struct DataTypeTraits<DataType::UInt8_4>
		: Traits<DataType::UInt8_4, std::array<uint8_t, 4>> {
	};

	template<>
	struct TypeTraits<std::array<uint8_t, 4>>
		: Traits<DataType::UInt8_4, std::array<uint8_t, 4>> {
	};

	// Int16 types
	template<>
	struct DataTypeTraits<DataType::Int16>
		: Traits<DataType::Int16, int16_t> {
	};

	template<>
	struct TypeTraits<int16_t>
		: Traits<DataType::Int16, int16_t> {
	};

	template<>
	struct DataTypeTraits<DataType::Int16_2>
		: Traits<DataType::Int16_2, std::array<int16_t, 2>> {
	};

	template<>
	struct TypeTraits<std::array<int16_t, 2>>
		: Traits<DataType::Int16_2, std::array<int16_t, 2>> {
	};

	template<>
	struct DataTypeTraits<DataType::Int16_3>
		: Traits<DataType::Int16_3, std::array<int16_t, 3>> {
	};

	template<>
	struct TypeTraits<std::array<int16_t, 3>>
		: Traits<DataType::Int16_3, std::array<int16_t, 3>> {
	};

	template<>
	struct DataTypeTraits<DataType::Int16_4>
		: Traits<DataType::Int16_4, std::array<int16_t, 4>> {
	};

	template<>
	struct TypeTraits<std::array<int16_t, 4>>
		: Traits<DataType::Int16_4, std::array<int16_t, 4>> {
	};

	// UInt16 types
	template<>
	struct DataTypeTraits<DataType::UInt16>
		: Traits<DataType::UInt16, uint16_t> {
	};

	template<>
	struct TypeTraits<uint16_t>
		: Traits<DataType::UInt16, uint16_t> {
	};

	template<>
	struct DataTypeTraits<DataType::UInt16_2>
		: Traits<DataType::UInt16_2, std::array<uint16_t, 2>> {
	};

	template<>
	struct TypeTraits<std::array<uint16_t, 2>>
		: Traits<DataType::UInt16_2, std::array<uint16_t, 2>> {
	};

	template<>
	struct DataTypeTraits<DataType::UInt16_3>
		: Traits<DataType::UInt16_3, std::array<uint16_t, 3>> {
	};

	template<>
	struct TypeTraits<std::array<uint16_t, 3>>
		: Traits<DataType::UInt16_3, std::array<uint16_t, 3>> {
	};

	template<>
	struct DataTypeTraits<DataType::UInt16_4>
		: Traits<DataType::UInt16_4, std::array<uint16_t, 4>> {
	};

	template<>
	struct TypeTraits<std::array<uint16_t, 4>>
		: Traits<DataType::UInt16_4, std::array<uint16_t, 4>> {
	};

	// Int32 types
	template<>
	struct DataTypeTraits<DataType::Int32>
		: Traits<DataType::Int32, int32_t> {
	};

	template<>
	struct TypeTraits<int32_t>
		: Traits<DataType::Int32, int32_t> {
	};

	template<>
	struct DataTypeTraits<DataType::Int32_2>
		: Traits<DataType::Int32_2, std::array<int32_t, 2>> {
	};

	template<>
	struct TypeTraits<std::array<int32_t, 2>>
		: Traits<DataType::Int32_2, std::array<int32_t, 2>> {
	};

	template<>
	struct DataTypeTraits<DataType::Int32_3>
		: Traits<DataType::Int32_3, std::array<int32_t, 3>> {
	};

	template<>
	struct TypeTraits<std::array<int32_t, 3>>
		: Traits<DataType::Int32_3, std::array<int32_t, 3>> {
	};

	template<>
	struct DataTypeTraits<DataType::Int32_4>
		: Traits<DataType::Int32_4, std::array<int32_t, 4>> {
	};

	template<>
	struct TypeTraits<std::array<int32_t, 4>>
		: Traits<DataType::Int32_4, std::array<int32_t, 4>> {
	};

	// UInt32 types
	template<>
	struct DataTypeTraits<DataType::UInt32>
		: Traits<DataType::UInt32, uint32_t> {
	};

	template<>
	struct TypeTraits<uint32_t>
		: Traits<DataType::UInt32, uint32_t> {
	};

	template<>
	struct DataTypeTraits<DataType::UInt32_2>
		: Traits<DataType::UInt32_2, std::array<uint32_t, 2>> {
	};

	template<>
	struct TypeTraits<std::array<uint32_t, 2>>
		: Traits<DataType::UInt32_2, std::array<uint32_t, 2>> {
	};

	template<>
	struct DataTypeTraits<DataType::UInt32_3>
		: Traits<DataType::UInt32_3, std::array<uint32_t, 3>> {
	};

	template<>
	struct TypeTraits<std::array<uint32_t, 3>>
		: Traits<DataType::UInt32_3, std::array<uint32_t, 3>> {
	};

	template<>
	struct DataTypeTraits<DataType::UInt32_4>
		: Traits<DataType::UInt32_4, std::array<uint32_t, 4>> {
	};

	template<>
	struct TypeTraits<std::array<uint32_t, 4>>
		: Traits<DataType::UInt32_4, std::array<uint32_t, 4>> {
	};

	// Int64 types
	template<>
	struct DataTypeTraits<DataType::Int64>
		: Traits<DataType::Int64, int64_t> {
	};

	template<>
	struct TypeTraits<int64_t>
		: Traits<DataType::Int64, int64_t> {
	};

	template<>
	struct DataTypeTraits<DataType::Int64_2>
		: Traits<DataType::Int64_2, std::array<int64_t, 2>> {
	};

	template<>
	struct TypeTraits<std::array<int64_t, 2>>
		: Traits<DataType::Int64_2, std::array<int64_t, 2>> {
	};

	template<>
	struct DataTypeTraits<DataType::Int64_3>
		: Traits<DataType::Int64_3, std::array<int64_t, 3>> {
	};

	template<>
	struct TypeTraits<std::array<int64_t, 3>>
		: Traits<DataType::Int64_3, std::array<int64_t, 3>> {
	};

	template<>
	struct DataTypeTraits<DataType::Int64_4>
		: Traits<DataType::Int64_4, std::array<int64_t, 4>> {
	};

	template<>
	struct TypeTraits<std::array<int64_t, 4>>
		: Traits<DataType::Int64_4, std::array<int64_t, 4>> {
	};

	// UInt64 types
	template<>
	struct DataTypeTraits<DataType::UInt64>
		: Traits<DataType::UInt64, uint64_t> {
	};

	template<>
	struct TypeTraits<uint64_t>
		: Traits<DataType::UInt64, uint64_t> {
	};

	template<>
	struct DataTypeTraits<DataType::UInt64_2>
		: Traits<DataType::UInt64_2, std::array<uint64_t, 2>> {
	};

	template<>
	struct TypeTraits<std::array<uint64_t, 2>>
		: Traits<DataType::UInt64_2, std::array<uint64_t, 2>> {
	};

	template<>
	struct DataTypeTraits<DataType::UInt64_3>
		: Traits<DataType::UInt64_3, std::array<uint64_t, 3>> {
	};

	template<>
	struct TypeTraits<std::array<uint64_t, 3>>
		: Traits<DataType::UInt64_3, std::array<uint64_t, 3>> {
	};

	template<>
	struct DataTypeTraits<DataType::UInt64_4>
		: Traits<DataType::UInt64_4, std::array<uint64_t, 4>> {
	};

	template<>
	struct TypeTraits<std::array<uint64_t, 4>>
		: Traits<DataType::UInt64_4, std::array<uint64_t, 4>> {
	};

	// Bool types
	template<>
	struct DataTypeTraits<DataType::Bool>
		: Traits<DataType::Bool, bool> {
	};

	template<>
	struct TypeTraits<bool>
		: Traits<DataType::Bool, bool> {
	};

	template<>
	struct DataTypeTraits<DataType::Bool_2>
		: Traits<DataType::Bool_2, std::array<bool, 2>> {
	};

	template<>
	struct TypeTraits<std::array<bool, 2>>
		: Traits<DataType::Bool_2, std::array<bool, 2>> {
	};

	template<>
	struct DataTypeTraits<DataType::Bool_3>
		: Traits<DataType::Bool_3, std::array<bool, 3>> {
	};

	template<>
	struct TypeTraits<std::array<bool, 3>>
		: Traits<DataType::Bool_3, std::array<bool, 3>> {
	};

	template<>
	struct DataTypeTraits<DataType::Bool_4>
		: Traits<DataType::Bool_4, std::array<bool, 4>> {
	};


	struct RuntimeTypeTraits {
		DataType dataTypeValue{};
		DataType underlyingDataTypeValue{};
		size_t vectorSize{};
		size_t byteSizeUnit{};
		size_t byteSizeVector{};
		bool isPrimitiveType{};
		std::type_index type_index;
		std::type_index underlying_type_index;

		template<typename T>
		RuntimeTypeTraits(T&& traits)
			: dataTypeValue(traits.dataTypeValue)
			, underlyingDataTypeValue(TypeTraits<typename T::UnderlyingType>::dataTypeValue)
			, vectorSize(traits.vectorSize)
			, byteSizeUnit(traits.byteSizeUnit)
			, byteSizeVector(traits.byteSizeVector)
			, isPrimitiveType(traits.isPrimitiveType)
			, type_index(traits.type_index())
			, underlying_type_index(traits.underlying_type_index())
		{
		}
	};

	template<typename T>
	RuntimeTypeTraits const& GetRuntimeTypeTraits(T dataType) {
		switch (dataType) {
#define CASE_TYPE(TypeEnum)\
			case TypeEnum: {\
			static RuntimeTypeTraits runtimetTraits(DataTypeTraits<TypeEnum>{}); \
			return runtimetTraits; }\
			
			CASE_TYPE(DataType::Float)
			CASE_TYPE(DataType::Float_2)
			CASE_TYPE(DataType::Float_3)
			CASE_TYPE(DataType::Float_4)
			CASE_TYPE(DataType::Float_2x2)
			CASE_TYPE(DataType::Float_3x3)
			CASE_TYPE(DataType::Float_4x4)
			CASE_TYPE(DataType::Double)
			CASE_TYPE(DataType::Double_2)
			CASE_TYPE(DataType::Double_3)
			CASE_TYPE(DataType::Double_4)
			CASE_TYPE(DataType::Double_2x2)
			CASE_TYPE(DataType::Double_3x3)
			CASE_TYPE(DataType::Double_4x4)
			CASE_TYPE(DataType::Int8)
			CASE_TYPE(DataType::Int8_2)
			CASE_TYPE(DataType::Int8_3)
			CASE_TYPE(DataType::Int8_4)
			CASE_TYPE(DataType::UInt8)
			CASE_TYPE(DataType::UInt8_2)
			CASE_TYPE(DataType::UInt8_3)
			CASE_TYPE(DataType::UInt8_4)
			CASE_TYPE(DataType::Int16)
			CASE_TYPE(DataType::Int16_2)
			CASE_TYPE(DataType::Int16_3)
			CASE_TYPE(DataType::Int16_4)
			CASE_TYPE(DataType::UInt16)
			CASE_TYPE(DataType::UInt16_2)
			CASE_TYPE(DataType::UInt16_3)
			CASE_TYPE(DataType::UInt16_4)
			CASE_TYPE(DataType::Int32)
			CASE_TYPE(DataType::Int32_2)
			CASE_TYPE(DataType::Int32_3)
			CASE_TYPE(DataType::Int32_4)
			CASE_TYPE(DataType::UInt32)
			CASE_TYPE(DataType::UInt32_2)
			CASE_TYPE(DataType::UInt32_3)
			CASE_TYPE(DataType::UInt32_4)
			CASE_TYPE(DataType::Int64)
			CASE_TYPE(DataType::Int64_2)
			CASE_TYPE(DataType::Int64_3)
			CASE_TYPE(DataType::Int64_4)
			CASE_TYPE(DataType::UInt64)
			CASE_TYPE(DataType::UInt64_2)
			CASE_TYPE(DataType::UInt64_3)
			CASE_TYPE(DataType::UInt64_4)
			CASE_TYPE(DataType::Bool)
			CASE_TYPE(DataType::Bool_2)
			CASE_TYPE(DataType::Bool_3)
			CASE_TYPE(DataType::Bool_4)
			default:
			CASE_TYPE(DataType::Void)
#undef CASE_TYPE
		}
	}




template<typename OutType>
struct DynamicTypeConvert {
	static void Convert(OutType& sample, std::byte const* sampleBytes, RuntimeTypeTraits const& dataTypeTraits) {
		using SampleTraits = TypeTraits<OutType>;

		if (typeid(OutType) == dataTypeTraits.type_index) {
			// Do no conversion, just copy the data
			sample = *reinterpret_cast<OutType const*>(sampleBytes);
		}
		else {
			// Do appropriate conversion with support for basic types
			auto loopConvert = [&]<typename Type>() {
				if constexpr (SampleTraits::vectorSize == 1) {
					sample = static_cast<typename SampleTraits::UnderlyingType>(reinterpret_cast<const Type*>(sampleBytes)[0]);
				}
				else {
					for (size_t i = 0, end = std::min(dataTypeTraits.vectorSize, SampleTraits::vectorSize); i < end; ++i) {
						sample[i] = static_cast<typename SampleTraits::UnderlyingType>(reinterpret_cast<const Type*>(sampleBytes)[i]);
					}
				}
			};

			switch (dataTypeTraits.underlyingDataTypeValue) {
			case DataType::Int8:
				loopConvert.template operator()<typename DataTypeTraits<DataType::Int8>::Type>();
				break;
			case DataType::UInt8:
				loopConvert.template operator()<typename DataTypeTraits<DataType::UInt8>::Type>();
				break;
			case DataType::Int16:
				loopConvert.template operator()<typename DataTypeTraits<DataType::Int16>::Type>();
				break;
			case DataType::UInt16:
				loopConvert.template operator()<typename DataTypeTraits<DataType::UInt16>::Type>();
				break;
			case DataType::Int32:
				loopConvert.template operator()<typename DataTypeTraits<DataType::Int32>::Type>();
				break;
			case DataType::UInt32:
				loopConvert.template operator()<typename DataTypeTraits<DataType::UInt32>::Type>();
				break;
			case DataType::Int64:
				loopConvert.template operator()<typename DataTypeTraits<DataType::Int64>::Type>();
				break;
			case DataType::UInt64:
				loopConvert.template operator()<typename DataTypeTraits<DataType::UInt64>::Type>();
				break;
			case DataType::Float:
				loopConvert.template operator()<typename DataTypeTraits<DataType::Float>::Type>();
				break;
			case DataType::Double:
				loopConvert.template operator()<typename DataTypeTraits<DataType::Double>::Type>();
				break;
			case DataType::Bool:
				loopConvert.template operator()<typename DataTypeTraits<DataType::Bool>::Type>();
				break;
			default:
				sample = {};
				break;
			}
		}
	}
};

};

