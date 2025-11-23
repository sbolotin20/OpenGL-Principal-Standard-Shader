cmake_minimum_required(VERSION 3.21)
project(Principal_Shader_Open_GL LANGUAGES C CXX)

if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Principal_Shader_Open_GL)
set(EXT_DIR ${SRC_DIR}/External)
set(IMGUI_DIR ${EXT_DIR}/imgui)

set(IMGUI_SRC
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp
  ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
  ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
  ${IMGUI_DIR}/ImGuiFileDialog.cpp
)
if(EXISTS "${IMGUI_DIR}/imgui_demo.cpp")
  list(APPEND IMGUI_SRC ${IMGUI_DIR}/imgui_demo.cpp)
endif()


add_executable(${PROJECT_NAME}
  ${SRC_DIR}/main.cpp
  ${SRC_DIR}/shader_utils.cpp
  ${SRC_DIR}/mesh_utils.cpp
  ${SRC_DIR}/texture_utils.cpp
  ${SRC_DIR}/uniforms.cpp
  ${EXT_DIR}/glad.c
  ${EXT_DIR}/tinyobjloader/tiny_obj_loader.cc 
  ${IMGUI_SRC}
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${SRC_DIR}
  ${EXT_DIR}
  ${EXT_DIR}/include
  ${IMGUI_DIR}
  ${IMGUI_DIR}/backends
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${SRC_DIR}
  ${EXT_DIR}
  ${EXT_DIR}/include
  ${EXT_DIR}/tinyobjloader      # <--- Add this line
  ${IMGUI_DIR}
  ${IMGUI_DIR}/backends
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  IMGUI_IMPL_OPENGL_LOADER_GLAD
  GLFW_INCLUDE_NONE
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  ${EXT_DIR}/lib/glfw3.lib     # or glfw3dll.lib
  opengl32 user32 gdi32 shell32
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SRC_DIR}/shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SRC_DIR}/textures $<TARGET_FILE_DIR:${PROJECT_NAME}>/textures)


target_compile_definitions(${PROJECT_NAME} PRIVATE
  IMGUI_IMPL_OPENGL_LOADER_GLAD
  GLFW_INCLUDE_NONE
  USE_STD_FILESYSTEM=1        # <- switch ImGuiFileDialog to std::filesystem
  NOMINMAX
  _CRT_SECURE_NO_WARNINGS
)
