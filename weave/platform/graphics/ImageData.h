/*
Weave Image Data
Image data holder with support for multiple layers and mipmaps
*/

#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>
#include <span>
#include "vulkan/vk_format.h"

namespace weave::graphics {
	class ImageLoader;

class ImageData {
	friend class ImageLoader;
private:
	uint32_t width = 0, height = 0, depth = 0; //3D size of the image. Images with missing dimensions have those set to 1
	uint32_t layerCount = 0; //Number of layers (e.g., 6 on a cubemap)
	uint32_t mipmapCount = 0; //Number of mipmap levels per layer
	uint32_t compCount = 0; //Number of components in the image
	uint32_t compByteSize = 0; //Byte size per component in the image. Represents the block size if compCount == 0

	weave::vulkan::ImageFormat format = {}; //Vulkan VkFormat compatible image format

	mutable std::vector<uint8_t> data; //Image data, containing all layers and mipmaps. Every layer is followed by its mipmap set

public:
	struct SurfaceData {
		uint32_t width{}, height{}, depth{};
		uint32_t layer{};
		uint32_t mipLevel{};
		uint32_t compCount{};
		uint32_t compByteSize{};
		std::span<uint8_t> data;
	};

public:
	ImageData() = default;
	ImageData(ImageData const& other) = default;
	ImageData(ImageData && other) = default;

	ImageData& operator=(ImageData const& other) = default;
	ImageData& operator=(ImageData && other) = default;

	~ImageData() = default;

	//Returns true if the image contains valid data
	bool IsValid() const;
	
	bool IsCompressed() const { return compCount == 0; }

	//Returns the maximum number of mipmaps this image could have
	uint32_t MaxMipmaps() const { return static_cast<uint32_t>(std::floor(std::log2(std::max(GetWidth(0), GetHeight(0))))) + 1; }

	uint32_t GetFullByteSize() const;
	uint32_t GetLayerByteSize() const;

	uint32_t GetByteSize(uint32_t mipLevel) const;
	uint32_t GetWidth(uint32_t mipLevel) const;
	uint32_t GetHeight(uint32_t mipLevel) const;
	uint32_t GetDepth(uint32_t mipLevel) const;
	
	uint32_t GetNumLayers() const { return layerCount; }
	uint32_t GetNumMipmaps() const { return mipmapCount; }
	
	uint32_t GetComponentCount() const { return compCount; }
	uint32_t GetComponentByteSize() const { return compByteSize; }

	weave::vulkan::ImageFormat GetImageFormat() const { return format; }
	//Returns a GL_* enum with the equivalent data type for the image
	uint32_t GetGLDataType() const;
	//Returns the VK_FORMAT data type for the image
	uint32_t GetVkFormat() const;

	//Returns a byte array with image data for the specified layer and mipmap level
	SurfaceData const GetSurface(uint32_t layer, uint32_t mipLevel) const;
	SurfaceData Surface(uint32_t layer, uint32_t mipLevel);

	//Returns an RGBA pixel value at a given position for a given layer and level
	//Returns false if the layer specified does not exist or the image is compressed
	bool GetRGBAPixel(uint32_t layer, uint32_t mipLevel, uint32_t x, uint32_t y, uint32_t z, uint8_t rgba[4]) const;
	//Faster access to pixel data where layer and level are assumed to be 0 and no bounds check are done
	void GetRGBAPixelFast(uint32_t x, uint32_t y, uint8_t rgba[4]) const;
	
	//Flips the image data vertically
	void FlipVertical();

#ifdef _WIN32
	//Converts the first layer of the stored image to a HBITMAP
	//@alphaMask: returns a black and white image based on the alpha channel
	HBITMAP MakeWin32Bitmap(uint32_t layer, uint32_t mipLevel, bool alphaMask) const;
	//Converts the first layer of the stored image into a usable HCURSOR
	HCURSOR MakeWin32Cursor(uint32_t layer, uint32_t mipLevel, DWORD xHotspot, DWORD yHotspot) const;
#endif
	//Converts all image layers into a 3D image. Will only work on layered 2D images, where all layers will
	//be converted into 3D depth 
	void ConvertLayersTo3D();
	
	//Merges the supplied image as a layer of the current image, if the images match
	bool MergeWith(ImageData const& otherImage);

	template<typename ...Images>
	static ImageData MergeImages(ImageData const& first, Images &&...images) {
		ImageData out = first;
		MergeNextImage(out, images...);
		return out;
	}

private:
	template<typename ...Images>
	static void MergeNextImage(ImageData& into, ImageData const& first, Images &&...images) {
		into.MergeWith(first);
		MergeNextImage(into, images...);
	}

	template<typename ...Images>
	static void MergeNextImage(ImageData&) {
	}
	
};

}