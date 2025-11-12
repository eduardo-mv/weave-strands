#include "ImageData.h"
#include <cstring>

using namespace weave;
using namespace graphics;

bool ImageData::IsValid() const {
	return data.size() && width && height && layerCount && mipmapCount;
}

uint32_t ImageData::GetFullByteSize() const
{
	return GetLayerByteSize() * layerCount;
}

uint32_t ImageData::GetLayerByteSize() const
{
	uint32_t layerSize = 0;
	for (uint32_t i = 0; i < mipmapCount; ++i) {
		layerSize += GetByteSize(i);
	}

	return layerSize;
}

uint32_t ImageData::GetByteSize(uint32_t mipLevel)  const {
	if (compCount == 0) {
		uint32_t linew = ((GetWidth(mipLevel) + 3) / 4);
		uint32_t lineh = ((GetHeight(mipLevel) + 3) / 4);
		uint32_t lined = ((GetDepth(mipLevel) + 3) / 4);
		return linew * lineh * lined * compByteSize;
	}
	else {
		return GetWidth(mipLevel) * GetHeight(mipLevel) * GetDepth(mipLevel) * compCount * compByteSize;
	}
}

uint32_t ImageData::GetWidth(uint32_t mipLevel)  const {
	return std::max(width >> mipLevel, 1u);
}

uint32_t ImageData::GetHeight(uint32_t mipLevel) const {
	return std::max(height >> mipLevel, 1u);
}

//Returns the depth size of an image. Level indicates the mipmap.
uint32_t ImageData::GetDepth(uint32_t mipLevel) const {
	return std::max(depth >> mipLevel, 1u);
}

uint32_t ImageData::GetGLDataType() const {
	switch (format) {
	case weave::vulkan::ImageFormat::BC1_RGBA_SRGB_BLOCK:
	case weave::vulkan::ImageFormat::BC1_RGBA_UNORM_BLOCK:
	case weave::vulkan::ImageFormat::BC1_RGB_SRGB_BLOCK:
	case weave::vulkan::ImageFormat::BC1_RGB_UNORM_BLOCK:
		return static_cast<uint32_t>(weave::opengl::GlFormat::COMPRESSED_RGB_S3TC_DXT1_EXT);

	case weave::vulkan::ImageFormat::BC2_SRGB_BLOCK:
	case weave::vulkan::ImageFormat::BC2_UNORM_BLOCK:
		return static_cast<uint32_t>(weave::opengl::GlFormat::COMPRESSED_RGBA_S3TC_DXT3_EXT);

	case weave::vulkan::ImageFormat::BC3_SRGB_BLOCK:
	case weave::vulkan::ImageFormat::BC3_UNORM_BLOCK:
		return static_cast<uint32_t>(weave::opengl::GlFormat::COMPRESSED_RGBA_S3TC_DXT5_EXT);

	case weave::vulkan::ImageFormat::R8G8B8A8_SINT:
	case weave::vulkan::ImageFormat::R8G8B8A8_SNORM:
	case weave::vulkan::ImageFormat::R8G8B8A8_SRGB:
	case weave::vulkan::ImageFormat::R8G8B8A8_SSCALED:
	case weave::vulkan::ImageFormat::R8G8B8A8_UINT:
	case weave::vulkan::ImageFormat::R8G8B8A8_UNORM:
	case weave::vulkan::ImageFormat::R8G8B8A8_USCALED:
		return static_cast<uint32_t>(weave::opengl::GlFormat::RGBA);

	case weave::vulkan::ImageFormat::R8G8B8_SINT:
	case weave::vulkan::ImageFormat::R8G8B8_SNORM:
	case weave::vulkan::ImageFormat::R8G8B8_SRGB:
	case weave::vulkan::ImageFormat::R8G8B8_SSCALED:
	case weave::vulkan::ImageFormat::R8G8B8_UINT:
	case weave::vulkan::ImageFormat::R8G8B8_UNORM:
	case weave::vulkan::ImageFormat::R8G8B8_USCALED:
		return static_cast<uint32_t>(weave::opengl::GlFormat::RGB);

	case weave::vulkan::ImageFormat::R8G8_SINT:
	case weave::vulkan::ImageFormat::R8G8_SNORM:
	case weave::vulkan::ImageFormat::R8G8_SRGB:
	case weave::vulkan::ImageFormat::R8G8_SSCALED:
	case weave::vulkan::ImageFormat::R8G8_UINT:
	case weave::vulkan::ImageFormat::R8G8_UNORM:
	case weave::vulkan::ImageFormat::R8G8_USCALED:
		return static_cast<uint32_t>(weave::opengl::GlFormat::RG);

	case weave::vulkan::ImageFormat::R8_SINT:
	case weave::vulkan::ImageFormat::R8_SNORM:
	case weave::vulkan::ImageFormat::R8_SRGB:
	case weave::vulkan::ImageFormat::R8_SSCALED:
	case weave::vulkan::ImageFormat::R8_UINT:
	case weave::vulkan::ImageFormat::R8_UNORM:
	case weave::vulkan::ImageFormat::R8_USCALED:
		return static_cast<uint32_t>(weave::opengl::GlFormat::RED);

	case weave::vulkan::ImageFormat::B8G8R8A8_SINT:
	case weave::vulkan::ImageFormat::B8G8R8A8_SNORM:
	case weave::vulkan::ImageFormat::B8G8R8A8_SRGB:
	case weave::vulkan::ImageFormat::B8G8R8A8_SSCALED:
	case weave::vulkan::ImageFormat::B8G8R8A8_UINT:
	case weave::vulkan::ImageFormat::B8G8R8A8_UNORM:
	case weave::vulkan::ImageFormat::B8G8R8A8_USCALED:
		return static_cast<uint32_t>(weave::opengl::GlFormat::BGRA);

	case weave::vulkan::ImageFormat::B8G8R8_SINT:
	case weave::vulkan::ImageFormat::B8G8R8_SNORM:
	case weave::vulkan::ImageFormat::B8G8R8_SRGB:
	case weave::vulkan::ImageFormat::B8G8R8_SSCALED:
	case weave::vulkan::ImageFormat::B8G8R8_UINT:
	case weave::vulkan::ImageFormat::B8G8R8_UNORM:
	case weave::vulkan::ImageFormat::B8G8R8_USCALED:
		return static_cast<uint32_t>(weave::opengl::GlFormat::BGR);

	case weave::vulkan::ImageFormat::R16G16B16A16_SNORM:
	case weave::vulkan::ImageFormat::R16G16B16A16_SSCALED:
	case weave::vulkan::ImageFormat::R16G16B16A16_UINT:
	case weave::vulkan::ImageFormat::R16G16B16A16_UNORM:
	case weave::vulkan::ImageFormat::R16G16B16A16_USCALED:
		return static_cast<uint32_t>(weave::opengl::GlFormat::RG16);

	default:
		return 0;
	}
}

