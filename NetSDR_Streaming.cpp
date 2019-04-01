#include "SoapyNetSDR.hpp"
#include <SoapySDR/Logger.hpp>
#include <SoapySDR/Formats.hpp>
#include <chrono>
#include <thread>
#include <algorithm>
//#include <stdio.h>

int SoapyNetSDR::readStream(
	SoapySDR::Stream *stream,
	void * const *buffs,
	const size_t numElems,
	int &flags,
	long long &timeNs,
	const long timeoutUs )
{
	return -1;
}
void SoapyNetSDR::closeStream( SoapySDR::Stream *stream )
{
	std::lock_guard<std::mutex> lock(_device_mutex);
	if (stream) {
	}
}
SoapySDR::Stream *SoapyNetSDR::setupStream(
	const int direction,
	const std::string &format,
	const std::vector<size_t> &channels,
	const SoapySDR::Kwargs &args )
{
	std::lock_guard<std::mutex> lock(_device_mutex);
	
	return NULL;
}
int SoapyNetSDR::activateStream(
	SoapySDR::Stream *stream,
	const int flags,
	const long long timeNs,
	const size_t numElems )
{

	std::lock_guard<std::mutex> lock(_device_mutex);
	return -1;
}
int SoapyNetSDR::deactivateStream(
	SoapySDR::Stream *stream,
	const int flags,
	const long long timeNs )
{


		std::lock_guard<std::mutex> lock(_device_mutex);
	return -1;
}

size_t SoapyNetSDR::getStreamMTU( SoapySDR::Stream *stream ) const
{
	return 0;
}
std::string SoapyNetSDR::getNativeStreamFormat(const int direction, const size_t channel, double &fullScale) const
{
	fullScale = 128;
	return SOAPY_SDR_CS8;
}

std::vector<std::string> SoapyNetSDR::getStreamFormats(const int direction, const size_t channel) const
{
	std::vector<std::string> formats;

	formats.push_back(SOAPY_SDR_CS8);
	formats.push_back(SOAPY_SDR_CS16);
	formats.push_back(SOAPY_SDR_CF32);
	formats.push_back(SOAPY_SDR_CF64);

	return formats;
}

