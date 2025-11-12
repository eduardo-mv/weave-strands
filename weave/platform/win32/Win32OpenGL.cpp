#ifdef _WIN32

#include "Win32OpenGL.h"
#include <sstream>
#include <thread>
#include <future>
#include <codecvt>

//OpenGL
#ifdef _MSC_VER
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#endif

weave::win32::Win32OpenGL::~Win32OpenGL() {
	DestroyContexts();
}

bool weave::win32::Win32OpenGL::InitGL(HWND hwnd_) {
	return InitGL(hwnd_, InitParams{});
}

bool weave::win32::Win32OpenGL::InitGL(HWND hwnd_, InitParams initParams) {

	//Create a fake window to initialize GL 1.0
	WNDCLASSW wc = { };
	wc.lpfnWndProc = DefWindowProcW;
	wc.hInstance = nullptr;
	wc.lpszClassName = L"GL window class";
	RegisterClassW(&wc);

	HWND fakeHwnd = CreateWindowExW(
		0,
		wc.lpszClassName,
		L"",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		nullptr, nullptr, nullptr, this);

	UnregisterClassW(wc.lpszClassName, nullptr);

	if (!fakeHwnd) {
		return false;
	}

	//Create the OpenGL 1.0 context
	PIXELFORMATDESCRIPTOR fakePfd = {};
	HDC fakeHdc = GetDC(fakeHwnd);
	SetPixelFormat(fakeHdc, 1, &fakePfd);
	HGLRC fakeGlrc = wglCreateContext(fakeHdc);
	wglMakeCurrent(fakeHdc, fakeGlrc);

	//Initialize GL extensions via glLoader. Some are needed to create a 4.5+ context.
	if (!gladLoadGL())
		return false;
	if (!gladLoadWGL(fakeHdc))
		return false;

	//Prepare the PFD target
	int pfdSetup[32] = {
		wgl::SUPPORT_OPENGL_ARB, 1, // Must support OGL rendering
		wgl::DRAW_TO_WINDOW_ARB, 1, // pf that can run a window
		wgl::ACCELERATION_ARB, wgl::FULL_ACCELERATION_ARB, // must be HW accelerated
		wgl::COLOR_BITS_ARB, int(initParams.colorBits), // Color buffer
		wgl::ALPHA_BITS_ARB, (initParams.colorBits > 24 ? 8 : 0), //Make sure it has an alpha channel if asked
		wgl::DEPTH_BITS_ARB, int(initParams.depthBits), // Depth buffer
		wgl::STENCIL_BITS_ARB, int(initParams.stencilBits), //Stencil buffer
		wgl::DOUBLE_BUFFER_ARB, int(initParams.numColorBuffers) - 1, // Double buffered context
		wgl::SAMPLE_BUFFERS_ARB, (initParams.msaaSamples > 0) ? (1) : (0), // MSAA on
		wgl::SAMPLES_ARB, int(initParams.msaaSamples), // MSAA factor
		wgl::PIXEL_TYPE_ARB, wgl::TYPE_RGBA_ARB, // pf should be RGBA type
		(initParams.sRGB ? wgl::FRAMEBUFFER_SRGB_CAPABLE_ARB : 0), int(initParams.sRGB), //Ask for a buffer capable of sRGB conversion
		0  // nullptr termination
	};

	//Select a suitable pixel format with the extended wgl function which uses the int pfd[32]
	int pixelformat[10] = {};
	unsigned int count = 0;
	wgl::ChoosePixelFormatARB(fakeHdc, pfdSetup, nullptr, 10, pixelformat, &count);

	//Scan the available GL version
	int major = initParams.majorVersion, minor = initParams.minorVersion;

	if (major < 1) {
		gl::GetIntegerv(gl::MAJOR_VERSION, &major);
		gl::GetIntegerv(gl::MINOR_VERSION, &minor);
	}

	//We can destroy the fake window and hdc now
	wglMakeCurrent(nullptr, nullptr);
	ReleaseDC(fakeHwnd, fakeHdc);
	DestroyWindow(fakeHwnd);
	wglDeleteContext(fakeGlrc);
	
	//If wglChoosePixelFormatARB failed, return
	if (count <= 0 || pixelformat[0] == -1)
		return false;

	//Initialize the window's surface and GL context
	hwnd = hwnd_;
	hdc = GetDC(hwnd); 
	PIXELFORMATDESCRIPTOR windowPfd = {};
	SetPixelFormat(hdc, pixelformat[0], &windowPfd);

	int glattribs[] = {
		wgl::CONTEXT_MAJOR_VERSION_ARB, major, //Major and minor version numbers of the desired GL context
		wgl::CONTEXT_MINOR_VERSION_ARB, minor,
		wgl::CONTEXT_FLAGS_ARB, wgl::CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | (initParams.debugContext ? wgl::CONTEXT_DEBUG_BIT_ARB : 0), //Set context to only support non deprecated functions
		wgl::CONTEXT_PROFILE_MASK_ARB, wgl::CONTEXT_CORE_PROFILE_BIT_ARB, //Set context to the core profile (3.0+ functions, removing old GL functions)
		0 //End structure
	};

	//Main context and shared contexts
	glrc.clear();
	glrc.push_back(wgl::CreateContextAttribsARB(hdc, 0, glattribs));
	
	weave::opengl::ContextPool::RegisterContext(hdc, glrc[0]);
	weave::opengl::ContextPool::ClaimContext();

	if (gl::GetError() != gl::NO_ERROR_ || glrc.back() == nullptr) {
		MessageBoxW(nullptr, L"Error creating OpenGL context", L"OpenGL Error", MB_ICONERROR | MB_SYSTEMMODAL | MB_OK);
		return false;
	}

	for (uint32_t i = 1; i < initParams.numGLContexts; ++i) {
		glrc.push_back(wgl::CreateContextAttribsARB(hdc, glrc[0], glattribs));
		weave::opengl::ContextPool::RegisterContext(hdc, glrc.back());

		if (gl::GetError() != gl::NO_ERROR_ || glrc.back() == nullptr)
			MessageBoxW(nullptr, L"Error creating OpenGL shared contexts", L"OpenGL Error", MB_ICONERROR | MB_SYSTEMMODAL | MB_OK);
	}

	UpdateWindow(hwnd);
	
	RECT rect = {};
	GetClientRect(hwnd, &rect);
	gl::Viewport(0, 0, rect.right, rect.bottom);

	return true;
}

