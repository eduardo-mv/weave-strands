set(target_name WeaveGraphicsLib)

add_library(${target_name} STATIC)

file(GLOB weave_graphics_core_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/*.h"
)

file(GLOB weave_graphics_gl_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/gl/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/gl/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/gl/resources/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/gl/resources/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/gl/context/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/gl/context/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/gl/gpu/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/gl/gpu/*.h"
)

file(GLOB weave_graphics_vulkan_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/vulkan/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/vulkan/*.h"
)

set(weave_graphics_sources
    ${weave_graphics_core_sources}
    ${weave_graphics_gl_sources}
    ${weave_graphics_vulkan_sources}
)

if(NOT WIN32)
    list(FILTER weave_graphics_sources EXCLUDE REGEX "glad_wgl\\.cpp$")
endif()

target_sources(${target_name} PRIVATE ${weave_graphics_sources})

if(NOT WIN32 AND WEAVE_ENABLE_WAYLAND)
    target_compile_definitions(${target_name} PRIVATE WEAVE_PLATFORM_WAYLAND)
endif()

weave_apply_default_settings(${target_name})
