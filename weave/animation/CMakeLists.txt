set(target_name WeaveAnimationLib)

add_library(${target_name} STATIC)

file(GLOB_RECURSE weave_animation_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

target_sources(${target_name} PRIVATE ${weave_animation_sources})

weave_apply_default_settings(${target_name})
