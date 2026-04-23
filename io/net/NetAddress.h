#pragma once

#include "../../string/String.h"

#include <stdint.h>
#include <netinet/in.h>

enum AddressVersion : uint8_t
{
	ADDR_V4 = 4,
	ADDR_V6 = 16
};

struct IpAddr
{
	uint8_t bytes[16];
	AddressVersion version;
};

struct NetAddress
{
	IpAddr ip;
	uint16_t port;
};

void init_ipaddr(IpAddr& ip, sockaddr* in, socklen_t in_size);
void init_ipaddr_v4(IpAddr& ip, uint8_t* bytes);
void init_ipaddr_v6(IpAddr& ip, uint8_t* bytes);

void init_netaddr(NetAddress& address, sockaddr* in, socklen_t in_size);
int init_netaddr(NetAddress& address, String& ip, uint16_t port);
void init_netaddr(NetAddress& address, IpAddr& ip, uint16_t port);

int netaddr_to_string(NetAddress& address, String& append_to);