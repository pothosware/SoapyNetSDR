#include "SoapyNetSDR.hpp"
#include <SoapySDR/Registry.hpp>
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <cerrno>
#include <vector>

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#else
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

/* discovery protocol internals taken from CuteSDR project */
PACK(typedef struct
{
	/* 56 fixed common byte fields */
	unsigned char length[2]; 	/* length of total message in bytes (little endian byte order) */
	unsigned char key[2];		/* fixed key key[0]==0x5A  key[1]==0xA5 */
	unsigned char op;			/* 0 == Tx_msg(to device), 1 == Rx_msg(from device), 2 == Set(to device) */
	char name[16];				/* Device name string null terminated */
	char sn[16];				/* Serial number string null terminated */
	unsigned char ipaddr[16];	/* device IP address (little endian byte order) */
	unsigned char port[2];		/* device Port number (little endian byte order) */
	unsigned char customfield;	/* Specifies a custom data field for a particular device */
}) discover_common_msg_t;

/* UDP port numbers for discovery protocol */
#define DISCOVER_SERVER_PORT 48321	/* PC client Tx port, SDR Server Rx Port */
#define DISCOVER_CLIENT_PORT 48322	/* PC client Rx port, SDR Server Tx Port */

#define DISCOVER_TIMEOUT_MS 100

#define KEY0      0x5A
#define KEY1      0xA5
#define MSG_REQ   0
#define MSG_RESP  1
#define MSG_SET   2

typedef struct
{
	std::string name;
	std::string sn;
	std::string addr;
	uint16_t port;
} unit_t;

static std::vector < unit_t > discover_netsdr();

/***********************************************************************
 * Find available devices
 **********************************************************************/
