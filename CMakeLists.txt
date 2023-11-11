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

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules")
set(CMAKE_BUILD_TYPE_INIT "RelWithDebInfo")
set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS ON)

set(BIN_NAME SmokeyBedrockParser)
set(LIB_NAME SmokeyBedrockParserLib)

project(SmokeyBedrockParser VERSION 0.1)

find_package(OpenGL REQUIRED)
find_package(unofficial-nativefiledialog CONFIG REQUIRED)

option(LEVELDB_BUILD_TESTS OFF)
set(NBT_BUILD_TESTS OFF CACHE INTERNAL "Don't build nbt++ tests")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

file(GLOB_RECURSE SRC_FILES src/*.cpp include/*cpp)
file(GLOB_RECURSE HEADER_FILES include/*.h include/*.hpp)
add_library(${LIB_NAME} STATIC ${SRC_FILES})

add_subdirectory(third-party/leveldb-mcpe)
add_subdirectory(third-party/spdlog)
add_subdirectory(third-party/libnbtplusplus)
add_subdirectory(third-party/glfw)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# GOOGLE disables RTTI for leveldb so we will do the same
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # Disable RTTI.
  string(REGEX REPLACE "/GR" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
else()
  # Disable RTTI.
  string(REGEX REPLACE "-frtti" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

target_include_directories(${LIB_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${LIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(${LIB_NAME}
  leveldb spdlog nbt++ glfw OpenGL::GL unofficial::nativefiledialog::nfd
)

add_executable(${BIN_NAME} src/SmokeyBedrockParser.cpp)
target_link_libraries(${BIN_NAME} PRIVATE ${LIB_NAME})
target_include_directories(${BIN_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})


if(VCPKG_APPLOCAL_DEPS AND VCPKG_TARGET_TRIPLET MATCHES "windows|uwp")
  install(DIRECTORY $<TARGET_FILE_DIR:SmokeyBedrockParser>/
    TYPE BIN FILES_MATCHING PATTERN "*.dll")
endif()