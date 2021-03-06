#ifndef __SOCKET_UTILS_H
#define __SOCKET_UTILS_H

#ifdef _WIN32
#include <Winsock2.h>
#include <io.h>
#include <Ws2def.h> 
#include <ws2ipdef.h>
#include <WS2tcpip.h> //inet_pton
#else
#include <sys/socket.h>
#include <netinet/in.h>  // IPPROTO_TCP
#include <netinet/tcp.h> // TCP_NODELAY
#include <arpa/inet.h>
#include <errno.h>
#endif

#include <fcntl.h>

#ifdef _WIN32
#define SOCKET_FD(handle) _open_osfhandle(handle, 0)
#define SOCKET_HANDLE(fd) _get_osfhandle(fd)
#define CLOSE_SOCKET(fd) closesocket(SOCKET_HANDLE(fd))
#define ERRNO WSAGetLastError()
typedef long long ssize_t;
#else
#define SOCKET_FD(fd) fd
#define SOCKET_HANDLE(fd) fd
#define CLOSE_SOCKET(fd) close(fd)
#define ERRNO errno
#endif

namespace netcore
{
	namespace socketutils
	{		
		void setNonBlocking(int fd);
		int createSocket();
		int createNonBlockingSocket();
	}
}

#endif