static SoapySDR::KwargsList find_netSDR(const SoapySDR::Kwargs &args)
{
	SoapyNetSDR_SocketInit socket_init;

	SoapySDR::KwargsList results;

	//locate the device on the system...
	//return a list of 0, 1, or more argument maps that each identify a device

	SoapySDR::Kwargs devInfo;
	for (const auto &unit : discover_netsdr()){

		//filter by serial when provided
		if (args.count("serial") != 0 and args.at("serial") != unit.sn) continue;

		devInfo["name"] = unit.name;
		devInfo["serial"] = unit.sn;
		devInfo["label"] = "RFspace NetSDR SN " + unit.sn;
		devInfo["netsdr"] = unit.addr + ":" + std::to_string(unit.port);

		//filter out duplicates in the discovery
		int push=1;
		for(unsigned long n=0;n<results.size();++n){
			 if(results[n] == devInfo){
				push=0;
				break;
			}
		}
		if(push)results.push_back(devInfo);

	}


	return results;
}
static std::vector < unit_t > discover_netsdr()
{
	std::vector < unit_t > units;

	std::vector<interfaceInformation> list=interfaceList();

	/* fill in the hosts's address and data */
	struct sockaddr_in host_sa; /* local address */
	memset(&host_sa, 0, sizeof(host_sa));
	host_sa.sin_family = AF_INET;
	host_sa.sin_addr.s_addr = htonl(INADDR_ANY);
	host_sa.sin_port = htons(DISCOVER_CLIENT_PORT);

	SOCKET sockRx;
	if ( (sockRx = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){
		return units;
	}

	if ( bind(sockRx, (struct sockaddr *)&host_sa, sizeof(host_sa)) < 0 )
	{
		perror("binding datagram sock2");
		printf("errno %d DISCOVER_SERVER_PORT %d\n",SOCKET_ERRNO,DISCOVER_SERVER_PORT);
		closesocket(sockRx);
		return units;
	}

	#ifdef _MSC_VER
	DWORD tv = DISCOVER_TIMEOUT_MS; //windows uses DWORD timeout in ms
	#else
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = DISCOVER_TIMEOUT_MS*1000;
	#endif
	if ( setsockopt(sockRx, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv)) < 0 )
	{
		closesocket(sockRx);
		return units;
	}

	for(int pass=0;pass<2;++pass){
		for(size_t n=0;n<list.size();++n){
			if(pass == 1){
				list[n].broadcast="255.255.255.255";
			}
			//std::cout << "name " << list[n].name << std::endl;
			//std::cout << "address " << list[n].address << std::endl;
			//std::cout << "broadcast " << list[n].broadcast << std::endl;

			SOCKET sockTx;


			if ( (sockTx = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
			continue;

			int sockoptval = 1;
			setsockopt(sockTx, SOL_SOCKET, SO_REUSEADDR, (const char *)&sockoptval, sizeof(int));
			sockoptval = 1;
			setsockopt(sockTx, SOL_SOCKET, SO_BROADCAST, (const char *)&sockoptval, sizeof(int));


			struct sockaddr_in peer_sa; /* remote address */
			struct sockaddr_in peer_sa2; /* remote address */

			/* fill in the server's address and data */
			memset((char*)&peer_sa, 0, sizeof(peer_sa));
			peer_sa.sin_family = AF_INET;
			inet_pton(AF_INET, list[n].address.c_str(), &peer_sa.sin_addr);
			peer_sa.sin_port = htons(DISCOVER_SERVER_PORT);


			/* fill in the server's address and data */
			memset((char*)&peer_sa2, 0, sizeof(peer_sa2));
			peer_sa2.sin_family = AF_INET;
			inet_pton(AF_INET, list[n].broadcast.c_str(), &peer_sa2.sin_addr);
			peer_sa2.sin_port = htons(DISCOVER_SERVER_PORT);

			if ( bind(sockTx, (struct sockaddr *)&peer_sa, sizeof(peer_sa)) < 0 )
			{
				closesocket(sockTx);
				continue;
			}

			discover_common_msg_t tx_msg;
			memset( (void *)&tx_msg, 0, sizeof(discover_common_msg_t) );

			tx_msg.length[0] = sizeof(discover_common_msg_t);
			tx_msg.length[1] = sizeof(discover_common_msg_t) >> 8;
			tx_msg.key[0] = KEY0;
			tx_msg.key[1] = KEY1;
			tx_msg.op = MSG_REQ;

			sendto(sockTx, (const char *)&tx_msg, sizeof(tx_msg), 0, (struct sockaddr *)&peer_sa, sizeof(peer_sa));
			sendto(sockTx, (const char *)&tx_msg, sizeof(tx_msg), 0, (struct sockaddr *)&peer_sa2, sizeof(peer_sa2));

			closesocket(sockTx);
		}
	}

	while ( 1 )
	{
		size_t rx_bytes = 0;
		unsigned char data[1024*2];

		socklen_t addrlen = sizeof(host_sa); // length of addresses

		int nbytes = recvfrom(sockRx, (char *)data, sizeof(data), 0, (struct sockaddr *)&host_sa, &addrlen);

		if ( nbytes <= 0 )
			break;
		rx_bytes = nbytes;


		if ( rx_bytes >= sizeof(discover_common_msg_t) )
		{
			discover_common_msg_t *rx_msg = (discover_common_msg_t *)data;

			if ( KEY0 == rx_msg->key[0] && KEY1 == rx_msg->key[1] &&
					MSG_RESP == rx_msg->op )
			{

				void *temp = rx_msg->port;
				uint16_t port = *((uint16_t *)temp);

				std::stringstream buffer;

				buffer << int(rx_msg->ipaddr[3]) << "."
						<< int(rx_msg->ipaddr[2]) << "."
						<< int(rx_msg->ipaddr[1]) << "."
						<< int(rx_msg->ipaddr[0]);


				std::string addr=buffer.str();

				//std::cout << "addr " << addr << std::endl;

				unit_t unit;

				unit.name = rx_msg->name;
				unit.sn = rx_msg->sn;
				unit.addr = addr;
				unit.port = port;

				units.push_back( unit );

			}
		}

	}
	closesocket(sockRx);

	return units;
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
