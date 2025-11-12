cmake_minimum_required(VERSION 3.21)

project(Weave LANGUAGES C CXX)

option(WEAVE_BUILD_DEMO "Build the Weave sample harness" ON)
option(WEAVE_WARNINGS_AS_ERRORS "Treat warnings as errors" ON)
option(WEAVE_ENABLE_NX "Enable Nintendo Switch specific sources" OFF)
option(WEAVE_ENABLE_WAYLAND "Enable Wayland platform backend on Linux" ON)

set(WEAVE_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

if(MINGW AND CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_CROSSCOMPILING_EMULATOR "wine" CACHE FILEPATH "Emulator used to run Windows targets on Linux" FORCE)
endif()

function(weave_apply_default_settings target_name)
    target_compile_features(${target_name} PUBLIC cxx_std_20)
    target_include_directories(${target_name} PUBLIC "${WEAVE_ROOT_DIR}")

    if(MSVC)
        target_compile_options(${target_name} PRIVATE /permissive- /Zc:__cplusplus /MP /W4)
        if(WEAVE_WARNINGS_AS_ERRORS)
            target_compile_options(${target_name} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -fms-extensions)
        if(WEAVE_WARNINGS_AS_ERRORS)
            target_compile_options(${target_name} PRIVATE -Werror)
        endif()
    endif()

    if(WIN32)
        target_compile_definitions(${target_name} PRIVATE UNICODE _UNICODE)
    endif()
endfunction()

add_subdirectory(weave/system)
add_subdirectory(weave/platform)
add_subdirectory(weave/animation)
add_subdirectory(weave/input)
add_subdirectory(weave/particles)




if(WEAVE_BUILD_DEMO)
    if(WIN32)
        add_executable(WeaveTest)
        target_sources(WeaveTest PRIVATE
            WeaveTests.cpp
            WeaveTests.h
        )
        weave_apply_default_settings(WeaveTest)
        target_link_libraries(WeaveTest PRIVATE
            WeaveSystemLib
            WeavePlatformLib
            WeaveAnimationLib
            WeaveInputLib
            WeaveParticlesLib
        )
    elseif(UNIX AND NOT APPLE)
        if(WEAVE_ENABLE_WAYLAND)
            add_executable(WeaveTest)
            target_sources(WeaveTest PRIVATE
                WeaveTests.cpp
                WeaveTests.h
            )
            weave_apply_default_settings(WeaveTest)
            target_link_libraries(WeaveTest PRIVATE
                WeaveSystemLib
                WeavePlatformLib
                WeaveAnimationLib
                WeaveInputLib
                WeaveParticlesLib
            )
        else()
            message(WARNING "Wayland backend disabled; skipping WeaveTest build.")
        endif()
    else()
        message(WARNING "WeaveTest demo not supported on this platform.")
    endif()
endif()
