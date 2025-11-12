#include "Utf.h"
#include <cstdint>

using namespace weave::utf;

#ifdef _WIN32
std::string weave::utf::Utf16toUtf8(std::wstring const& str) {
	return ConverterUtf8Utf16().to_bytes(str);
}
std::wstring weave::utf::Utf8toUtf16(std::string const& str) {
	return ConverterUtf8Utf16().from_bytes(str);
}

#endif

std::pair<uint32_t, size_t> weave::utf::Utf8touint32(char* bytes) {
	//Count the amount of MSB's at 1 on the first character to know the amount  of bytes required to read this character
	char ch = bytes[0];
	unsigned int count = 0;
	unsigned char mask = static_cast<unsigned char>(0x80);
	for (; ch & 0x80; ch <<= 1) {
		++count;
		mask = (mask >> 1 | 0x80); //This mask (inverted) is applied to the first byte later on to discard all unimportant bytes
	}

	//Dump the character bytes
	unsigned int widech = static_cast<unsigned int>(bytes[0] & (~mask));

	size_t byteOffset = 0;
	for (unsigned int i = 1; i < count; ++i) {
		widech <<= 6;
		widech |= static_cast<unsigned int>(bytes[i] & 0x3F);
		++byteOffset;
	}

	return { widech, byteOffset };
}

void weave::utf::SwapEndian(std::span<uint64_t> data) {
	for (uint64_t i = 0; i < data.size(); ++i) {
		uint64_t x = data[i];
		data[i] = (x << 56) | ((x & 0xFF00ul) << 40) | ((x & 0xFF0000ul) << 24) | ((x & 0xFF000000) << 8) |
			((x & 0xFF00000000ul) >> 8) | ((x & 0xFF0000000000ul) >> 24) | ((x & 0xFF0000000000ul) >> 40) | (x >> 56);
	}
}

void weave::utf::SwapEndian(std::span<uint32_t> data) {
	for (uint32_t i = 0; i < data.size(); ++i) {
		uint32_t x = data[i];
		data[i] = (x << 24) | ((x & 0xFF00) << 8) | ((x & 0xFF0000) >> 8) | (x >> 24);
	}
}

void weave::utf::SwapEndian(std::span<uint16_t> data) {
	for (uint16_t i = 0; i < data.size(); ++i) {
		uint16_t x = data[i];
		data[i] = (x << 8) | (x >> 8);
	}
}