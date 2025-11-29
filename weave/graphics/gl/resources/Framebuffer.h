/*
OpenGL Framebuffer
Encapsulation of an OpenGL Frame Buffer Object (FBO) with a set of helper methods.
*/
#pragma once
#include "weave/graphics/gl/gl46.h"

#include <limits>
#include <vector>
#include <unordered_map>
#include <string>

namespace weave::opengl {

class Framebuffer {
protected:
	//The glFramebuffer object id
	unsigned int gl_fbo = 0;
	//The attached color targets
	std::vector<std::pair<GLenum, unsigned int>> color_tbuffers;
	//The attached Depth / Stencil targets. The depth_target is used as depth and depth+stencil
	std::pair<GLenum, unsigned int> depth_target{}, stencil_target{};
	//The attached Renderbuffers kept only for cleanup purposes. Renderbuffers are gl-only objects
	unsigned int depth_rbuffer = 0, stencil_rbuffer = 0;

	//Holds the size of the viewport as defined by the width and height of the first color attachment
	unsigned int width = 0, height = 0;

public:
	~Framebuffer();

	//Created an empty GL FBO. If the FBO exists, no operation is performed
	unsigned int CreateFramebuffer();

	//Attaches a given target as a color attachment. The first parameter determines the color attachment number
	//location indicates the output location on the shader, level the mipmap level and layer the layer within the texture (for arrays, cubes, etc). Setting layer to negative will bind the whole
	//layered texture as a layared framebuffer instead of a single layer of the texture.
	unsigned int AttachColorTarget(std::pair<GLenum, unsigned int> target, unsigned int location = 0, int gl_level = 0, int gl_layer = -1);
	//Attaches a given target as a depth attachment
	unsigned int AttachDepthTarget(std::pair<GLenum, unsigned int> target, int gl_level = 0, int gl_layer = -1);
	//Attaches a given target as a depth and stencil attachment
	unsigned int AttachDepthStencilTarget(std::pair<GLenum, unsigned int> target, int gl_level = 0, int gl_layer = -1);
	//Attaches a given target as a stencil attachment
	unsigned int AttachStencilTarget(std::pair<GLenum, unsigned int> target, int gl_level = 0, int gl_layer = -1);

	//Automatically generates a Renderbuffer for the Depth attachment. Width and Height can be specified but if set to 0 it will automatically generate one based on the attachment for the
	//color buffer #0. If there's no attachment at 0 it will try every possible color attachment until one is found. If there's no color attachment the call will fail to attach a buffer unless
	//the size is specified.
	unsigned int AttachDepthBuffer(bool depth32 = false, unsigned int width = 0, unsigned int height = 0);
	//Automatically generates a Renderbuffer for the Depth and Stencil attachment. Width and Height can be specified but if set to 0 it will automatically generate one based on the attachment for the
	//color buffer #0. If there's no attachment at 0 it will try every possible color attachment until one is found. If there's no color attachment the call will fail to attach a buffer unless
	//the size is specified.
	unsigned int AttachDepthStencilBuffer(bool depth32 = false, unsigned int width = 0, unsigned int height = 0);
	//Automatically generates a Renderbuffer for the Depth and Stencil attachment. Width and Height can be specified but if set to 0 it will automatically generate one based on the attachment for the
	//color buffer #0. If there's no attachment at 0 it will try every possible color attachment until one is found. If there's no color attachment the call will fail to attach a buffer unless
	//the size is specified.
	unsigned int AttachStencilBuffer(unsigned int width = 0, unsigned int height = 0);

	//Attaches a render buffer given the opengl Id
	unsigned int AttachSharedBuffer(unsigned int gl_id, unsigned int gl_attachment);

	//Sets the width and height of the FBO
	void SetSize(unsigned int w, unsigned int h);

	//Retreives the nth color attachment as a RenderTarget
	std::pair<GLenum, unsigned int> GetColorTarget(unsigned int location = 0) const;
	//Returns the depth (and depth+stencil) attachment
	std::pair<GLenum, unsigned int> GetDepthTarget() const { return depth_target; }
	//Returns the depth (and depth+stencil) attachment
	std::pair<GLenum, unsigned int> GetDepthStencilTarget() const { return depth_target; }
	//Returns the stencil attachment
	std::pair<GLenum, unsigned int> GetStencilTarget() const { return stencil_target; }

	//Binds the FBO and sets the viewport to the correct size for the GLFramebuffer
	void Bind() const;
	void Bind(unsigned int w, unsigned int h) const;
	//Unbinds the FBO by binding back the default framebuffer and returns the viewport to the stored viewport size from the previous Bind() call
	void Unbind();

	//Returns the gl id for the FBO
	unsigned int GLId() const { return gl_fbo; }
	unsigned int GetDepthRenderbuffer() const { return depth_rbuffer; }
	unsigned int GetStencilRenderbuffer() const { return stencil_rbuffer; }

	unsigned int Width() const { return width; }
	unsigned int Height() const { return height; }

	//Returns the enum given by gl::CheckFramebufferStatus(gl::DRAW_FRAMEBUFFER)
	unsigned int CheckStatus(bool debugAssert = false) const;
	std::string CheckStatusString() const;

	//Generates mipmaps through GL for all color targets
	void GenerateMipmaps() const;

protected:
	unsigned int AttachRenderbuffer(unsigned int width, unsigned int height, unsigned int gl_attachment, unsigned int gl_format);
	unsigned int AttachTarget(std::pair<GLenum, unsigned int> target, unsigned int gl_attachment, int gl_level, int layer);
	void FindViewportSize();
};

}