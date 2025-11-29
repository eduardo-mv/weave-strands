/*
OpenGL 4.5 Sampler object
Wrapper for OpenGL 4.5 sampler objects
The implementation doesn't wrap every sampler option but provides convinient shortcuts to the most common operations.
*/
#pragma once

#include "weave/graphics/gl/gl46.h"
#include <string>

namespace weave::opengl {

class TextureSampler  {
private:
	//GL Sampler object id
	unsigned int gl_so = 0; 

public:
	TextureSampler() = default;
	TextureSampler(TextureSampler const&) = delete;
	TextureSampler(TextureSampler && other);
	TextureSampler& operator=(TextureSampler const&) = delete;
	TextureSampler& operator=(TextureSampler &&other);

	TextureSampler(unsigned int gl_filter[2], unsigned int gl_clamp[3], float anisotropy, unsigned int depthFunc, float *border);
	~TextureSampler();

	void SetState(unsigned int gl_filter[2], unsigned int gl_clamp[3], float anisotropy, unsigned int depthFunc, float *border);
	
	void SetFiltering(unsigned int gl_magfilter, unsigned int gl_minfilter);
	void SetClamping(unsigned int gl_clampS, unsigned int gl_clampT, unsigned int gl_clampR);
	void SetAnisotropy(float anis);
	void SetBorder(float color[4]);
	void SetDepthFunction(unsigned int gl_depthFunc, bool compareRefToTexture = true);

	//Returns the GL object id
	unsigned int GLId() const { return gl_so; }
	//Overwrites the internal gl_id with the one supplied. Useful to encapsulate externaly created samplers
	void OverrideId(unsigned int gl_id);

	//Binds the sampler to the given texture unit via GL
	void Bind(unsigned int tiu) const;

	//Returns a GL filter code for the two specified strings that define the filtering of the base texture and mipmaps
	static unsigned int MinFilterStringToGL(std::string const &filter, std::string const &mipmap);

protected:
	//Creates the sampler gl object if not already created
	void CreateGLObject();
};

}