uint32_t ImageData::GetVkFormat() const
{
	return static_cast<uint32_t>(format);
}

ImageData::SurfaceData const ImageData::GetSurface(uint32_t layer, uint32_t mipLevel) const {
	if (layer >= layerCount || mipLevel >= mipmapCount)
		return {};

	//Each set of mipmaps, representing a layer, is set contiguously on the data buffer
	uint32_t layerOffset = GetLayerByteSize() * layer;
	uint32_t mipOffset = 0;
	for (uint32_t i = 0; i < mipLevel; ++i) {
		mipOffset += GetByteSize(i);
	}

	return SurfaceData{
		GetWidth(mipLevel),
		GetHeight(mipLevel),
		GetDepth(mipLevel),
		layer,
		mipLevel,
		compCount,
		compByteSize,
		std::span<uint8_t>{&data[layerOffset + mipOffset], GetByteSize(mipLevel)}
	};
}

ImageData::SurfaceData ImageData::Surface(uint32_t surface, uint32_t level) {
	return GetSurface(surface, level);
}

//Returns an RGBA pixel value at a given position for a given layer and level
bool ImageData::GetRGBAPixel(uint32_t layer, uint32_t mipLevel, uint32_t x, uint32_t y, uint32_t z, uint8_t rgba[4]) const {
	if (layer >= layerCount || compCount == 0 || x > GetWidth(mipLevel) || y > GetHeight(mipLevel) || z > GetDepth(mipLevel))
		return false;

	auto surface = GetSurface(layer, mipLevel);

	uint32_t compBytes = compCount * compByteSize;
	uint32_t pos = (z * surface.width * surface.height + y * surface.width + x) * compBytes;
	rgba[0] = surface.data[pos];
	rgba[1] = surface.data[pos + std::min(compBytes - 1, 1u)];
	rgba[2] = surface.data[pos + std::min(compBytes - 1, 2u)];
	rgba[3] = surface.data[pos + std::min(compBytes - 1, 3u)];

	return true;
}

//Faster access to pixel data where layer and level are assumed to be 0 and no bounds check are done
void ImageData::GetRGBAPixelFast(uint32_t x, uint32_t y, uint8_t rgba[4]) const {
	assert(compCount != 0); //This is a compressed image, this shouldn't be called!

	uint32_t compBytes = compCount * compByteSize;

	uint32_t pos = (y * width + x) * compBytes;
	rgba[0] = data[pos];
	rgba[1] = data[pos + std::min(compBytes - 1, 1u)];
	rgba[2] = data[pos + std::min(compBytes - 1, 2u)];
	rgba[3] = data[pos + std::min(compBytes - 1, 3u)];
}

