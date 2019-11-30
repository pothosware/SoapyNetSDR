#include "SoapyNetSDR.hpp"
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <cerrno>
#include <vector>
#include <chrono>
#include <thread>

#define DEFAULT_HOST  "127.0.0.1" /* We assume a running "siqs" from CuteSDR project */
#define DEFAULT_PORT  50000

static SOCKET connectToServer(char *serverName,unsigned short *Port);

SoapyNetSDR::SoapyNetSDR(const SoapySDR::Kwargs &args)
{
 	const SoapySDR::Kwargs options;
	if(args.size()){
	/*
		printf("driver args %lu ",args.count("driver"));
		if(args.count("driver"))printf(" %s \n",args.at("driver").c_str());
		printf("label args %lu ",args.count("label"));
		if(args.count("label"))printf(" %s \n",args.at("label").c_str());
		printf("netsdr args %lu ",args.count("netsdr"));
		if(args.count("netsdr"))printf(" %s \n",args.at("netsdr").c_str());
	*/
	}

  _tcp=-1;
  _udp=-1;
  _running=false;
  _keep_running=false;
  _sequence=0;
  _nchan=1;
  _sample_rate=0;
  _bandwidth=0.0;



    std::string host=args.at("netsdr");

	unsigned short Port;

	_tcp=(SOCKET)connectToServer((char *)host.c_str(),&Port);
    if(_tcp == -1){
      fprintf(stderr,"connect failed\n");
      throw std::runtime_error(socket_strerror(SOCKET_ERRNO));
	}


    if ( (_udp = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
    {
      closesocket(_tcp);
      throw std::runtime_error("Could not create UDP socket");
    }

    int sockoptval = 1;
    setsockopt(_udp, SOL_SOCKET, SO_REUSEADDR, (const char *)&sockoptval, sizeof(int));

    /* fill in the hosts's address and data */
    memset(&host_sa, 0, sizeof(host_sa));
    host_sa.sin_family = AF_INET;
    host_sa.sin_addr.s_addr = htonl(INADDR_ANY);
    host_sa.sin_port = htons(DEFAULT_PORT);

    if ( bind(_udp, (struct sockaddr *)&host_sa, sizeof(host_sa)) < 0 )
    {
      closesocket(_tcp);
      closesocket(_udp);
      throw std::runtime_error("Bind of UDP socket failed: " + std::string(socket_strerror(SOCKET_ERRNO)));
    }


  std::vector< unsigned char > response;

  {
    std::cerr << "Using ";

    unsigned char name[] = { 0x04, 0x20, 0x01, 0x00 }; /* NETSDR 4.1.1 Target Name */
    if ( transaction( name, sizeof(name), response ) )
      std::cerr << "RFSPACE " << &response[sizeof(name)] << " ";

    unsigned char sern[] = { 0x04, 0x20, 0x02, 0x00 }; /* NETSDR 4.1.2 Target Serial Number */
    if ( transaction( sern, sizeof(sern), response ) )
      std::cerr << "SN " << &response[sizeof(sern)] << " ";
  }

  unsigned char prod[] = { 0x04, 0x20, 0x09, 0x00 }; /* NETSDR 4.1.6 Product ID */
  if ( transaction( prod, sizeof(prod), response ) )
  {
    uint32_t product_id = htonl(*((uint32_t *)&response[sizeof(prod)]));
//    std::cerr << std::hex << product_id << std::dec << " ";

    if ( 0x5affa500 == product_id ) /* SDR-IQ 5.1.6 Product ID */
      _radio = RFSPACE_SDR_IQ;
    else if ( 0x53445203 == product_id ) /* SDR-IP 4.1.6 Product ID */
      _radio = RFSPACE_SDR_IP;
    else if ( 0x53445204 == product_id ) /* NETSDR 4.1.6 Product ID */
      _radio = RFSPACE_NETSDR;
    else if ( 0x434C4951 == product_id ) /* CloudIQ Product ID */
      _radio = RFSPACE_CLOUDIQ;
    else
      std::cerr << "UNKNOWN ";
  }

  bool has_X2_option = false;

  if ( RFSPACE_NETSDR == _radio )
  {
    unsigned char opts[] = { 0x04, 0x20, 0x0A, 0x00 }; /* NETSDR 4.1.7 Options */
    if ( transaction( opts, sizeof(opts), response ) )
    {
      if ( response[sizeof(opts)] )
      {
        has_X2_option = (response[sizeof(opts)] & 16 ? true : false);

        std::cerr << "option ";
        std::cerr << (response[sizeof(opts)] & 16 ? "2" : "-"); /* X2 board */
        std::cerr << (response[sizeof(opts)] &  8 ? "U" : "-"); /* Up Converter */
        std::cerr << (response[sizeof(opts)] &  4 ? "D" : "-"); /* Down Converter */
        std::cerr << (response[sizeof(opts)] &  2 ? "R" : "-"); /* Reflock board */
        std::cerr << (response[sizeof(opts)] &  1 ? "S" : "-"); /* Sound Enabled */
        std::cerr << " ";
      }
    }
  }
  /* NETSDR 4.1.4 Hardware/Firmware Versions */

  unsigned char bootver[] = { 0x05, 0x20, 0x04, 0x00, 0x00 };
  if ( transaction( bootver, sizeof(bootver), response ) )
    std::cerr << "BOOT " << *((uint16_t *)&response[sizeof(bootver)]) << " ";

  unsigned char firmver[] = { 0x05, 0x20, 0x04, 0x00, 0x01 };
  if ( transaction( firmver, sizeof(firmver), response ) )
    std::cerr << "FW " << *((uint16_t *)&response[sizeof(firmver)]) << " ";


  if ( RFSPACE_NETSDR == _radio ||
       RFSPACE_SDR_IP == _radio ||
       RFSPACE_CLOUDIQ == _radio)
  {
    unsigned char hardver[] = { 0x05, 0x20, 0x04, 0x00, 0x02 };
    if ( transaction( hardver, sizeof(hardver), response ) )
      std::cerr << "HW " << *((uint16_t *)&response[sizeof(hardver)]) << " ";
  }

  if ( RFSPACE_NETSDR == _radio ||
       RFSPACE_CLOUDIQ == _radio)
  {
    unsigned char fpgaver[] = { 0x05, 0x20, 0x04, 0x00, 0x03 };
    if ( transaction( fpgaver, sizeof(fpgaver), response ) )
      std::cerr << "FPGA " << int(response[sizeof(fpgaver)])
                << "/" << int(response[sizeof(fpgaver)+1]) << " ";
  }

  std::cerr << std::endl;


  if ( RFSPACE_NETSDR == _radio )
  {
    /* NETSDR 4.2.2 Receiver Channel Setup */
    unsigned char rxchan[] = { 0x05, 0x00, 0x19, 0x00, 0x00 };

    unsigned char mode = 0; /* 0 = Single Channel Mode */

    if ( 2 == _nchan )
    {
      if ( has_X2_option )
        mode = 6; /* Dual Channel with dual A/D RF Path (requires X2 option) */
      else
        mode = 4; /* Dual Channel with single A/D RF Path using main A/D. */

      //set_output_signature( gr::io_signature::make (2, 2, sizeof (gr_complex)) );
    }

    rxchan[sizeof(rxchan)-1] = mode;
    transaction( rxchan, sizeof(rxchan) );

    // fprintf(stderr,"mode %d\n",mode);
  }

}
static int copyl(char *p1,char *p2,long n)
{
	if(!p1 || !p2)return 1;

	while(n-- > 0)*p2++ = *p1++;

	return 0;
}

static SOCKET connectToServer(char *serverName,unsigned short *Port)
{
	struct sockaddr_in serverSocketAddr;
	struct hostent *serverHostEnt;
	SOCKET toServerSocket;
	int ret;
	unsigned int hostAddr;
	unsigned int oneNeg;
	short result,Try;
	char *np;
	int buf_size;

	/* oneNeg=0xffffffff; */
	oneNeg = -1L;

	long netsize=200000;

	buf_size=(int)(netsize+30);

	result= -1;

    memset(&serverSocketAddr, 0, sizeof(serverSocketAddr));

	if(!(np=strrchr(serverName,':'))){
	    printf("Bad Address (%s)",serverName);
	    return result;
	}else{
	    *np=0;
	    np += 1;
	    *Port=(unsigned short)atol(np);
	}
	hostAddr=(unsigned int)inet_addr(serverName);
	if((long)hostAddr != (long)oneNeg){
	    serverSocketAddr.sin_addr.s_addr=hostAddr;
	    printf("Found Address %lx hostAddr %x oneNeg %x diff %x\n",(long)hostAddr,hostAddr,oneNeg,hostAddr-oneNeg);
	}else{
	    serverHostEnt=gethostbyname(serverName);
	    if(serverHostEnt == NULL){
	        printf("Could Not Find Host (%s)\n",serverName);
	        return result;
	    }
	    copyl((char *)serverHostEnt->h_addr,(char *)&serverSocketAddr.sin_addr,serverHostEnt->h_length);
	}
	serverSocketAddr.sin_family=AF_INET;
	serverSocketAddr.sin_port=htons(*Port);
	Try=0;
	while(Try++ < 10){
	    if((toServerSocket=socket(AF_INET,SOCK_STREAM,0)) < 0){
            printf("socket Error  (%ld)\n",(long)SOCKET_ERRNO);
	        return toServerSocket;
	    }

	    ret=setsockopt( toServerSocket, SOL_SOCKET, SO_SNDBUF,
                  (char *)&buf_size, sizeof(int) );
        if(ret < 0)printf("setsockopt SO_SNDBUF failed\n");

	    ret=connect(toServerSocket,(struct sockaddr *)&serverSocketAddr,sizeof(serverSocketAddr));
	    if(ret == -1){
                if (SOCKET_ERRNO == SOCKET_ECONNREFUSED)  {
                    printf("Connection Refused  Try(%d)\n",Try);
                    closesocket(toServerSocket);
                    std::this_thread::sleep_for(std::chrono::seconds(4));
                    continue;
                }else{
                    printf("Connection Error  (%ld)\n",(long)SOCKET_ERRNO);
                    return ret;
                }
	    }
	    return toServerSocket;
	}

       return ret;
}
bool SoapyNetSDR::transaction( const unsigned char *cmd, size_t size )
{
  std::vector< unsigned char > response;

  if ( ! transaction( cmd, size, response ) )
    return false;

  /* comparing the contents is not really feasible due to protocol */
  if ( response.size() == size ) /* check response size against request */
    return true;

  return false;
}

//#define VERBOSE

bool SoapyNetSDR::transaction( const unsigned char *cmd, size_t size,
                                   std::vector< unsigned char > &response )
{
  size_t rx_bytes = 0;
  unsigned char data[1024*2];

  response.clear();

#ifdef VERBOSE
  printf("< ");
  for (size_t i = 0; i < size; i++)
    printf("%02x ", (unsigned char) cmd[i]);
  printf("\n");
#endif

  {
    std::lock_guard<std::mutex> lock(_tcp_lock);


    if ( send(_tcp, (const char *)cmd, size, 0) != (int)size )
      return false;

    int nbytes = recv(_tcp, (char *)data, 2, 0); /* read header */
    if ( nbytes != 2 )
      return false;

    int length = (data[1] & 0x1f) | data[0];

    if ( (length < 2) || (length > (int)sizeof(data)) )
      return false;

    length -= 2; /* subtract header size */

    nbytes = recv(_tcp, (char *)&data[2], length, 0); /* read payload */
    if ( nbytes != length )
      return false;

    rx_bytes = 2 + length; /* header + payload */
  }

  response.resize( rx_bytes );
  memcpy( response.data(), data, rx_bytes );

#ifdef VERBOSE
  printf("> ");
  for (size_t i = 0; i < rx_bytes; i++)
    printf("%02x ", (unsigned char) data[i]);
  printf("\n");
#endif

  return true;
}

SoapyNetSDR::~SoapyNetSDR(void)
{
	closesocket(_tcp);
  	closesocket(_udp);
}
void SoapyNetSDR::setAntenna( const int direction, const size_t channel, const std::string &name )
{
	return;
}
void SoapyNetSDR::setGainMode( const int direction, const size_t channel, const bool automatic )
{
	/* enable AGC if the hardware supports it, or remove this function */
}
bool SoapyNetSDR::getGainMode( const int direction, const size_t channel ) const
{
	return(false);
	/* ditto for the AGC */
}

#define BANDWIDTH 34e6

void SoapyNetSDR::setBandwidth( const int direction, const size_t channel, const double bandwidth )
{
	std::lock_guard<std::mutex> lock(_device_mutex);

  /* SDR-IP 4.2.5 RF Filter Selection */
  /* NETSDR 4.2.7 RF Filter Selection */
  unsigned char filter[] = { 0x06, 0x00, 0x44, 0x00, 0x00, 0x00 };

  apply_channel( filter, channel );

  if ( 0.0f == bandwidth )
  {
    _bandwidth = 0.0f;
    filter[sizeof(filter)-1] = 0x00; /* Select bandpass filter based on NCO frequency */
  }
  else if ( BANDWIDTH == bandwidth )
  {
    _bandwidth = BANDWIDTH;
    filter[sizeof(filter)-1] = 0x0B; /* Bypass bandpass filter, use only antialiasing */
  }

  transaction( filter, sizeof(filter) );


}


void SoapyNetSDR::apply_channel( unsigned char *cmd, size_t chan )
{
  unsigned char value = 0;

  if ( 0 == chan )
  {
    value = 0;
  }
  else if ( 1 == chan )
  {
    if ( _nchan < 2 )
      throw std::runtime_error("Channel must be 0 only");

    value = 2;
  }
  else
    throw std::runtime_error("Channel must be 0 or 1");

  cmd[4] = value;
}

void SoapyNetSDR::setFrequency( const int direction, const size_t channel, const std::string &name, const double frequency, const SoapySDR::Kwargs &args )
{
	std::lock_guard<std::mutex> lock(_device_mutex);

  uint32_t u32_freq = frequency;

  /* SDR-IQ 5.2.2 Receiver Frequency */
  /* SDR-IP 4.2.2 Receiver Frequency */
  /* NETSDR 4.2.3 Receiver Frequency */
  unsigned char tune[] = { 0x0A, 0x00, 0x20, 0x00, 0x00, 0xb0, 0x19, 0x6d, 0x00, 0x00 };

  apply_channel( tune, channel );

  tune[sizeof(tune)-5] = u32_freq >>  0;
  tune[sizeof(tune)-4] = u32_freq >>  8;
  tune[sizeof(tune)-3] = u32_freq >> 16;
  tune[sizeof(tune)-2] = u32_freq >> 24;
  tune[sizeof(tune)-1] = 0;

  transaction( tune, sizeof(tune) );

}
void SoapyNetSDR::setFrequency(const int direction, const size_t channel, const double frequency, const SoapySDR::Kwargs &args)
{
	std::lock_guard<std::mutex> lock(_device_mutex);

  uint32_t u32_freq = frequency;

  /* SDR-IQ 5.2.2 Receiver Frequency */
  /* SDR-IP 4.2.2 Receiver Frequency */
  /* NETSDR 4.2.3 Receiver Frequency */
  unsigned char tune[] = { 0x0A, 0x00, 0x20, 0x00, 0x00, 0xb0, 0x19, 0x6d, 0x00, 0x00 };

  apply_channel( tune, channel );

  tune[sizeof(tune)-5] = u32_freq >>  0;
  tune[sizeof(tune)-4] = u32_freq >>  8;
  tune[sizeof(tune)-3] = u32_freq >> 16;
  tune[sizeof(tune)-2] = u32_freq >> 24;
  tune[sizeof(tune)-1] = 0;

  transaction( tune, sizeof(tune) );


}
void SoapyNetSDR::setSampleRate( const int direction, const size_t channel, const double rate )
{
	std::lock_guard<std::mutex> lock(_device_mutex);
  /* SDR-IQ 5.2.4 I/Q Data Output Sample Rate */
  /* SDR-IP 4.2.8 DDC Output Sample Rate */
  /* NETSDR 4.2.9 I/Q Output Data Sample Rate */
  unsigned char samprate[] = { 0x09, 0x00, 0xB8, 0x00, 0x00, 0x20, 0xA1, 0x07, 0x00 };

  uint32_t u32_rate = rate;
  samprate[sizeof(samprate)-4] = u32_rate >>  0;
  samprate[sizeof(samprate)-3] = u32_rate >>  8;
  samprate[sizeof(samprate)-2] = u32_rate >> 16;
  samprate[sizeof(samprate)-1] = u32_rate >> 24;

  std::vector< unsigned char > response;

  if ( _running )
  {
    _keep_running = true;

    stop();
  }

  if ( ! transaction( samprate, sizeof(samprate), response ) )
    throw std::runtime_error("set_sample_rate failed");

  if ( _running )
  {
    start();
  }

  u32_rate = 0;
  u32_rate |= response[sizeof(samprate)-4] <<  0;
  u32_rate |= response[sizeof(samprate)-3] <<  8;
  u32_rate |= response[sizeof(samprate)-2] << 16;
  u32_rate |= response[sizeof(samprate)-1] << 24;

  _sample_rate = u32_rate;


	if(_sample_rate <= 1333333){
		datasize=240;
	}else{
		datasize=256;
	}


	datacount=0;

  if ( rate != _sample_rate )
    std::cerr << "Radio reported a sample rate of " << (uint32_t)_sample_rate << " Hz"
              << "Requested rate " << rate << " Hz" <<std::endl;

}


bool SoapyNetSDR::start()
{
  _sequence = 0;
  _running = true;
  _keep_running = false;

  /* SDR-IP 4.2.1 Receiver State */
  /* NETSDR 4.2.1 Receiver State */
  unsigned char start[] = { 0x08, 0x00, 0x18, 0x00, 0x80, 0x02, 0x00, 0x00 };

  /* SDR-IQ 5.2.1 Receiver State */
  if ( RFSPACE_SDR_IQ == _radio )
    start[sizeof(start)-4] = 0x81;

  unsigned char mode = 0; /* 0 = 16 bit Contiguous Mode */

  if(_sample_rate <= 1333333){
  	mode = 0x80; /* 24 bit Contiguous mode */
  	datasize=240;
  }else{
  	datasize=256;
  }

  if ( 0 ) /* TODO: 24 bit Contiguous mode */
    mode |= 0x80;

  if ( 0 ) /* TODO: Hardware Triggered Pulse mode */
    mode |= 0x03;

  start[sizeof(start)-2] = mode;

  return transaction( start, sizeof(start) );
}

bool SoapyNetSDR::stop()
{
  if ( ! _keep_running )
    _running = false;
  _keep_running = false;

/*
  if ( _fifo )
    _fifo->clear();
*/

  /* SDR-IP 4.2.1 Receiver State */
  /* NETSDR 4.2.1 Receiver State */
  unsigned char stop[] = { 0x08, 0x00, 0x18, 0x00, 0x00, 0x01, 0x00, 0x00 };

  /* SDR-IQ 5.2.1 Receiver State */
  if ( RFSPACE_SDR_IQ == _radio )
    stop[sizeof(stop)-4] = 0x81;

  return transaction( stop, sizeof(stop) );
}
void SoapyNetSDR::setGain( const int direction, const size_t channel, const double gain )
{
	std::lock_guard<std::mutex> lock(_device_mutex);

  /* SDR-IQ 5.2.5 RF Gain */
  /* SDR-IP 4.2.3 RF Gain */
  /* NETSDR 4.2.6 RF Gain */
  unsigned char atten[] = { 0x06, 0x00, 0x38, 0x00, 0x00, 0x00 };

  apply_channel( atten, channel );

  if ( RFSPACE_SDR_IQ == _radio )
  {
    if ( gain <= -20 )
      atten[sizeof(atten)-1] = 0xE2;
    else if ( gain <= -10 )
      atten[sizeof(atten)-1] = 0xEC;
    else if ( gain <= 0 )
      atten[sizeof(atten)-1] = 0xF6;
    else /* +10 dB */
      atten[sizeof(atten)-1] = 0x00;
  }
  else /* SDR-IP & NETSDR */
  {
    if ( gain <= -30 )
      atten[sizeof(atten)-1] = 0xE2;
    else if ( gain <= -20 )
      atten[sizeof(atten)-1] = 0xEC;
    else if ( gain <= -10 )
      atten[sizeof(atten)-1] = 0xF6;
    else /* 0 dB */
      atten[sizeof(atten)-1] = 0x00;
  }

  transaction( atten, sizeof(atten) );


}
void SoapyNetSDR::setGain( const int direction, const size_t channel, const std::string &name, const double gain )
{
    setGain(direction,channel,gain );
}

std::string SoapyNetSDR::getAntenna( const int direction, const size_t channel ) const
{
	return("RX");
}

double SoapyNetSDR::getBandwidth( const int direction, const size_t channel ) const
{
	return (_bandwidth);
}

double SoapyNetSDR::getFrequency( const int direction, const size_t channel, const std::string &name )
{
	std::lock_guard<std::mutex> lock(_device_mutex);
  /* SDR-IQ 5.2.2 Receiver Frequency */
  /* SDR-IP 4.2.2 Receiver Frequency */
  /* NETSDR 4.2.3 Receiver Frequency */
  unsigned char freq[] = { 0x05, 0x20, 0x20, 0x00, 0x00 };

  apply_channel( freq, channel );

  std::vector< unsigned char > response;

  if ( ! transaction( freq, sizeof(freq), response ) )
    throw std::runtime_error("get_center_freq failed");

  uint32_t frequency = 0;
  frequency |= response[response.size()-5] <<  0;
  frequency |= response[response.size()-4] <<  8;
  frequency |= response[response.size()-3] << 16;
  frequency |= response[response.size()-2] << 24;

  return frequency;
}
double SoapyNetSDR::getFrequency(const int direction, const size_t channel)
{
	std::lock_guard<std::mutex> lock(_device_mutex);
  /* SDR-IQ 5.2.2 Receiver Frequency */
  /* SDR-IP 4.2.2 Receiver Frequency */
  /* NETSDR 4.2.3 Receiver Frequency */
  unsigned char freq[] = { 0x05, 0x20, 0x20, 0x00, 0x00 };

  apply_channel( freq, channel );

  std::vector< unsigned char > response;

  if ( ! transaction( freq, sizeof(freq), response ) )
    throw std::runtime_error("get_center_freq failed");

  uint32_t frequency = 0;
  frequency |= response[response.size()-5] <<  0;
  frequency |= response[response.size()-4] <<  8;
  frequency |= response[response.size()-3] << 16;
  frequency |= response[response.size()-2] << 24;

  return frequency;
}

std::string SoapyNetSDR::getDriverKey( void ) const
{
	return("netSDR");
}
std::vector<std::string> SoapyNetSDR::listGains( const int direction, const size_t channel ) const
{
	std::vector<std::string> options;
	options.push_back( "ATT" );						// RX: rf_gain
	return(options);
}
SoapySDR::RangeList SoapyNetSDR::getFrequencyRange( const int direction, const size_t channel, const std::string &name )
{

	return(getFrequencyRange(direction,channel));
}

SoapySDR::RangeList SoapyNetSDR::getFrequencyRange(const int direction, const size_t channel)
{
  /* query freq range(s) of the radio */

  SoapySDR::RangeList list;

fprintf(stderr,"getFrequencyRange in \n");

  /* SDR-IP 4.2.2 Receiver Frequency */
  /* NETSDR 4.2.3 Receiver Frequency */
  unsigned char frange[] = { 0x05, 0x40, 0x20, 0x00, 0x00 };

  apply_channel( frange, channel );

  std::vector< unsigned char > response;

  transaction( frange, sizeof(frange), response );

  if ( response.size() >= sizeof(frange) + 1 )
  {
    for ( size_t i = 0; i < response[sizeof(frange)]; i++ )
    {
      uint32_t min = *((uint32_t *)&response[sizeof(frange)+1+i*15]);
      uint32_t max = *((uint32_t *)&response[sizeof(frange)+1+5+i*15]);
      //uint32_t vco = *((uint32_t *)&response[sizeof(frange)+1+10+i*15]);

      //std::cerr << min << " " << max << " " << vco << std::endl;

          list.push_back(SoapySDR::Range( min, max ) );
    }
  }

fprintf(stderr,"getFrequencyRange out %zu\n",list.size());

	return(list);
}
size_t SoapyNetSDR::getNumChannels( const int dir ) const
{
	if(dir == SOAPY_SDR_RX)return(_nchan);
	
	return(0);
}

bool SoapyNetSDR::getFullDuplex( const int direction, const size_t channel ) const
{
	return(false);
}
std::string SoapyNetSDR::getHardwareKey( void ) const
{
	return("NetSDR");
}

SoapySDR::Kwargs SoapyNetSDR::getHardwareInfo( void ) const
{
	std::lock_guard<std::mutex> lock(_device_mutex);
	SoapySDR::Kwargs info;

	return(info);

}

std::vector<std::string> SoapyNetSDR::listAntennas( const int direction, const size_t channel ) const
{
	std::vector<std::string> options;
	options.push_back( "RX" );
	return(options);
}
std::vector<double> SoapyNetSDR::listSampleRates( const int direction, const size_t channel ) const
{
	std::vector<double> options;
		options.push_back( 20e3 );
		options.push_back( 32e3 );
		options.push_back( 40e3 );
		options.push_back( 50e3 );
		options.push_back( 80e3 );
		options.push_back( 100e3 );
		options.push_back( 125e3 );
		options.push_back( 160e3 );
		options.push_back( 200e3 );
		options.push_back( 250e3 );
		options.push_back( 500e3 );
		options.push_back( 625e3 );
		options.push_back( 800e3 );
		options.push_back( 1000e3 );
		options.push_back( 1250e3 );
		options.push_back( 2000e3 );
	return(options);
}

SoapySDR::Range SoapyNetSDR::getGainRange( const int direction, const size_t channel, const std::string &name ) const
{
	return(SoapySDR::Range( -30.0, 0 ) );
}

SoapySDR::Range SoapyNetSDR::getGainRange( const int direction, const size_t channel) const
{
	return(SoapySDR::Range( -30.0, 0 ) );
}
double SoapyNetSDR::getGain( const int direction, const size_t channel, const std::string &name )
{
	return(getGain(direction,channel));
}

double SoapyNetSDR::getGain( const int direction, const size_t channel)
{
	std::lock_guard<std::mutex> lock(_device_mutex);
  /* SDR-IQ 5.2.5 RF Gain */
  /* SDR-IP 4.2.3 RF Gain */
  /* NETSDR 4.2.6 RF Gain */
  unsigned char atten[] = { 0x05, 0x20, 0x38, 0x00, 0x00 };

  apply_channel( atten, channel );

  std::vector< unsigned char > response;

  if ( ! transaction( atten, sizeof(atten), response ) )
    throw std::runtime_error("get_gain failed");

  unsigned char code = response[response.size()-1];

  double gain = code;

  if( code & 0x80 )
    gain = (code & 0x7f) - 0x80;

  if ( RFSPACE_SDR_IQ == _radio )
    gain += 10;

  return gain;
}
double SoapyNetSDR::getSampleRate( const int direction, const size_t channel ) const
{
	std::lock_guard<std::mutex> lock(_device_mutex);


	return(_sample_rate);
}
std::vector<double> SoapyNetSDR::listBandwidths( const int direction, const size_t channel ) const
{
	std::vector<double> options;
	options.push_back( 34e6 );
	return(options);
}
std::vector<std::string> SoapyNetSDR::listFrequencies( const int direction, const size_t channel ) const
{
	std::vector<std::string> names;
	names.push_back( "RF" );
	return(names);
}



