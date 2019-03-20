#include "SoapyNetSDR.hpp"
#include <SoapySDR/Registry.hpp>

/***********************************************************************
 * Find available devices
 **********************************************************************/
static SoapySDR::KwargsList find_netSDR(const SoapySDR::Kwargs &args)
{
    SoapySDR::KwargsList results;

    return results;
}

/***********************************************************************
 * Make device instance
 **********************************************************************/
static SoapySDR::Device *make_netSDR(const SoapySDR::Kwargs &args)
{
    return new SoapyNetSDR(args);
}

/***********************************************************************
 * Registration
 **********************************************************************/
static SoapySDR::Registry register_netsdr("netsdr", &find_netSDR, &make_netSDR, SOAPY_SDR_ABI_VERSION);
