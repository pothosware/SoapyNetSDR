
#include "SoapyNetSDR.hpp"

std::vector<interfaceInformation> interfaceList()
{
	std::vector<interfaceInformation> list;
	interfaceInformation c1;

	struct ifaddrs *addrs,*iloop;
	char buf[INET_ADDRSTRLEN],buf2[INET_ADDRSTRLEN];
	struct sockaddr_in *s4;

	getifaddrs(&addrs);
	for (iloop = addrs; iloop != NULL; iloop = iloop->ifa_next)
	{
		if (iloop->ifa_addr == NULL)
			continue;

		if (iloop->ifa_addr->sa_family != AF_INET) continue; //just IPv4

		s4 = (struct sockaddr_in *)(iloop->ifa_addr);
		buf[0]=0;
		if(s4){
			inet_ntop(iloop->ifa_addr->sa_family, (void *)&(s4->sin_addr), buf, sizeof(buf));
		}else{
			continue;
		}

		s4 = (struct sockaddr_in *)(iloop->ifa_dstaddr);
		buf2[0]=0;
		if(s4){
			inet_ntop(iloop->ifa_dstaddr->sa_family, (void *)&(s4->sin_addr), buf2, sizeof(buf2));
		}else{
			continue;
		}

		if(!(iloop->ifa_flags & IFF_UP) || !(iloop->ifa_flags & IFF_BROADCAST))continue;

		c1.name = iloop->ifa_name;

		c1.address = buf;

		c1.broadcast = buf2;

		list.push_back(c1);

	}

	freeifaddrs(addrs);
	return list;
}
