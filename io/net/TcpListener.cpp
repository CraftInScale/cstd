#include "TcpListener.h"

#include "../../error.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>

int create_tcp_listener(TcpListener& listener, NetAddress& addr)
{
	listener.socket.addr = addr;
	int fd;

	switch (addr.ip.version)
	{
	case ADDR_V4:
	{
		fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (fd == -1)
			return E_ERRNO;

		sockaddr_in in4;
		bzero(&in4, sizeof(in4));
		memcpy(&in4.sin_addr, addr.ip.bytes, 4);
		in4.sin_family = AF_INET;
		in4.sin_port = htons(addr.port);

		int ret = bind(fd, (sockaddr*)&in4, sizeof(sockaddr_in));
		if (ret == -1)
		{
			close(fd);
			return E_ERRNO;
		}
	}
		break;

	case ADDR_V6:
	{
		fd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

		sockaddr_in6 in6;
		bzero(&in6, sizeof(in6));
		memcpy(&in6.sin6_addr, addr.ip.bytes, 16);
		in6.sin6_family = AF_INET;
		in6.sin6_port = htons(addr.port);

		int ret = bind(fd, (sockaddr*)&in6, sizeof(sockaddr_in6));
		if (ret == -1)
		{
			close(fd);
			return E_ERRNO;
		}
	}
		break;
	}

	int ret = listen(fd, SOMAXCONN);
	if (ret == -1)
	{
		close(fd);
		return E_ERRNO;
	}

	listener.socket.fd.store(fd);

	return E_OK;
}

int destroy_tcp_listener(TcpListener& listener)
{
	return tcp_socket_close(listener.socket);
}

int tcp_listener_accept(TcpListener& listener, TcpSocket& client)
{
	//accept(listener.socket.fd, )

	return E_OK;
}
