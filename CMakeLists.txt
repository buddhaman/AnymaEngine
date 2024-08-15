cmake_minimum_required(VERSION 3.19)

option(USE_ASAN "Enable AddressSanitizer" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS_DEBUG "-static-libgcc -static-libstdc++ -g")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
endif()

set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
set(VCPKG_CRT_LINKAGE static)
set(VCPKG_LIBRARY_LINKAGE static)

set(CMAKE_TOOLCHAIN_FILE "C:/local/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(AnymaEngine)

if(USE_ASAN)
    add_compile_options(/fsanitize=address)
    add_link_options(/fsanitize=address)
endif()

file(GLOB PROJECT_SOURCES "Src/*.cpp")
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

if(MSVC)
    # Set the /SUBSYSTEM:WINDOWS linker flag for Windows GUI applications in Release mode
    set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY
        LINK_FLAGS_RELEASE " /SUBSYSTEM:WINDOWS")
    
    set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY
        LINK_FLAGS_DEBUG " /SUBSYSTEM:CONSOLE")
endif()

find_package(GLEW REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC GLEW::GLEW)

find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC SDL2::SDL2 SDL2::SDL2main)

find_package(ImGui REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC imgui::imgui)

find_package(implot REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC implot::implot)

target_compile_definitions(${PROJECT_NAME} PUBLIC -DIMGUI_USER_CONFIG="ImGuiConfig.h")

add_subdirectory(Test)

# Assets
# Define the assets directory
set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")

# Define the destination for assets in the build directory
set(ASSETS_DESTINATION "${CMAKE_BINARY_DIR}/assets")

# Ensure ASSETS_DIR is defined
if(NOT DEFINED ASSETS_DIR)
    message(FATAL_ERROR "ASSETS_DIR is not defined")
endif()

# Define the output directory
set(OUTPUT_DIR $<TARGET_FILE_DIR:AnymaEngine>/assets)

# Debugging message
message("ASSETS_DIR: ${ASSETS_DIR}")
message("OUTPUT_DIR: ${OUTPUT_DIR}")

# Define a custom target to copy assets
add_custom_target(
    copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR} # Ensure output directory exists
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSETS_DIR} ${OUTPUT_DIR}
    COMMENT "Copying assets to build directory"
)

# Gather all files in the assets directory to track dependencies
file(GLOB_RECURSE ASSET_FILES "${ASSETS_DIR}/*")

# Ensure copy_assets target is rebuilt when any asset file changes
add_custom_target(
    copy_assets_trigger
    DEPENDS ${ASSET_FILES}
    COMMENT "Trigger to check asset file changes"
)

# Ensure AnymaEngine depends on the copy_assets target
add_dependencies(copy_assets copy_assets_trigger)
add_dependencies(AnymaEngine copy_assets)
