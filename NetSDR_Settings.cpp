#include "SoapyNetSDR.hpp"

SoapyNetSDR::SoapyNetSDR(const SoapySDR::Kwargs &args)
{
 	const SoapySDR::Kwargs options;
	if(args.size()){ 
		printf("driver args %lu ",args.count("driver"));
		if(args.count("driver"))printf(" %s \n",args.at("driver").c_str());
		printf("label args %lu ",args.count("label"));
		if(args.count("label"))printf(" %s \n",args.at("label").c_str());
		printf("netsdr args %lu ",args.count("netsdr"));
		if(args.count("netsdr"))printf(" %s \n",args.at("netsdr").c_str());
	}
	//printf("I am here! args %s\n",args["driver"].c_str());
	//printf("I am here! args %s\n",args["label"].c_str());
	//printf("I am here! args %s\n",args["netsdr"].c_str());
   
}

SoapyNetSDR::~SoapyNetSDR(void)
{
	printf("I am Dead!\n");
}
void SoapyNetSDR::setAntenna( const int direction, const size_t channel, const std::string &name )
{
	;
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
void SoapyNetSDR::setBandwidth( const int direction, const size_t channel, const double bw )
{
	std::lock_guard<std::mutex> lock(_device_mutex);
}
void SoapyNetSDR::setFrequency( const int direction, const size_t channel, const std::string &name, const double frequency, const SoapySDR::Kwargs &args )
{
	std::lock_guard<std::mutex> lock(_device_mutex);
}
void SoapyNetSDR::setFrequency(const int direction, const size_t channel, const double frequency, const SoapySDR::Kwargs &args)
{
	std::lock_guard<std::mutex> lock(_device_mutex);
}
void SoapyNetSDR::setSampleRate( const int direction, const size_t channel, const double rate )
{
	std::lock_guard<std::mutex> lock(_device_mutex);
}
void SoapyNetSDR::setGain( const int direction, const size_t channel, const double value )
{
	std::lock_guard<std::mutex> lock(_device_mutex);
}
void SoapyNetSDR::setGain( const int direction, const size_t channel, const std::string &name, const double value )
{
	std::lock_guard<std::mutex> lock(_device_mutex);
}

std::string SoapyNetSDR::getAntenna( const int direction, const size_t channel ) const
{
	return("RX");
}

double SoapyNetSDR::getBandwidth( const int direction, const size_t channel ) const
{
	std::lock_guard<std::mutex> lock(_device_mutex);
	double bw(0.0);

	return (bw);
}

double SoapyNetSDR::getFrequency( const int direction, const size_t channel, const std::string &name ) const
{
	std::lock_guard<std::mutex> lock(_device_mutex);
	double freq(0.0);
	return(freq);
}
double SoapyNetSDR::getFrequency(const int direction, const size_t channel) const
{
	std::lock_guard<std::mutex> lock(_device_mutex);
	double freq(0.0);
	return(freq);
}

std::string SoapyNetSDR::getDriverKey( void ) const
{

	return("netSDR");
}
std::vector<std::string> SoapyNetSDR::listGains( const int direction, const size_t channel ) const
{
	std::vector<std::string> options;
		options.push_back( "LNA" );						// RX: if_gain
		options.push_back( "AMP" );						// RX: rf_gain
		options.push_back( "VGA" );						// RX: bb_gain

	return(options);
}
SoapySDR::RangeList SoapyNetSDR::getFrequencyRange( const int direction, const size_t channel, const std::string &name ) const
{
	return(SoapySDR::RangeList( 1, SoapySDR::Range( 0, 7250000000ull ) ) );
}

SoapySDR::RangeList SoapyNetSDR::getFrequencyRange(const int direction, const size_t channel) const
{
	return(SoapySDR::RangeList( 1, SoapySDR::Range( 0, 7250000000ull ) ) );
}

size_t SoapyNetSDR::getNumChannels( const int dir ) const
{
	return(1);
}


bool SoapyNetSDR::getFullDuplex( const int direction, const size_t channel ) const
{
	return(false);
}
std::string SoapyNetSDR::getHardwareKey( void ) const
{
	std::lock_guard<std::mutex> lock(_device_mutex);
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
	for ( double r = 1e6; r <= 20e6; r += 1e6 )
	{
		options.push_back( r );
	}
	return(options);
}

SoapySDR::Range SoapyNetSDR::getGainRange( const int direction, const size_t channel, const std::string &name ) const
{
	return(SoapySDR::Range( 0, 0 ) );
}

SoapySDR::Range SoapyNetSDR::getGainRange( const int direction, const size_t channel) const
{
	return(SoapySDR::Range( 0, 0 ) );
}
double SoapyNetSDR::getGain( const int direction, const size_t channel, const std::string &name ) const
{
	std::lock_guard<std::mutex> lock(_device_mutex);
	double gain = 0.0;
	return(gain);
}

double SoapyNetSDR::getGain( const int direction, const size_t channel) const
{
	std::lock_guard<std::mutex> lock(_device_mutex);
	double gain = 0.0;
	return(gain);
}
double SoapyNetSDR::getSampleRate( const int direction, const size_t channel ) const
{
	std::lock_guard<std::mutex> lock(_device_mutex);
	double samp(0.0);
	return(samp);
}
std::vector<double> SoapyNetSDR::listBandwidths( const int direction, const size_t channel ) const
{
	std::vector<double> options;
	options.push_back( 1750000 );
	options.push_back( 2500000 );
	options.push_back( 3500000 );
	options.push_back( 5000000 );
	options.push_back( 5500000 );
	options.push_back( 6000000 );
	options.push_back( 7000000 );
	options.push_back( 8000000 );
	options.push_back( 9000000 );
	options.push_back( 10000000 );
	options.push_back( 12000000 );
	options.push_back( 14000000 );
	options.push_back( 15000000 );
	options.push_back( 20000000 );
	options.push_back( 24000000 );
	options.push_back( 28000000 );
	return(options);
}
std::vector<std::string> SoapyNetSDR::listFrequencies( const int direction, const size_t channel ) const
{
	std::vector<std::string> names;
	names.push_back( "RF" );
	return(names);
}



