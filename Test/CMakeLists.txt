
project(MathTest)

file(GLOB TEST_SOURCES "Src/*.cpp")

set(CORE_TEST_FILES "../src/linalg.h")

add_executable(${PROJECT_NAME} ${TEST_SOURCES} ${CORE_TEST_FILES})

find_package(Catch2 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Catch2::Catch2)
