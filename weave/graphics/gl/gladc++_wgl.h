
#pragma once

/*
Auto generated C++ namespaced wrappers for glad generated WGL headers
*/
#include "glad_wgl.h"

namespace wgl {
enum {
    CONTEXT_DEBUG_BIT_ARB = 0x00000001,
    CONTEXT_FORWARD_COMPATIBLE_BIT_ARB = 0x00000002,
    CONTEXT_MAJOR_VERSION_ARB = 0x2091,
    CONTEXT_MINOR_VERSION_ARB = 0x2092,
    CONTEXT_LAYER_PLANE_ARB = 0x2093,
    CONTEXT_FLAGS_ARB = 0x2094,
    CONTEXT_PROFILE_MASK_ARB = 0x9126,
    CONTEXT_CORE_PROFILE_BIT_ARB = 0x00000001,
    CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB = 0x00000002,
    CONTEXT_ROBUST_ACCESS_BIT_ARB = 0x00000004,
    LOSE_CONTEXT_ON_RESET_ARB = 0x8252,
    CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB = 0x8256,
    NO_RESET_NOTIFICATION_ARB = 0x8261,
    FRAMEBUFFER_SRGB_CAPABLE_ARB = 0x20A9,
    SAMPLE_BUFFERS_ARB = 0x2041,
    SAMPLES_ARB = 0x2042,
    NUMBER_PIXEL_FORMATS_ARB = 0x2000,
    DRAW_TO_WINDOW_ARB = 0x2001,
    DRAW_TO_BITMAP_ARB = 0x2002,
    ACCELERATION_ARB = 0x2003,
    NEED_PALETTE_ARB = 0x2004,
    NEED_SYSTEM_PALETTE_ARB = 0x2005,
    SWAP_LAYER_BUFFERS_ARB = 0x2006,
    SWAP_METHOD_ARB = 0x2007,
    NUMBER_OVERLAYS_ARB = 0x2008,
    NUMBER_UNDERLAYS_ARB = 0x2009,
    TRANSPARENT_ARB = 0x200A,
    TRANSPARENT_RED_VALUE_ARB = 0x2037,
    TRANSPARENT_GREEN_VALUE_ARB = 0x2038,
    TRANSPARENT_BLUE_VALUE_ARB = 0x2039,
    TRANSPARENT_ALPHA_VALUE_ARB = 0x203A,
    TRANSPARENT_INDEX_VALUE_ARB = 0x203B,
    SHARE_DEPTH_ARB = 0x200C,
    SHARE_STENCIL_ARB = 0x200D,
    SHARE_ACCUM_ARB = 0x200E,
    SUPPORT_GDI_ARB = 0x200F,
    SUPPORT_OPENGL_ARB = 0x2010,
    DOUBLE_BUFFER_ARB = 0x2011,
    STEREO_ARB = 0x2012,
    PIXEL_TYPE_ARB = 0x2013,
    COLOR_BITS_ARB = 0x2014,
    RED_BITS_ARB = 0x2015,
    RED_SHIFT_ARB = 0x2016,
    GREEN_BITS_ARB = 0x2017,
    GREEN_SHIFT_ARB = 0x2018,
    BLUE_BITS_ARB = 0x2019,
    BLUE_SHIFT_ARB = 0x201A,
    ALPHA_BITS_ARB = 0x201B,
    ALPHA_SHIFT_ARB = 0x201C,
    ACCUM_BITS_ARB = 0x201D,
    ACCUM_RED_BITS_ARB = 0x201E,
    ACCUM_GREEN_BITS_ARB = 0x201F,
    ACCUM_BLUE_BITS_ARB = 0x2020,
    ACCUM_ALPHA_BITS_ARB = 0x2021,
    DEPTH_BITS_ARB = 0x2022,
    STENCIL_BITS_ARB = 0x2023,
    AUX_BUFFERS_ARB = 0x2024,
    NO_ACCELERATION_ARB = 0x2025,
    GENERIC_ACCELERATION_ARB = 0x2026,
    FULL_ACCELERATION_ARB = 0x2027,
    SWAP_EXCHANGE_ARB = 0x2028,
    SWAP_COPY_ARB = 0x2029,
    SWAP_UNDEFINED_ARB = 0x202A,
    TYPE_RGBA_ARB = 0x202B,
    TYPE_COLORINDEX_ARB = 0x202C,
    TYPE_RGBA_FLOAT_ARB = 0x21A0,
    CONTEXT_ES2_PROFILE_BIT_EXT = 0x00000004,
    TYPE_RGBA_UNSIGNED_FLOAT_EXT = 0x20A8,
    ARB_create_context = 1,
    ARB_create_context_profile = 1,
    ARB_create_context_robustness = 1,
    ARB_extensions_string = 1,
    ARB_framebuffer_sRGB = 1,
    ARB_multisample = 1,
    ARB_pixel_format = 1,
    ARB_pixel_format_float = 1,
    NV_swap_group = 1

};

inline HGLRC CreateContextAttribsARB(HDC hDC, HGLRC hShareContext, const int *attribList){ return glad_wglCreateContextAttribsARB(hDC, hShareContext, attribList); }
inline const char * GetExtensionsStringARB(HDC hdc){ return glad_wglGetExtensionsStringARB(hdc); }
inline BOOL GetPixelFormatAttribivARB(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, int *piValues){ return glad_wglGetPixelFormatAttribivARB(hdc, iPixelFormat, iLayerPlane, nAttributes, piAttributes, piValues); }
inline BOOL GetPixelFormatAttribfvARB(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, FLOAT *pfValues){ return glad_wglGetPixelFormatAttribfvARB(hdc, iPixelFormat, iLayerPlane, nAttributes, piAttributes, pfValues); }
inline BOOL ChoosePixelFormatARB(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats){ return glad_wglChoosePixelFormatARB(hdc, piAttribIList, pfAttribFList, nMaxFormats, piFormats, nNumFormats); }
inline const char * GetExtensionsStringEXT(void){ return glad_wglGetExtensionsStringEXT(); }
inline BOOL SwapIntervalEXT(int interval){ return glad_wglSwapIntervalEXT(interval); }
inline int GetSwapIntervalEXT(void){ return glad_wglGetSwapIntervalEXT(); }
inline BOOL JoinSwapGroupNV(HDC hDC, GLuint group){ return glad_wglJoinSwapGroupNV(hDC, group); }
inline BOOL BindSwapBarrierNV(GLuint group, GLuint barrier){ return glad_wglBindSwapBarrierNV(group, barrier); }
inline BOOL QuerySwapGroupNV(HDC hDC, GLuint *group, GLuint *barrier){ return glad_wglQuerySwapGroupNV(hDC, group, barrier); }
inline BOOL QueryMaxSwapGroupsNV(HDC hDC, GLuint *maxGroups, GLuint *maxBarriers){ return glad_wglQueryMaxSwapGroupsNV(hDC, maxGroups, maxBarriers); }
inline BOOL QueryFrameCountNV(HDC hDC, GLuint *count){ return glad_wglQueryFrameCountNV(hDC, count); }
inline BOOL ResetFrameCountNV(HDC hDC){ return glad_wglResetFrameCountNV(hDC); }

}

