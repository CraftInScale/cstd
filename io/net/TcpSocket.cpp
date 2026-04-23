#include "TcpSocket.h"

#include "../../error.h"

#include <fcntl.h>

int tcp_socket_setblocking(TcpSocket& socket, bool blocking)
{
    int fd = socket.fd.load();

    int flags = fcntl(fd, F_GETFL);
    flags &= ~O_NONBLOCK;

    if (!blocking)
        flags |= O_NONBLOCK;

    fcntl(fd, F_SETFL, flags);

    return 0;
}

int tcp_socket_close(TcpSocket& socket)
{
    int fd = socket.fd.load();

    if (fd > -1)
    {
        int error = shutdown(fd, SHUT_RDWR);
        close(fd);

        socket.destroyed_fd.store(fd);
        socket.fd.store(-1);

        if (error == -1 && errno != ENOTCONN)
            return E_ERRNO;
    }

    return E_OK;
}
