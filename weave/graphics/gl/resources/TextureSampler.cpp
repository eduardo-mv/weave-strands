#include "TextureSampler.h"
#include <sstream>

using namespace weave::opengl;

TextureSampler::TextureSampler(TextureSampler&& other) : gl_so(other.gl_so) {
	other.gl_so = 0;
}

TextureSampler& TextureSampler::operator=(TextureSampler&& other) {
	std::swap(gl_so, other.gl_so);
	return *this;
}


TextureSampler::TextureSampler(unsigned int gl_filter[2], unsigned int gl_clamp[3], float anisotropy, unsigned int depthFunc, float *border) {
	SetState(gl_filter, gl_clamp, anisotropy, depthFunc, border);
}

TextureSampler::~TextureSampler() {
	gl::DeleteSamplers(1, &gl_so);
}

void TextureSampler::SetState(unsigned int gl_filter[2], unsigned int gl_clamp[3], float anisotropy, unsigned int depthFunc, float *border) {
	CreateGLObject();

	SetFiltering(gl_filter[0], gl_filter[1]);
	SetClamping(gl_clamp[0], gl_clamp[1], gl_clamp[2]);
	SetAnisotropy(anisotropy);
	if(border)
		SetBorder(border);
	if(depthFunc)
		SetDepthFunction(depthFunc, true);
}

void TextureSampler::SetFiltering(unsigned int gl_magfilter, unsigned int gl_minfilter) {
	CreateGLObject();

	gl::SamplerParameteri(gl_so, gl::TEXTURE_MAG_FILTER, gl_magfilter);
	gl::SamplerParameteri(gl_so, gl::TEXTURE_MIN_FILTER, gl_minfilter);
}

void TextureSampler::SetClamping(unsigned int gl_clampS, unsigned int gl_clampT, unsigned int gl_clampR) {
	CreateGLObject(); 
	
	gl::SamplerParameteri(gl_so, gl::TEXTURE_WRAP_S, gl_clampS);
	gl::SamplerParameteri(gl_so, gl::TEXTURE_WRAP_T, gl_clampT);
	gl::SamplerParameteri(gl_so, gl::TEXTURE_WRAP_R, gl_clampR);
}

void TextureSampler::SetAnisotropy(float anisotropy) {
	CreateGLObject();

	gl::SamplerParameterf(gl_so, gl::TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
}

void TextureSampler::SetBorder(float color[4]) {
	CreateGLObject(); 
	
	gl::TexParameterfv(gl::TEXTURE_2D, gl::TEXTURE_BORDER_COLOR, color);
}

void TextureSampler::SetDepthFunction(unsigned int gl_depthFunc, bool compareRefToTexture) {
	CreateGLObject(); 
	
	gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_COMPARE_MODE, compareRefToTexture ? gl::COMPARE_REF_TO_TEXTURE : gl::NONE);
	gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_COMPARE_FUNC, gl_depthFunc);
}

//Overwrites the internal gl_id with the one supplied. Useful to encapsulate externaly created samplers so that they can be registered on the resource base
void TextureSampler::OverrideId(unsigned int gl_id) {
	gl::DeleteSamplers(1, &gl_so);
	gl_so = gl_id;
}

//Binds the sampler to the given texture unit via GL
void TextureSampler::Bind(unsigned int tiu) const {
	gl::BindSampler(tiu, gl_so);
}

//Returns a GL filter code for the two specified strings that define  the filtering of the base texture and mipmaps
unsigned int TextureSampler::MinFilterStringToGL(std::string const & filter, std::string const & mipmap) {
	unsigned int gl_filter = gl::NEAREST;

	if(filter == "nearest") {
		if(mipmap == "nearest") {
			gl_filter = gl::NEAREST_MIPMAP_NEAREST;
		}
		else if(mipmap == "linear") {
			gl_filter = gl::NEAREST_MIPMAP_LINEAR;
		}
		else {
			gl_filter = gl::NEAREST;
		}
	}
	else if(filter == "linear") {
		if(mipmap == "nearest") {
			gl_filter = gl::LINEAR_MIPMAP_NEAREST;
		}
		else if(mipmap == "linear") {
			gl_filter = gl::LINEAR_MIPMAP_LINEAR;
		}
		else {
			gl_filter = gl::LINEAR;
		}
	}

	return gl_filter;
}

//Creates the sampler gl object if not already created
void TextureSampler::CreateGLObject() {
	if(!gl_so)
		gl::GenSamplers(1, &gl_so);
}
