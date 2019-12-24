########################################################################
# Build Soapy SDR support module for NetSDR
########################################################################
cmake_minimum_required(VERSION 3.1)

project(SoapyNetSDR CXX)

if(CMAKE_COMPILER_IS_GNUCXX)

    #disable warnings for unused parameters
    add_definitions(-Wno-unused-parameter)

endif(CMAKE_COMPILER_IS_GNUCXX)

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
