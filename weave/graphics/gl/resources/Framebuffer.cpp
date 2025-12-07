#include "Framebuffer.h"
#include "ApiHelpers.h"
#include <cassert>

using namespace weave;
using namespace weave::opengl;

Framebuffer::~Framebuffer() {
	//Delete the gl Framebuffer object
	gl::DeleteFramebuffers(1, &gl_fbo);

	//Delete all the gl Renderbuffers
	gl::DeleteRenderbuffers(1, &depth_rbuffer);
	gl::DeleteRenderbuffers(1, &stencil_rbuffer);
}

unsigned int Framebuffer::CreateFramebuffer() {
	//Don't remake the Framebuffer if it's already created
	if(gl_fbo)
		return gl_fbo;

	gl::GenFramebuffers(1, &gl_fbo);

	return gl_fbo;
}

unsigned int Framebuffer::AttachColorTarget(std::pair<GLenum, unsigned int> target, unsigned int location, int gl_level, int gl_layer) {
	if(location >= color_tbuffers.size())
		color_tbuffers.resize(location+1);

	if(!target.second) {
		color_tbuffers.erase(std::begin(color_tbuffers) + location);
	}
	else {
		color_tbuffers[location] = target;
	}

	AttachTarget(target, gl::COLOR_ATTACHMENT0 + location, gl_level, gl_layer);
	FindViewportSize();
	return gl_fbo;
}

unsigned int Framebuffer::AttachDepthTarget(std::pair<GLenum, unsigned int> target, int gl_level, int gl_layer) {
	depth_target = target;
	return AttachTarget(target, gl::DEPTH_ATTACHMENT, gl_level, gl_layer);
}

unsigned int Framebuffer::AttachDepthStencilTarget(std::pair<GLenum, unsigned int> target, int gl_level, int gl_layer) {
	depth_target = target;
	return AttachTarget(target, gl::DEPTH_STENCIL_ATTACHMENT, gl_level, gl_layer);
}

unsigned int Framebuffer::AttachStencilTarget(std::pair<GLenum, unsigned int> target, int gl_level, int gl_layer) {
	stencil_target = target;
	return AttachTarget(target, gl::STENCIL_ATTACHMENT, gl_level, gl_layer);
}

unsigned int Framebuffer::AttachTarget(std::pair<GLenum, unsigned int> target, unsigned int gl_attachment, int gl_level, int gl_layer) {
	CreateFramebuffer();
	
	Bind();

	//If the supplied target is null, unbind the target from the FBO else bind the new target to the requested attachment point
	if(!target.second)
		gl::FramebufferTexture(gl::DRAW_FRAMEBUFFER, gl_attachment, 0, 0);
	else {
		if(weave::opengl::IsLayeredTarget(target.first) && gl_layer >= 0)
			gl::FramebufferTextureLayer(gl::DRAW_FRAMEBUFFER, gl_attachment, target.second, gl_level, gl_layer);
		else
			gl::FramebufferTexture(gl::DRAW_FRAMEBUFFER, gl_attachment, target.second, gl_level);
	}

	Unbind();

	return gl_fbo;
}

//Automatically generates a Renderbuffer for the Depth attachment. Width and Height can be specified but if set to 0 it will automatically generate one based on the attachment for the
//color buffer #0. If there's no attachment at 0 it will try every possible color attachment until one is found. If there's no color attachment the call will fail to attach a buffer unless
//the size is specified.
unsigned int Framebuffer::AttachDepthBuffer(bool depth32, unsigned int fbWidth, unsigned int fbHeight) {
	return AttachRenderbuffer(fbWidth, fbHeight, gl::DEPTH_ATTACHMENT, depth32 ? gl::DEPTH_COMPONENT32F : gl::DEPTH_COMPONENT24);
}

//Automatically generates a Renderbuffer for the Depth and Stencil attachment. Width and Height can be specified but if set to 0 it will automatically generate one based on the attachment for the
//color buffer #0. If there's no attachment at 0 it will try every possible color attachment until one is found. If there's no color attachment the call will fail to attach a buffer unless
//the size is specified.
unsigned int Framebuffer::AttachDepthStencilBuffer(bool depth32, unsigned int fbWidth, unsigned int fbHeight) {
	return AttachRenderbuffer(fbWidth, fbHeight, gl::DEPTH_STENCIL_ATTACHMENT, depth32 ? gl::DEPTH32F_STENCIL8 : gl::DEPTH24_STENCIL8);
}

//Automatically generates a Renderbuffer for the Depth and Stencil attachment. Width and Height can be specified but if set to 0 it will automatically generate one based on the attachment for the
//color buffer #0. If there's no attachment at 0 it will try every possible color attachment until one is found. If there's no color attachment the call will fail to attach a buffer unless
//the size is specified.
unsigned int Framebuffer::AttachStencilBuffer(unsigned int fbWidth, unsigned int fbHeight) {
	return AttachRenderbuffer(fbWidth, fbHeight, gl::STENCIL_ATTACHMENT, gl::STENCIL_INDEX8);
}

