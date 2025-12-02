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
add_subdirectory(weave/graphics)
add_subdirectory(weave/platform)
add_subdirectory(weave/animation)
add_subdirectory(weave/input)
add_subdirectory(weave/particles)
add_subdirectory(weave/scene)
add_subdirectory(weave/scenegraph)
set(WEAVE_TEST_SOURCES
    WeaveTests.cpp
    WeaveTests.h
    tests/blender/NodeTests.cpp
    tests/particles/ParticleBufferTests.cpp
    tests/particles/ParticleMachineTests.cpp
    tests/scenegraph/SceneGraphTests.cpp
    tests/graphics/core/ImageDataTests.cpp
    tests/graphics/core/ImageLoaderTests.cpp
    tests/graphics/core/MeshBuilderTests.cpp
    tests/graphics/core/MeshDataTests.cpp
    tests/graphics/core/MeshLayoutTests.cpp
    tests/graphics/core/MeshLoaderTests.cpp
    tests/graphics/core/MeshUtilitiesTests.cpp
    tests/graphics/core/Primitives3DTests.cpp
    tests/system/memory/DataTypeTests.cpp
    tests/system/memory/StreamingForgeTests.cpp
)
if(WEAVE_BUILD_DEMO)
    if(WIN32)
        add_executable(WeaveTest)
        target_sources(WeaveTest PRIVATE
            ${WEAVE_TEST_SOURCES}
        )
        weave_apply_default_settings(WeaveTest)
        target_link_libraries(WeaveTest PRIVATE
            WeaveSystemLib
            WeaveGraphicsLib
            WeavePlatformLib
            WeaveAnimationLib
            WeaveInputLib
            WeaveParticlesLib
            WeaveSceneLib
            WeaveSceneGraphLib
        )
    elseif(UNIX AND NOT APPLE)
        if(WEAVE_ENABLE_WAYLAND)
            add_executable(WeaveTest)
            target_sources(WeaveTest PRIVATE
                ${WEAVE_TEST_SOURCES}
            )
            weave_apply_default_settings(WeaveTest)
            target_link_libraries(WeaveTest PRIVATE
                WeaveSystemLib
                WeaveGraphicsLib
                WeavePlatformLib
                WeaveAnimationLib
                WeaveInputLib
                WeaveParticlesLib
                WeaveSceneLib
                WeaveSceneGraphLib
            )
        else()
            message(WARNING "Wayland backend disabled; skipping WeaveTest build.")
        endif()
    else()
        message(WARNING "WeaveTest demo not supported on this platform.")
    endif()
endif()
