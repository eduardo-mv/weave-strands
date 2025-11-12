/*
UTF8 support functions
*/
#pragma once

#include <codecvt>
#include <locale>
#include <span>
#include <cstdint>

namespace weave::utf {
#ifdef _WIN32
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning(disable: 4996) //4996: Deprecated wstring_convert in C++17
#endif
	//Converter adapter. from_bytes: utf8 -> utf16  / to_bytes: utf16 -> utf8
	using ConverterUtf8Utf16 = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>; 

	std::string Utf16toUtf8(std::wstring const& str);
	std::wstring Utf8toUtf16(std::string const& str);
#endif
	//Helper tool to convert a multi byte UTF-8 character into a full 32bit representation. Returns the converted character and the byteOffset offset to apply to the original string
	std::pair<uint32_t, size_t> Utf8touint32(char* bytes);

	//Swaps endianness of 64bit values
	void SwapEndian(std::span<uint64_t> data);

	//Swaps endianness of 32bit values
	void SwapEndian(std::span<uint32_t> data);

	//Swap endianness of 16bit values
	void SwapEndian(std::span<uint16_t> data);
}

//#ifdef _MSC_VER
//#pragma warning(pop)
//#endif
