#pragma once

#include "TcpSocket.h"

struct TcpListener
{
	TcpSocket socket;
};

int create_tcp_listener(TcpListener& listener, NetAddress& addr);
int destroy_tcp_listener(TcpListener& listener);

int tcp_listener_accept(TcpListener& listener, TcpSocket& client);