########################################################################
# Build Soapy SDR support module for NetSDR
########################################################################
cmake_minimum_required(VERSION 3.1)
project(SoapyNetSDR CXX)

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
