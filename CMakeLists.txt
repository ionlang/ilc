# General project settings.
cmake_minimum_required(VERSION 3.12.4)
project(onyx)

# Setup compiler flag(s) & standard(s).
set(CMAKE_CXX_STANDARD 17)

# General project information.
set(PROJECT_URL "https://github.com/ionlang/onyx")

# Setup source and build directories.
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Setup default build flags.
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} /MT")

# Option to use MD.
option(USE_MD "Use MD instead of MTd" OFF)

if (USE_MD)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MD")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    message(STATUS "Using /MD flag")
endif(USE_MD)

# Options.
option(USE_CLANG "Use Clang and Clang++ as compilers")

if (USE_CLANG)
    # TODO
    set(T "")
endif(USE_CLANG)

# Setup versioning.
set(VERSION_MAJOR "1")
set(VERSION_MINOR "0")
set(VERSION_PATCH "0")
set(VERSION "$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)")

# Set source file(s).
file(GLOB_RECURSE SOURCES
    "src/*.h"
    "src/*.cpp"
    "libs/ionir/src/*.cpp"
)

# Specify that this project is a library.
add_executable(${PROJECT_NAME} ${SOURCES})

# Provide include directories to be used in the build command. Position in file matters.
include_directories("src")
include_directories(libs)
include_directories(libs/ionir/src)
include_directories(libs/ionir/libs)

# Setup unit testing using Google Test (GTest) if applicable. This binds the CMakeLists.txt on the test project.
option(BUILD_TESTS "Build tests" OFF)

if (BUILD_TESTS)
    add_subdirectory(libs/googletest)
    # add_subdirectory(test)
endif()