void weave::win32::Win32OpenGL::DestroyContexts(){

	//Release ownership of the GL context
	wglMakeCurrent(nullptr,nullptr);

	for(auto rc : glrc) {
		wglDeleteContext(rc);
	}

	if (hdc) {
		ReleaseDC(hwnd, hdc);
	}
	
	hwnd = nullptr;
	hdc = nullptr;
	glrc.clear();
}

std::vector<std::string> weave::win32::Win32OpenGL::EnumPixelFormats() const {
	GLint queryc[]= { wgl::NUMBER_PIXEL_FORMATS_ARB };
	
	int pfcount = 0;
	wgl::GetPixelFormatAttribivARB(hdc, 1, 0, 1, queryc, &pfcount);

	GLint capabilities[] = { 
		wgl::DRAW_TO_WINDOW_ARB,
		wgl::ACCELERATION_ARB,
		wgl::SUPPORT_OPENGL_ARB,
		wgl::DOUBLE_BUFFER_ARB,
		wgl::COLOR_BITS_ARB,
		wgl::DEPTH_BITS_ARB,
		wgl::STENCIL_BITS_ARB,
		wgl::RED_BITS_ARB,
		wgl::GREEN_BITS_ARB,
		wgl::BLUE_BITS_ARB,
		wgl::ALPHA_BITS_ARB,
		wgl::SAMPLE_BUFFERS_ARB,
		wgl::SAMPLES_ARB,
		wgl::FRAMEBUFFER_SRGB_CAPABLE_ARB,
		wgl::PIXEL_TYPE_ARB
	};
		
	std::vector<std::string> out;
	std::stringstream str;

	for (int i=0; i<pfcount; i++)
	{
		int results[15];
		
		str << "Pixel format " << i <<" details:\n---------------------\n";
		
		wgl::GetPixelFormatAttribivARB(hdc, i, 0, 15, capabilities, results);
		str << "Draw to Window: " << results[0] << "\n";
		str << "HW Accelerated: " << results[1] << "\n";
		str << "Supports OpenGL: " << results[2] << "\n";
		str << "Double Buffered: " << results[3] << "\n";
		str << "Color Bits: " << results[4] << "\n";
		str << "Depth Bits: " << results[5] << "\n";
		str << "Stencil Bits: " << results[6] << "\n";
		str << "Red Bits: " << results[7] << "\n";
		str << "Green Bits: " << results[8] << "\n";
		str << "Blue Bits: " << results[9] << "\n";
		str << "Alpha Bits: " << results[10] << "\n";
		str << "Multi Sample: " << results[11] << "\n";
		str << "Multi Sample Value: " << results[12] << "\n";
		str << "sRGB Capable: " << results[13] << "\n";
		str << "Pixel Type: " << results[14] << "\n\n";

		out.emplace_back(str.str());

		str.str("");
		str.clear();
	}

	return out;
}

std::pair<int, int> weave::win32::Win32OpenGL::GetGLVersion() const {
	std::pair<int, int> version;
	gl::GetIntegerv(gl::MAJOR_VERSION, &version.first);
	gl::GetIntegerv(gl::MINOR_VERSION, &version.second);

	return version;
}

#endif
