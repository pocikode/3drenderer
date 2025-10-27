cmake_minimum_required(VERSION 3.30)
project(3drenderer C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL3 CONFIG REQUIRED)
include_directories(${SDL3_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/external/upng)

file(GLOB SRC_FILES "src/*.c")
file(GLOB UPNG_SRC_FILES "external/upng/upng.c")

add_executable(${PROJECT_NAME} ${SRC_FILES} ${UPNG_SRC_FILES})
target_link_libraries(${PROJECT_NAME} ${SDL3_LIBRARIES} m)
