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

project(SmokeyBedrockParser VERSION 0.1 LANGUAGES CXX)

option(LEVELDB_BUILD_TESTS OFF)
add_subdirectory(leveldb-mcpe)

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

if(VCPKG_APPLOCAL_DEPS AND VCPKG_TARGET_TRIPLET MATCHES "windows|uwp")
  install(DIRECTORY $<TARGET_FILE_DIR:SmokeyBedrockParser>
    TYPE BIN
    FILES_MATCHING PATTERN "*.dll")
endif()

add_executable(SmokeyBedrockParser
  nbt.cpp
  Source.cpp
)
target_link_libraries(SmokeyBedrockParser leveldb)
target_include_directories(SmokeyBedrockParser PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")

if(VCPKG_APPLOCAL_DEPS AND VCPKG_TARGET_TRIPLET MATCHES "windows|uwp")
  install(DIRECTORY $<TARGET_FILE_DIR:SmokeyBedrockParser>/
    TYPE BIN FILES_MATCHING PATTERN "*.dll")
endif()