#include "SoapyNetSDR.hpp"
#include <SoapySDR/Logger.hpp>
#include <SoapySDR/Formats.hpp>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cstddef>

SoapySDR::Stream *SoapyNetSDR::setupStream(
	const int direction,
	const std::string &format,
	const std::vector<size_t> &channels,
	const SoapySDR::Kwargs &args )
{
	std::lock_guard<std::mutex> lock(_device_mutex);

	//fprintf(stderr, "setupStream direction %d format %s channels %lu args %lu\n",direction,format.c_str(),
	//		channels.size(),args.size());

	if (direction != SOAPY_SDR_RX)return NULL;

	if (format != SOAPY_SDR_CF32)return NULL;

	return RX_STREAM;
}

int SoapyNetSDR::processUDP(float *datao)
{
	//fprintf(stderr, "SoapyNetSDR::ProcessUDP\n");

	unsigned char data[(4+1440)*2];

	socklen_t addrlen = sizeof(host_sa); // length of addresses

	errno = 0;

	int nbytes = recvfrom(_udp, (char *)data, sizeof(data), 0, (struct sockaddr *)&host_sa, &addrlen);

	if ( nbytes <= 0 ) {
		return nbytes;
	}

	uint16_t sequence = *((uint16_t *)(data + 2));

	uint16_t diff = sequence - _sequence;

	if ( diff > 1 )
	{
		printf("Netsdr Lost %d packets\n",diff );
	}

	_sequence = (0xffff == sequence) ? 0 : sequence;

	if ( (0x04 == data[0] && (0x84 == data[1] || 0x82 == data[1])) )
	{
		int ndata = (nbytes - 4) / 4;

		//fprintf(stderr, "nbytes %d ndata %d\n",nbytes,ndata);

		float scale = 1.0f/32768.0f;
		int16_t *in = (int16_t *)&data[4];
		for (int n = 0; n < ndata; ++n) {
			datao[2 * n + 0] = scale * (*in++);
			datao[2 * n + 1] = scale * (*in++);
		}

		return ndata;
	}

		else if ( (0xA4 == data[0] && 0x85 == data[1]) ||
			(0x84 == data[0] && 0x81 == data[1]) )
	{

		int ndata = (nbytes - 4) / 6;

		union {
			unsigned char c[4];
			int i;
			unsigned int u;
		} uu;

		int n = 0;
		for (int i = 4; i < nbytes; i += 6)
		{
			uu.u = 0;
			uu.c[0] = data[i + 0];
			uu.c[1] = data[i + 1];
			uu.c[2] = data[i + 2];
			if (uu.c[2] & 0x80) {
				uu.c[3] = 0xff;
			}
			datao[2 * n + 0] = (float)uu.i/8388608.0f;
			uu.u = 0;
			uu.c[0] = data[i + 3];
			uu.c[1] = data[i + 4];
			uu.c[2] = data[i + 5];
			if (uu.c[2] & 0x80) {
				uu.c[3] = 0xff;
			}
			datao[2 * n + 1] = (float)uu.i/8388608.0f;
			++n;
		}

		//fprintf(stderr, "ndata %d n %d\n",ndata,n);

		return ndata;
	}

/*
		else if ( (0xA4 == data[0] && 0x85 == data[1]) ||
			(0x84 == data[0] && 0x81 == data[1]) )
	{

		int ndata = (nbytes - 4) / 6;

typedef union
{
	struct bs
	{
		unsigned char b0;
		unsigned char b1;
		unsigned char b2;
		unsigned char b3;
	}bytes;
	int all;
}ByteToLong;

	ByteToLong dat;

		int n = 0;
		for (int i = 4; i < nbytes; i += 6)
		{
			dat.all = 0;
			dat.bytes.b1 = data[i];
			dat.bytes.b2 = data[i+1];
			dat.bytes.b3 = data[i+2];
			datao[2*n] = (float)dat.all/65536.0f;
			dat.all = 0;
			dat.bytes.b1 = data[i+3];
			dat.bytes.b2 = data[i+4];
			dat.bytes.b3 = data[i+5];
			datao[2*n+1] = (float)dat.all/65536.0f;
			n += 2;
		}

		return ndata;
	}
*/
	else {
		return 0;
	}

	return -1;
}