unsigned int Framebuffer::AttachRenderbuffer(unsigned int fbWidth, unsigned int fbHeight, unsigned int gl_attachment, unsigned int gl_format) {
	//If there's no framebuffer created, make one
	CreateFramebuffer();
	
	//Check the size of the requested buffer
	if(fbWidth == 0 || fbHeight == 0) {
		//Find out the size of the viewport
		FindViewportSize();

		//If no size was found, return. The Renderbuffer cannot be created
		if(width == 0 || height == 0) {
			return 0;
		}
		else {
			fbWidth = width;
			fbHeight = height;
		}
	}

	//Bind the FBO
	Bind();

	unsigned int &rbuffer = (gl_attachment == gl::STENCIL_ATTACHMENT ? stencil_rbuffer : depth_rbuffer);

	//Delete the previous renderbuffer, if there was any
	gl::DeleteRenderbuffers(1, &rbuffer);
	//Create the gl renderbuffer
	gl::GenRenderbuffers(1, &rbuffer);
	gl::BindRenderbuffer(gl::RENDERBUFFER, rbuffer);
	gl::RenderbufferStorage(gl::RENDERBUFFER, gl_format, fbWidth, fbHeight);
	//Bind it to the FBO
	gl::FramebufferRenderbuffer(gl::DRAW_FRAMEBUFFER, gl_attachment, gl::RENDERBUFFER, rbuffer);

	//Revert the previous binding
	Unbind();

	return gl_fbo;
}

//Attaches a render buffer given the opengl ID
unsigned int Framebuffer::AttachSharedBuffer(unsigned int gl_renderbufferid, unsigned int gl_attachment) {
	//Bind the FBO
	Bind();
	gl::FramebufferRenderbuffer(gl::DRAW_FRAMEBUFFER, gl_attachment, gl::RENDERBUFFER, gl_renderbufferid);
	Unbind();

	return gl_fbo;
}

//Sets the width and height of the FBO
void Framebuffer::SetSize(unsigned int w, unsigned int h) {
	width = w;
	height = h;
}

//Retreives the nth color attachment as a RenderTarget
std::pair<GLenum, unsigned int> Framebuffer::GetColorTarget(unsigned int location) const {
	return (location < color_tbuffers.size() ? color_tbuffers[location] : std::pair<GLenum, unsigned int>{});
}

//Binds the FBO and sets the viewport to the correct size for the Framebuffer. Also sets the Drawbuffers bindings if previously set via SetDrawbuffersBindings()
void Framebuffer::Bind() const {
	static constexpr GLenum buffers[] = {gl::COLOR_ATTACHMENT0, gl::COLOR_ATTACHMENT1, gl::COLOR_ATTACHMENT2, gl::COLOR_ATTACHMENT3, gl::COLOR_ATTACHMENT4, gl::COLOR_ATTACHMENT5, gl::COLOR_ATTACHMENT6, gl::COLOR_ATTACHMENT7, gl::COLOR_ATTACHMENT8, gl::COLOR_ATTACHMENT9, gl::COLOR_ATTACHMENT10, gl::COLOR_ATTACHMENT11, gl::COLOR_ATTACHMENT12, gl::COLOR_ATTACHMENT13, gl::COLOR_ATTACHMENT14, gl::COLOR_ATTACHMENT15};
	//Bind the current FBO
	gl::BindFramebuffer(gl::FRAMEBUFFER, gl_fbo);
	if (gl_fbo != 0) {
		//Activate all related color buffers
		gl::DrawBuffers(GLsizei(color_tbuffers.size()), buffers);
	}
	//Set the current viewport to the correct size
	if(width > 0 && height > 0)
		gl::Viewport(0,0,width,height);

}

void Framebuffer::Bind(unsigned int w, unsigned int h) const {
	gl::BindFramebuffer(gl::FRAMEBUFFER, gl_fbo);
	gl::Viewport(0, 0, w, h);
}

//Unbinds the FBO by binding back the default framebuffer and returns the viewport to the stored viewport size from the previous Bind() call
void Framebuffer::Unbind() {
	//Bind FBO 0 to reset the FBO binding
	gl::BindFramebuffer(gl::FRAMEBUFFER, 0);
}

void Framebuffer::FindViewportSize() {
	width = height = 0;
	//Find out the width of the first color attachment and match that
	for(auto &ctarget : color_tbuffers) {
		if(ctarget.second){
			int miplevel = 0;
			gl::GetTexLevelParameteriv(ctarget.first, miplevel, gl::TEXTURE_WIDTH, reinterpret_cast<int*>(&width));
			gl::GetTexLevelParameteriv(ctarget.first, miplevel, gl::TEXTURE_HEIGHT, reinterpret_cast<int*>(&height));
			break;
		}
	}
}

unsigned int Framebuffer::CheckStatus([[maybe_unused]] bool debugAssert) const {
	Bind();
	auto ret = gl::CheckFramebufferStatus(gl::DRAW_FRAMEBUFFER);
	switch(ret) {
	case gl::FRAMEBUFFER_COMPLETE:
		break;
	case gl::FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		assert(false || !debugAssert); 
		break;
	case gl::FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		assert(false || !debugAssert);
		break;

	case gl::FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		assert(false || !debugAssert);
		break;

	default:
		assert(false || !debugAssert);
		break;
	}

	return ret;
}

void Framebuffer::GenerateMipmaps() const {
	for(auto &ctarget : color_tbuffers) {
		gl::BindTexture(ctarget.first, ctarget.second);
		gl::GenerateMipmap(ctarget.first);
	}	
}
