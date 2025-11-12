set(target_name WeaveInputLib)

add_library(${target_name} STATIC)

file(GLOB_RECURSE weave_input_core_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/system/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/system/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/processors/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/processors/*.h"
)

set(weave_input_sources ${weave_input_core_sources})

if(WIN32)
    file(GLOB_RECURSE weave_input_win32_sources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/win32/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/win32/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/rawinput/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/rawinput/*.h"
    )
    list(APPEND weave_input_sources ${weave_input_win32_sources})

    target_link_libraries(${target_name} PUBLIC
        user32
        hid
    )
endif()

if(NOT WIN32 AND WEAVE_ENABLE_WAYLAND)
    file(GLOB_RECURSE weave_input_wayland_sources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/wayland/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/wayland/*.h"
    )
    list(APPEND weave_input_sources ${weave_input_wayland_sources})

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WAYLAND_CLIENT REQUIRED IMPORTED_TARGET wayland-client)
    pkg_check_modules(XKBCOMMON REQUIRED IMPORTED_TARGET xkbcommon)
    pkg_check_modules(LIBUDEV REQUIRED IMPORTED_TARGET libudev)
    pkg_check_modules(LIBEVDEV REQUIRED IMPORTED_TARGET libevdev)
    pkg_check_modules(ALSA REQUIRED IMPORTED_TARGET alsa)

    target_link_libraries(${target_name} PUBLIC
        PkgConfig::WAYLAND_CLIENT
        PkgConfig::XKBCOMMON
        PkgConfig::LIBUDEV
        PkgConfig::LIBEVDEV
        PkgConfig::ALSA
    )
endif()

if(WEAVE_ENABLE_NX)
    file(GLOB_RECURSE weave_input_nx_sources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/nx/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/nx/*.h"
    )
    list(APPEND weave_input_sources ${weave_input_nx_sources})
endif()

target_sources(${target_name} PRIVATE ${weave_input_sources})
target_link_libraries(${target_name} PUBLIC WeaveSystemLib)

weave_apply_default_settings(${target_name})
