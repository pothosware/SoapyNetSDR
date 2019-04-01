
#pragma once
#include <SoapySDR/Device.hpp>
#include "SoapyRPCSocket.hpp"
#include "SoapyURLUtils.hpp"

class SoapyNetSDR : public SoapySDR::Device
{
public:
    SoapyNetSDR(const SoapySDR::Kwargs &args);
    ~SoapyNetSDR(void);

    /*******************************************************************
     * Identification API
     ******************************************************************/
    std::string getDriverKey(void) const;

    std::string getHardwareKey(void) const;

    SoapySDR::Kwargs getHardwareInfo(void) const;

    /*******************************************************************
     * Channels API
     ******************************************************************/
    size_t getNumChannels(const int direction) const;

    bool getFullDuplex(const int direction, const size_t channel) const;

    /*******************************************************************
     * Stream API
     ******************************************************************/
    std::vector<std::string> getStreamFormats(const int direction, const size_t channel) const;

    std::string getNativeStreamFormat(const int direction, const size_t channel, double &fullScale) const;

    SoapySDR::Stream *setupStream(
        const int direction,
        const std::string &format,
        const std::vector<size_t> &channels,
        const SoapySDR::Kwargs &args);

    void closeStream(SoapySDR::Stream *stream);

    size_t getStreamMTU(SoapySDR::Stream *stream) const;

    int activateStream(
        SoapySDR::Stream *stream,
        const int flags,
        const long long timeNs,
        const size_t numElems);

    int deactivateStream(
        SoapySDR::Stream *stream,
        const int flags,
        const long long timeNs);

    int readStream(
        SoapySDR::Stream *stream,
        void * const *buffs,
        const size_t numElems,
        int &flags,
        long long &timeNs,
        const long timeoutUs);

    /*******************************************************************
     * Antenna API
     ******************************************************************/
    std::vector<std::string> listAntennas(const int direction, const size_t channel) const;

    void setAntenna(const int direction, const size_t channel, const std::string &name);

    std::string getAntenna(const int direction, const size_t channel) const;

    /*******************************************************************
     * Frontend corrections API
     ******************************************************************/
   // bool hasFrequencyCorrection(const int direction, const size_t channel) const;

   // void setFrequencyCorrection(const int direction, const size_t channel, const double value);

   // double getFrequencyCorrection(const int direction, const size_t channel) const;

    /*******************************************************************
     * Gain API
     ******************************************************************/
    std::vector<std::string> listGains(const int direction, const size_t channel) const;

    //bool hasGainMode(const int direction, const size_t channel) const;

    void setGainMode(const int direction, const size_t channel, const bool automatic);

    bool getGainMode(const int direction, const size_t channel) const;

    void setGain(const int direction, const size_t channel, const double value);

    void setGain(const int direction, const size_t channel, const std::string &name, const double value);

    double getGain(const int direction, const size_t channel) const;

    double getGain(const int direction, const size_t channel, const std::string &name) const;

    SoapySDR::Range getGainRange(const int direction, const size_t channel) const;

    SoapySDR::Range getGainRange(const int direction, const size_t channel, const std::string &name) const;

    /*******************************************************************
     * Frequency API
     ******************************************************************/
    void setFrequency(const int direction, const size_t channel, const double frequency, const SoapySDR::Kwargs &args);

    void setFrequency(const int direction, const size_t channel, const std::string &name, const double frequency, const SoapySDR::Kwargs &args);

    double getFrequency(const int direction, const size_t channel) const;

    double getFrequency(const int direction, const size_t channel, const std::string &name) const;

    std::vector<std::string> listFrequencies(const int direction, const size_t channel) const;

    SoapySDR::RangeList getFrequencyRange(const int direction, const size_t channel) const;

    SoapySDR::RangeList getFrequencyRange(const int direction, const size_t channel, const std::string &name) const;

    /*******************************************************************
     * Sample Rate API
     ******************************************************************/
    void setSampleRate(const int direction, const size_t channel, const double rate);

    double getSampleRate(const int direction, const size_t channel) const;

    std::vector<double> listSampleRates(const int direction, const size_t channel) const;

    //SoapySDR::RangeList getSampleRateRange(const int direction, const size_t channel) const;

    /*******************************************************************
     * Bandwidth API
     ******************************************************************/
    void setBandwidth(const int direction, const size_t channel, const double bw);

    double getBandwidth(const int direction, const size_t channel) const;

    std::vector<double> listBandwidths(const int direction, const size_t channel) const;

    //SoapySDR::RangeList getBandwidthRange(const int direction, const size_t channel) const;

private:
    //sockets here
    
    	mutable std::mutex	_device_mutex;

};
