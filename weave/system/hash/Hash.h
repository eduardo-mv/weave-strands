/*
Hash
Implementation an STL compatible adapter for MurmurHash3
*/
#pragma once

#include "MurmurHash3.h"
#include <cassert>
#include <string>
#include <string_view>
#include <vector>

namespace weave {

	//A no hashing function, useful for maps with prehashed keys so that they key is used directly instead of hashed again.
	class NoHash {
	public:
		size_t operator()(size_t const &key) const {
			return key;
		}
	};

	//A no hashing function, useful for maps with prehashed keys so that they key is used directly instead of hashed again.
	//This version is useful for 64bit prehashed keys and will default to the std::hash if the size of the key is not 64bit
	class NoHash64 {
		
	public:
		template<int = sizeof(size_t)>
		size_t operator()(uint64_t const &key) const {
			static std::hash<uint64_t> hasher;
			return hasher(key);
		}
	};

	template<>
	inline size_t NoHash64::operator()<8>(uint64_t const &key) const {
		return size_t(key);
	}


	//Base template for Murmur3
	/*
	keyType: The type of data to generate a hash for
	returnType: The return type wished for the hash. For good results it should be an unsigned integer of 4 or 8 bytes
	seed: The base seed for the hash. Defaults at 13579
	int: The size of the returnType. Should be 4 or 8 bytes. Automatically set to sizeof(returnType).
	*/
	template<typename keyType, typename returnType = size_t, int seed = 13579, int = sizeof(returnType)>
	class MurmurHash3 {
	public:
		returnType operator()(keyType const &key) const;
	};

	//Generic keyType, 4 byte return hash
	template<typename keyType, typename returnType, int seed>
	class MurmurHash3<keyType, returnType, seed, 4> {
	public:
		returnType operator()(keyType const &key) const {
			//Hash a 128 bit and return only 64bit to fit a size_t
			uint32_t hash;
			MurmurHash3_x86_32(&key, sizeof(keyType), seed, &hash);
			return hash;
		}
	};

	//Generic keyType, 8 byte return hash
	template<typename keyType, typename returnType, int seed>
	class MurmurHash3<keyType, returnType, seed, 8> {
	public:
		returnType operator()(keyType const &key) const {
			//Hash a 128 bit and return only 64bit to fit a size_t
			uint64_t hash[2];
			MurmurHash3_x64_128(&key, sizeof(keyType), seed, hash);
			return hash[0];
		}
	};

	//Specialization for std::string
	template<typename returnType, int seed>
	class MurmurHash3<std::string, returnType, seed, 4> {
	public:
		returnType operator()(std::string const &key) const {
			//Hash a 128 bit and return only 64bit to fit a size_t
			uint32_t hash;
			MurmurHash3_x86_32(key.data(), (int)key.length(), seed, &hash);
			return hash;
		}
	};

	template<typename returnType, int seed>
	class MurmurHash3<std::string, returnType, seed, 8> {
	public:
		returnType operator()(std::string const &key) const {
			//Hash a 128 bit and return only 64bit to fit a size_t
			uint64_t hash[2];
			MurmurHash3_x64_128(key.data(), (int)key.length(), seed, hash);
			return hash[0];
		}

	};

	//Specialization for std::wstring (wide characters)
	template<typename returnType, int seed>
	class MurmurHash3<std::wstring, returnType, seed, 4> {
	public:
		returnType operator()(std::wstring const &key) const {
			//Hash a 128 bit and return only 64bit to fit a size_t
			uint32_t hash;
			MurmurHash3_x86_32(key.data(), (int)key.length() * sizeof(wchar_t), seed, &hash);
			return hash;
		}
	};

	template<typename returnType, int seed>
	class MurmurHash3<std::wstring, returnType, seed, 8> {
	public:
		returnType operator()(std::wstring const &key) const {
			//Hash a 128 bit and return only 64bit to fit a size_t
			uint64_t hash[2];
			MurmurHash3_x64_128(key.data(), (int)key.length() * sizeof(wchar_t), seed, hash);
			return hash[0];
		}

	};

	//Specialization for std::string_view
	template<typename returnType, int seed>
	class MurmurHash3<std::string_view, returnType, seed, 4> {
	public:
		returnType operator()(std::string const& key) const {
			//Hash a 128 bit and return only 64bit to fit a size_t
			uint32_t hash;
			MurmurHash3_x86_32(key.data(), (int)key.length(), seed, &hash);
			return hash;
		}
	};

	template<typename returnType, int seed>
	class MurmurHash3<std::string_view, returnType, seed, 8> {
	public:
		returnType operator()(std::string_view const& key) const {
			//Hash a 128 bit and return only 64bit to fit a size_t
			uint64_t hash[2];
			MurmurHash3_x64_128(key.data(), (int)key.length(), seed, hash);
			return hash[0];
		}

	};

	//Support inline function that hashes a string with a default hasher
	inline uint64_t HashString(std::string const &name) {
		static weave::MurmurHash3<std::string, uint64_t> hash;
		return hash(name);
	}

	inline uint64_t HashString(std::string_view name) {
		static weave::MurmurHash3<std::string_view, uint64_t> hash;
		return hash(name);
	}

	inline uint64_t HashString(char const *name) {
		return HashString(std::string_view{ name });
	}

	inline uint64_t HashData(char const* data, size_t byteSize) {
		return HashString(std::string_view{ data, byteSize });
	}

	template<typename Type>
	inline uint64_t HashData(std::vector<Type> const &vector) {
		return HashString(std::string_view{ reinterpret_cast<char const*>(vector.data()), vector.size() * sizeof(Type) });
	}
}
