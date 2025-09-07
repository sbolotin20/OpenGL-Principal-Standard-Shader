cmake_minimum_required(VERSION 3.21)
project(Principal_Shader_Open_GL LANGUAGES C CXX)

if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Match MSVC runtime with most prebuilt libs (/MD in Release, /MDd in Debug)
if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Paths based on your screenshot
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Principal_Shader_Open_GL)
set(EXT_DIR ${SRC_DIR}/External)
set(EXT_INCLUDE ${EXT_DIR}/include)

add_executable(${PROJECT_NAME}
  ${SRC_DIR}/main.cpp
  ${EXT_DIR}/glad.c           # compile glad!
  ${SRC_DIR}/main.cpp
  ${EXT_DIR}/glad.c
  ${SRC_DIR}/shader_utils.cpp
  ${SRC_DIR}/mesh_utils.cpp
  ${SRC_DIR}/texture_utils.cpp
  ${SRC_DIR}/uniforms.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${EXT_INCLUDE}              # glad/, GLFW/, KHR/
  ${SRC_DIR}                  # for "External/stb_image.h"
)

# Link GLFW (your prebuilt lib) + system OpenGL
target_link_libraries(${PROJECT_NAME} PRIVATE
  ${EXT_DIR}/lib/glfw3.lib
  opengl32
)

# ---- Copy runtime assets next to the exe for debugging ----
# shaders/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SRC_DIR}/shaders
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SRC_DIR}/textures
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/textures
)

# Base color texture
if(EXISTS ${SRC_DIR}/textures/base_color.jpg)
  set(BASE_COLOR_TEX ${SRC_DIR}/textures/base_color.jpg)
elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/textures/base_color.jpg)
  set(BASE_COLOR_TEX ${CMAKE_CURRENT_SOURCE_DIR}/textures/base_color.jpg)
endif()

# Normal map texture
if(EXISTS ${SRC_DIR}/textures/normal_map.png)
  set(NORMAL_MAP_TEX ${SRC_DIR}/textures/normal_map.png)
elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/textures/normal_map.png)
  set(NORMAL_MAP_TEX ${CMAKE_CURRENT_SOURCE_DIR}/textures/normal_map.png)
endif()

# roughness map texture
if(EXISTS ${SRC_DIR}/textures/roughness_map.png)
  set(ROUGHNESS_MAP_TEX ${SRC_DIR}/textures/roughness_map.png)
elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/textures/roughness_map.png)
  set(ROUGHNESS_MAP_TEX ${CMAKE_CURRENT_SOURCE_DIR}/textures/roughness_map.png)
endif()


if(TEX)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${TEX}
      $<TARGET_FILE_DIR:${PROJECT_NAME}>
  )
endif()