int SoapyNetSDR::activateStream(
	SoapySDR::Stream *stream,
	const int flags,
	const long long timeNs,
	const size_t numElems )
{
	std::lock_guard<std::mutex> lock(_device_mutex);

	fprintf(stderr, "activateStream + start %p %d %lld %zu\n",stream,flags,timeNs,numElems);

	datacount = 0;

	start();

	return 0;
}

int SoapyNetSDR::readStream(
	SoapySDR::Stream *stream,
	void * const *buffs,
	const size_t numElems,
	int &flags,
	long long &timeNs,
	const long timeoutUs )
{
	// fill in the select structures
	struct timeval tv;
	tv.tv_sec = timeoutUs / 1000000;
	tv.tv_usec = timeoutUs % 1000000;

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(_udp, &fds);

	// wait for timeout
	int ret = ::select((int)(_udp+1), NULL, &fds, NULL, &tv);
	if (ret < 0) return SOAPY_SDR_STREAM_ERROR;
	if (ret == 0) return SOAPY_SDR_TIMEOUT;

	float *out = (float *)buffs[0];

	size_t nn = numElems;

	if (datacount) {
		size_t nd = datasize - datacount;
		//fprintf(stderr, "t numElems %lu datacount %lu nn %lu nd %d datasize %lu \n", numElems, datacount, nn, nd, datasize);
		for (size_t n = nd; n < datasize; ++n) {
			if (nn == 0) break;
			*out++ = datasave[2 * n + 0];
			*out++ = datasave[2 * n + 1];
			--nn;
			--datacount;
		}
		//fprintf(stderr, "t numElems %lu datacount %lu nn %lu\n", numElems, datacount, nn);
		if (nn == 0) return (int)numElems;
		datacount = 0;
	}

	while (nn >= datasize) {
		int ret = processUDP(out);
		if (ret > 0) {
			out += 2 * ret;
			nn -= ret;
		}
	}

	if (nn > 0) {
		int ret = processUDP(datasave);
		if (ret > 0) {
			for (size_t n = 0; n < nn; ++n) {
				*out++ = datasave[2 * n + 0];
				*out++ = datasave[2 * n + 1];
				ret--;
			}
			datacount = ret;
			if (ret < 0) datacount = 0;
			//fprintf(stderr, "b numElems %lu datacount %lu ret %d nn %lu\n",numElems,datacount,ret,nn);
		}
	}

	return (int)numElems;
}

int SoapyNetSDR::deactivateStream(
	SoapySDR::Stream *stream,
	const int flags,
	const long long timeNs )
{
	std::lock_guard<std::mutex> lock(_device_mutex);
	fprintf(stderr, "deactivateStream\n");
	stop();
	return 0;
}

void SoapyNetSDR::closeStream( SoapySDR::Stream *stream )
{
	std::lock_guard<std::mutex> lock(_device_mutex);
	if (stream) {
	}
	fprintf(stderr, "closeStream\n");
}

size_t SoapyNetSDR::getStreamMTU( SoapySDR::Stream *stream ) const
{
	fprintf(stderr, "getStreamMTU\n");
	return 0;
}

std::string SoapyNetSDR::getNativeStreamFormat(const int direction, const size_t channel, double &fullScale) const
{
	fullScale = 1.0;
	fprintf(stderr, "getNativeStreamFormat\n");
	return SOAPY_SDR_CF32;
}

std::vector<std::string> SoapyNetSDR::getStreamFormats(const int direction, const size_t channel) const
{
	std::vector<std::string> formats;

	formats.push_back(SOAPY_SDR_CF32);
	fprintf(stderr, "getStreamFormats\n");

	return formats;
}
