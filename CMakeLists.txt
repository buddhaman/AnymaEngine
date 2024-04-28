cmake_minimum_required(VERSION 3.19)

option(USE_ASAN "Enable AddressSanitizer" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
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

# Create a custom target to copy assets when they change
add_custom_target(
    copy_assets ALL  # Re-run this target on every build to check dependencies
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSETS_DIR} $<TARGET_FILE_DIR:AnymaEngine>/assets
    COMMENT "Copying assets to build directory"
    DEPENDS ${ASSETS_DIR}/*  # This ensures the target is triggered when any file in the assets folder changes
)

add_dependencies(AnymaEngine copy_assets)





