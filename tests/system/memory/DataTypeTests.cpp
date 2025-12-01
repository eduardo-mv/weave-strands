#include "tests/TestEntryPoints.h"

#include "weave/system/memory/DataType.h"

#include <array>
#include <cstring>

namespace weave::tests::system::memory {

namespace wt = weave::types;

TestReport TestDataType() {
	TestReport report;

	{
		std::array<std::byte, sizeof(float) * 4> buffer{};
		std::array<float, 4> sample{ 1.0f, 2.0f, 3.0f, 4.0f };
		wt::DynamicTypeConvert(sample, buffer.data(), wt::DataType::Float_4);

		auto unpacked = wt::DynamicTypeConvert<std::array<float, 4>>(buffer.data(), wt::DataType::Float_4);
		for (size_t i = 0; i < sample.size(); ++i) {
			report.Expect(unpacked[i] == sample[i], "Float4 conversion mismatch");
		}
	}

	{
		uint32_t value = 42;
		std::array<std::byte, sizeof(uint32_t)> bytes{};
		wt::DynamicTypeConvert(value, bytes.data(), wt::DataType::UInt32);
		auto roundTrip = wt::DynamicTypeConvert<uint32_t>(bytes.data(), wt::DataType::UInt32);
		report.Expect(roundTrip == value, "UInt32 round trip failed");
	}

	{
		float sample = 5.5f;
		std::array<std::byte, sizeof(double)> bytes{};
		wt::DynamicTypeConvert(sample, bytes.data(), wt::DataType::Double);
		auto converted = wt::DynamicTypeConvert<double>(bytes.data(), wt::DataType::Double);
		report.Expect(converted == static_cast<double>(sample), "Float->Double conversion failed");
	}

	{
		std::array<float, 2> sample{ 7.0f, 8.0f };
		std::array<std::byte, sizeof(int32_t) * 2> bytes{};
		wt::DynamicTypeConvert(sample, bytes.data(), wt::DataType::Int32_2);
		auto ints = wt::DynamicTypeConvert<std::array<int32_t, 2>>(bytes.data(), wt::DataType::Int32_2);
		report.Expect(ints[0] == 7 && ints[1] == 8, "Float vec to Int vec conversion failed");
	}

	{
		std::array<uint8_t, 4> sample{ 255, 0, 128, 64 };
		std::array<std::byte, sizeof(float) * 4> bytes{};
		wt::DynamicTypeConvert(sample, bytes.data(), wt::DataType::Float_4);
		auto floats = wt::DynamicTypeConvert<std::array<float, 4>>(bytes.data(), wt::DataType::Float_4);
		report.Expect(floats[0] == 255.0f && floats[1] == 0.0f && floats[2] == 128.0f && floats[3] == 64.0f,
			"UInt8 vec to float vec conversion failed");
	}

	return report;
}

} // namespace weave::tests::system::memory

