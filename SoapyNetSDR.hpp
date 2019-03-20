
#pragma once
#include <SoapySDR/Device.hpp>

class SoapyNetSDR : public SoapySDR::Device
{
public:
    SoapyNetSDR(const SoapySDR::Kwargs &args);
    ~SoapyNetSDR(void);

private:
    //sockets here
};
