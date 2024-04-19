########################################################################
# Build Soapy SDR support module for NetSDR
########################################################################
cmake_minimum_required(VERSION 3.1)

project(SoapyNetSDR CXX)

#enable c++11 features
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")

    #C++11 is a required language feature for this project
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=c++11" HAS_STD_CXX11)
    if(HAS_STD_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    else(HAS_STD_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    endif()

    #Thread support enabled (not the same as -lpthread)
    list(APPEND RTLSDR_LIBRARIES -pthread)

    #disable warnings for unused parameters
    add_compile_options(-Wall -Wextra -Wno-unused-parameter -Wno-overloaded-virtual)
endif()

if(APPLE)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wc++11-extensions")
endif(APPLE)

find_package(SoapySDR CONFIG)
if (NOT SoapySDR_FOUND)
    message(FATAL_ERROR "Soapy SDR development files not found...")
endif ()

SOAPY_SDR_MODULE_UTIL(
    TARGET netSDRSupport
    SOURCES
        NetSDR_Registration.cpp
        NetSDR_Settings.cpp
        NetSDR_Streaming.cpp
)

#network libraries
if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
    target_link_libraries(netSDRSupport PRIVATE ws2_32)
endif (WIN32)

#network interface enumeration
if (WIN32)
    target_sources(netSDRSupport PRIVATE InterfacesWindows.cpp)
else ()
    target_sources(netSDRSupport PRIVATE InterfacesUnix.cpp)
endif ()
