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

# texture.png (handle either location)
if(EXISTS ${SRC_DIR}/texture.png)
  set(TEX ${SRC_DIR}/texture.png)
elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/texture.png)
  set(TEX ${CMAKE_CURRENT_SOURCE_DIR}/texture.png)
endif()

if(TEX)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${TEX}
      $<TARGET_FILE_DIR:${PROJECT_NAME}>
  )
endif()
