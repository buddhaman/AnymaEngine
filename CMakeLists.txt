cmake_minimum_required(VERSION 3.19)

option(USE_ASAN "Enable AddressSanitizer" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_TOOLCHAIN_FILE "C:/local/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(AnymaEngine)

if(USE_ASAN)
    add_compile_options(/fsanitize=address)
    add_link_options(/fsanitize=address)
endif()

file(GLOB PROJECT_SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

find_package(GLEW REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC GLEW::GLEW)

find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC SDL2::SDL2 SDL2::SDL2main)

find_package(ImGui REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC imgui::imgui)

find_package(implot REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC implot::implot)

target_compile_definitions(${PROJECT_NAME} PUBLIC -DIMGUI_USER_CONFIG="ImGuiConfig.h")



