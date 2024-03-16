
project(MathTest)

file(GLOB TEST_SOURCES "Src/*.cpp")
add_executable(${PROJECT_NAME} ${TEST_SOURCES})

find_package(Catch2 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Catch2::Catch2)
