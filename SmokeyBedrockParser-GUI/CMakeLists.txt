cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0074 NEW)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "In-source builds not allowed. Please run CMake from the './build' directory. You may need to remove CMakeCache.txt.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BIN_NAME SmokeyBedrockParser)
set(LIB_NAME SmokeyBedrockParserLib)

project(SmokeyBedrockParser VERSION 0.1)

find_package(OpenGL REQUIRED)
find_package(unofficial-nativefiledialog CONFIG REQUIRED)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.c include/*cpp)
file(GLOB_RECURSE HEADER_FILES include/*.h include/*.hpp)
add_library(${LIB_NAME} STATIC ${SRC_FILES})
file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(../SmokeyBedrockParser-Core SmokeyBedrockParser-Core)
add_subdirectory(third-party/glfw)
add_subdirectory(third-party/glm)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

target_include_directories(${LIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(${LIB_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${LIB_NAME}
  glfw glm OpenGL::GL unofficial::nativefiledialog::nfd SmokeyBedrockParser-Core
)

add_executable(${BIN_NAME} src/SmokeyBedrockParser.cpp)
target_link_libraries(${BIN_NAME} PRIVATE ${LIB_NAME})
target_include_directories(${BIN_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})