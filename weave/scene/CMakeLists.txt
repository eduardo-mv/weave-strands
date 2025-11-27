set(target_name WeaveSceneLib)

add_library(${target_name} STATIC)

file(GLOB_RECURSE weave_scene_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

target_sources(${target_name} PRIVATE ${weave_scene_sources})

weave_apply_default_settings(${target_name})
