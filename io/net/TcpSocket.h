#pragma once

#include "NetAddress.h"

#include <atomic>

struct TcpSocket
{
	std::atomic<int> fd;
	std::atomic<int> destroyed_fd;
	NetAddress addr;
};

int tcp_socket_setblocking(TcpSocket& socket, bool blocking);
int tcp_socket_close(TcpSocket& socket);