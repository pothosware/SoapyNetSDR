
#pragma once
#include <SoapySDR/Device.hpp>
#include "SocketDefs.h"
#include <cstring>
#include <mutex>

struct SoapyNetSDR_SocketInit
{
    SoapyNetSDR_SocketInit(void)
    {
        #ifdef _MSC_VER
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        #endif
    }
    ~SoapyNetSDR_SocketInit(void)
    {
        #ifdef _MSC_VER
        WSACleanup();
        #endif
    }
};

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

    double getGain(const int direction, const size_t channel);

    double getGain(const int direction, const size_t channel, const std::string &name);

    SoapySDR::Range getGainRange(const int direction, const size_t channel) const;

    SoapySDR::Range getGainRange(const int direction, const size_t channel, const std::string &name) const;

    /*******************************************************************
     * Frequency API
     ******************************************************************/
    void setFrequency(const int direction, const size_t channel, const double frequency, const SoapySDR::Kwargs &args);

    void setFrequency(const int direction, const size_t channel, const std::string &name, const double frequency, const SoapySDR::Kwargs &args);

    double getFrequency(const int direction, const size_t channel);

    double getFrequency(const int direction, const size_t channel, const std::string &name);

    std::vector<std::string> listFrequencies(const int direction, const size_t channel) const;

    SoapySDR::RangeList getFrequencyRange(const int direction, const size_t channel);

    SoapySDR::RangeList getFrequencyRange(const int direction, const size_t channel, const std::string &name);

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

   bool transaction( const unsigned char *cmd, size_t size );

  	bool transaction( const unsigned char *cmd, size_t size,
                    std::vector< unsigned char > &response );

   	bool start();

	bool stop();

	void apply_channel( unsigned char *cmd, size_t chan );

	int processUPD(float *data);

SOCKET _tcp;
SOCKET _udp;

struct sockaddr_in host_sa; /* local address */

private:
    SoapyNetSDR_SocketInit socket_init;

    mutable std::mutex	_device_mutex;
    std::mutex	_tcp_lock;

  enum radio_type
  {
    RADIO_UNKNOWN = 0,
    RFSPACE_SDR_IQ,
    RFSPACE_SDR_IP,
    RFSPACE_NETSDR,
    RFSPACE_CLOUDIQ
  };

  radio_type _radio;


  bool _running;
  bool _keep_running;
  uint16_t _sequence;

  size_t _nchan;
  double _sample_rate;
  double _bandwidth;

  SoapySDR::Stream* const RX_STREAM = (SoapySDR::Stream*) 0x2;

	float datasave[256*2*sizeof(float)];
	size_t datacount;
	size_t datasize;

};
