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

set(EXTRA_LIBS "")

if(APPLE)
    find_library(COREMIDI_LIBRARY CoreMIDI)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    find_library(COREAUDIO_LIBRARY CoreAudio)
    set(EXTRA_LIBS ${COREMIDI_LIBRARY} ${COREFOUNDATION_LIBRARY} ${COREAUDIO_LIBRARY})
    add_definitions("-D__MACOSX_CORE__")
elseif(UNIX)
    find_package(ALSA REQUIRED)
    set(EXTRA_LIBS -lpthread ${ALSA_LIBRARIES})
    include_directories(${ALSA_INCLUDE_DIRS})
    add_definitions("-D__LINUX_ALSA__")
endif()


set(SOURCES
    ext/RtMidi/RtMidi.cpp
    ext/json11/json11.cpp
    src/BLM.cpp
    src/Debug.cpp
    src/LaunchpadController.cpp
    src/LaunchpadDevice.cpp
    src/Midi.cpp
    src/Timer.cpp
    src/Settings.cpp
    src/UdpSocket.cpp
    src/main.cpp
)

include_directories(ext)

add_executable(blm-proxy ${SOURCES})
target_link_libraries(blm-proxy ${EXTRA_LIBS})

