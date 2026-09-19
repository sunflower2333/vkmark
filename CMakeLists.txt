cmake_minimum_required(VERSION 3.24)
project(vkmark VERSION 2025.01 LANGUAGES CXX)
if(NOT WIN32)
    message(FATAL_ERROR "Use the upstream Meson build on non-Windows platforms")
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Vulkan REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(getopt CONFIG REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_file(VKMARK_VK_XML vk.xml
    HINTS "${Vulkan_INCLUDE_DIR}/../share/vulkan/registry"
          "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/vulkan/registry"
    REQUIRED)
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/format_map_gen.h"
    COMMAND Python3::Interpreter "${CMAKE_CURRENT_SOURCE_DIR}/src/gen_format_map.py"
        "${VKMARK_VK_XML}" "${CMAKE_CURRENT_BINARY_DIR}/format_map_gen.h"
    DEPENDS src/gen_format_map.py "${VKMARK_VK_XML}" VERBATIM)
add_executable(vkmark
    src/main.cpp src/benchmark.cpp src/benchmark_collection.cpp
    src/default_benchmarks.cpp src/device_uuid.cpp src/log.cpp src/main_loop.cpp
    src/mesh.cpp src/model.cpp src/options.cpp src/scene.cpp
    src/scene_collection.cpp src/util.cpp src/vulkan_state.cpp
    src/window_system_loader_win32.cpp src/ws/win32_native_system.cpp
    src/ws/swapchain_window_system.cpp
    src/vkutil/buffer_builder.cpp src/vkutil/copy_buffer.cpp
    src/vkutil/descriptor_set_builder.cpp src/vkutil/find_matching_memory_type.cpp
    src/vkutil/framebuffer_builder.cpp src/vkutil/image_builder.cpp
    src/vkutil/image_view_builder.cpp src/vkutil/map_memory.cpp
    src/vkutil/one_time_command_buffer.cpp src/vkutil/pipeline_builder.cpp
    src/vkutil/render_pass_builder.cpp src/vkutil/semaphore_builder.cpp
    src/vkutil/texture_builder.cpp src/vkutil/transition_image_layout.cpp
    src/scenes/clear_scene.cpp src/scenes/cube_scene.cpp
    src/scenes/default_options_scene.cpp src/scenes/desktop_scene.cpp
    src/scenes/effect2d_scene.cpp src/scenes/shading_scene.cpp
    src/scenes/texture_scene.cpp src/scenes/vertex_scene.cpp
    "${CMAKE_CURRENT_BINARY_DIR}/format_map_gen.h")
target_include_directories(vkmark PRIVATE src "${CMAKE_CURRENT_BINARY_DIR}")
target_compile_definitions(vkmark PRIVATE VK_USE_PLATFORM_WIN32_KHR NOMINMAX
    WIN32_LEAN_AND_MEAN _USE_MATH_DEFINES _CRT_SECURE_NO_WARNINGS
    VKMARK_VERSION_STR="${PROJECT_VERSION}" VKMARK_DATA_DIR="data"
    VKMARK_WINDOW_SYSTEM_DIR="builtin")
target_link_libraries(vkmark PRIVATE Vulkan::Vulkan glm::glm assimp::assimp user32
    $<IF:$<TARGET_EXISTS:getopt::getopt_shared>,getopt::getopt_shared,getopt::getopt_static>)
if(MSVC)
    target_compile_options(vkmark PRIVATE /EHsc /W4)
endif()
install(TARGETS vkmark RUNTIME DESTINATION .)
install(DIRECTORY data/shaders data/models data/textures DESTINATION data)
