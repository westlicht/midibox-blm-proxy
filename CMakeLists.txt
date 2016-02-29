cmake_minimum_required(VERSION 2.8)

project(blm-proxy)

# load custom cmake modules
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# enable C++11
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

set(LIBS "")

if(APPLE)
    add_definitions("-D__MACOSX_CORE__")
    set(LIBS "${LIBS} -framework CoreMIDI -framework CoreFoundation -framework CoreAudio")
elseif(UNIX)
endif()


set(SOURCES
    src/BLM.cpp
    src/LaunchpadController.cpp
    src/Midi.cpp
    src/RtMidi.cpp
    src/main.cpp
)

add_executable(blm-proxy ${SOURCES})
target_link_libraries(blm-proxy ${LIBS})