void ImageData::FlipVertical() {
	//Flip all the layers and mipmaps
	uint32_t compBytes = compCount * compByteSize;
	uint32_t maxLineSize = width * compBytes;
	std::vector<uint8_t> line;
	line.resize(maxLineSize);
	
	for (uint32_t layer = 0; layer < layerCount; ++layer) {
		for(uint32_t level = 0; level < mipmapCount; ++level) {

			auto surface = Surface(layer, level);
			uint32_t lineSize = surface.width * compBytes;

			for (uint32_t i = 0, j = surface.height - 1; i < j; ++i, --j) {
				memcpy(line.data(), &surface.data[j * lineSize], lineSize);
				memcpy(&surface.data[j * lineSize], &surface.data[i * lineSize], lineSize);
				memcpy(&surface.data[i * lineSize], line.data(), lineSize);
			}
		}
	}
}

#ifdef _WIN32
//Converts the first layer of the stored image to a HBITMAP
//@alphaMask: returns a black and white image based on the alpha channel
HBITMAP ImageData::MakeWin32Bitmap(uint32_t layer, uint32_t mipLevel, bool alphaMask) const {
	//Cannot convert compressed images :(
	if (IsCompressed())
		return NULL;

	HDC hDC = ::GetDC(NULL);
	HDC hBitmapDC = CreateCompatibleDC(hDC);

	//Get the dimensions of the source bitmap
	HBITMAP bitmap = CreateCompatibleBitmap(hDC, GetWidth(layer), GetHeight(layer));

	//Select the bitmaps to DC
	SelectObject(hBitmapDC, bitmap);

	//Scan each pixel of the souce bitmap and create the masks
	uint32_t compBytes = compCount * compByteSize;
	auto img = GetSurface(layer, mipLevel);

	for (uint32_t j = 0; j < img.height; ++j) {
		for (uint32_t i = 0; i < img.width; ++i) {
			uint32_t pos = j * img.width * compBytes + i * compBytes;
			uint8_t r = img.data[pos];
			uint8_t g = img.data[pos + std::min(compBytes - 1, 1u)];
			uint8_t b = img.data[pos + std::min(compBytes - 1, 2u)];
			uint8_t a = img.data[pos + std::min(compBytes - 1, 3u)];

			if (a < 255 && compCount > 3) {
				r = g = b = 0;
				a = 255;
			}
			else {
				a = 0;
			}

			COLORREF pixel = (alphaMask ? RGB(a, a, a) : RGB(r, g, b));
			SetPixel(hBitmapDC, int(i), int(j), pixel);
		}
	}

	DeleteDC(hBitmapDC);
	ReleaseDC(NULL, hDC);

	return bitmap;
}

//Converts the first layer of the stored image into a usable HCURSOR
HCURSOR ImageData::MakeWin32Cursor(uint32_t layer, uint32_t mipLevel, DWORD xHotspot, DWORD yHotspot) const {
	HCURSOR hRetCursor = NULL;

	//Create the AND and XOR masks for the bitmap
	HBITMAP hAlphaMask = MakeWin32Bitmap(layer, mipLevel, true);
	HBITMAP hColorMask = MakeWin32Bitmap(layer, mipLevel, false);

	//Create the cursor using the masks and the hotspot values provided
	ICONINFO iconinfo{};
	iconinfo.fIcon = FALSE;
	iconinfo.xHotspot = xHotspot;
	iconinfo.yHotspot = yHotspot;
	iconinfo.hbmMask = hAlphaMask;
	iconinfo.hbmColor = hColorMask;

	hRetCursor = ::CreateIconIndirect(&iconinfo);

	return hRetCursor;
}
#endif

void ImageData::ConvertLayersTo3D() {
	if (depth > 1 || layerCount <= 1) {
		return;
	}

	std::vector<uint8_t> data3d;
	data3d.reserve(data.size());

	for (uint32_t mip = 0; mip < mipmapCount; ++mip) {
		for (uint32_t layer = 0; layer < layerCount; ++layer) {
			auto surface = GetSurface(layer, mip);
			data3d.insert(std::end(data3d), std::begin(surface.data), std::end(surface.data));
		}
	}

	depth = layerCount;
	layerCount = 1;

	std::swap(data, data3d);
}


bool ImageData::MergeWith(ImageData const& otherImage) {
	//Only identical images can be merged. Any amount of layers is valid
	if (width == otherImage.width && 
		height == otherImage.height && 
		depth == otherImage.depth && 
		mipmapCount == otherImage.mipmapCount && 
		compCount == otherImage.compCount && 
		compByteSize == otherImage.compByteSize &&
		format == otherImage.format) {

		layerCount += otherImage.layerCount;

		data.reserve(data.size() + otherImage.data.size());
		data.insert(data.end(), otherImage.data.begin(), otherImage.data.end());

		return true;
	}

	return false;
}
