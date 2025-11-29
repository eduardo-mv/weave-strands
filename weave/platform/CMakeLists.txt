set(target_name WeavePlatformLib)

add_library(${target_name} STATIC)

set(weave_platform_sources)

if(WIN32)
    file(GLOB weave_platform_win32_sources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/win32/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/win32/*.h"
    )
    list(APPEND weave_platform_sources ${weave_platform_win32_sources})

    target_link_libraries(${target_name} PUBLIC
        user32
        gdi32
        shell32
        shlwapi
        ole32
        oleaut32
        opengl32
        glu32
        dbghelp
    )
endif()

if(NOT WIN32 AND WEAVE_ENABLE_WAYLAND)
    file(GLOB weave_platform_wayland_sources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/wayland/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/wayland/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/wayland/*.h"
    )
    list(APPEND weave_platform_wayland_sources
        "${CMAKE_CURRENT_SOURCE_DIR}/wayland/xdg-shell-protocol.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/wayland/xdg-decoration-unstable-v1-protocol.c"
    )
    list(APPEND weave_platform_sources ${weave_platform_wayland_sources})

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WAYLAND_CLIENT REQUIRED IMPORTED_TARGET wayland-client)
    pkg_check_modules(WAYLAND_EGL REQUIRED IMPORTED_TARGET wayland-egl)
    pkg_check_modules(EGL REQUIRED IMPORTED_TARGET egl)

    target_link_libraries(${target_name} PUBLIC PkgConfig::WAYLAND_CLIENT PkgConfig::WAYLAND_EGL PkgConfig::EGL)
    target_compile_definitions(${target_name} PRIVATE WEAVE_PLATFORM_WAYLAND)
endif()

target_sources(${target_name} PRIVATE ${weave_platform_sources})

weave_apply_default_settings(${target_name})

target_link_libraries(${target_name} PUBLIC WeaveGraphicsLib)
