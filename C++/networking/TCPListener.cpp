#include "TCPListener.h"

#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "Exceptions.h"
#include "TCPConnection.h"

#pragma comment (lib, "Ws2_32.lib")

using namespace std;

TCPListener::TCPListener( int port )
		: listenSock(INVALID_SOCKET)
{
	stringstream ps;
	ps << port;

	addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	if (getaddrinfo(nullptr, ps.str().c_str(), &hints, &ai) != 0)
		throw NetworkException("getaddrinfo failed.", __FUNCTION__);
}

TCPListener::~TCPListener()
{
	freeaddrinfo(ai);

	if (listenSock != INVALID_SOCKET)
		closesocket(listenSock);
}

void TCPListener::Start( int maxRequests /*= SOMAXCONN*/ )
{
	if (listenSock != INVALID_SOCKET)
		throw InvalidOperationException("The listener has already been started.", __FUNCTION__);

	listenSock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);

	if (listenSock == INVALID_SOCKET)
		throw NetworkException("Socket creation failed.", __FUNCTION__);

	if (bind(listenSock, ai->ai_addr, (int)ai->ai_addrlen) == SOCKET_ERROR)
	{
		closesocket(listenSock);
		throw NetworkException("A socket could not be bound to its given port.",
		                       __FUNCTION__);
	}

	if (listen(listenSock, maxRequests) == SOCKET_ERROR)
	{
		throw NetworkException("A socket failed on listening for connection requests.",
		                       __FUNCTION__);
	}
}

void TCPListener::Stop()
{
	if (listenSock == INVALID_SOCKET)
	{
		throw InvalidOperationException("The listener has not been started, so it cannot be stopped.",
		                                __FUNCTION__);
	}

	if (closesocket(listenSock) != 0)
		throw NetworkException("The listener failed while stopping itself", __FUNCTION__);
}

TCPConnection* TCPListener::Accept()
{
	if (listenSock == INVALID_SOCKET)
	{
		throw InvalidOperationException("The listener must be started before it can accept connections.",
		                                __FUNCTION__);
	}

	SOCKET connSock = accept(listenSock, NULL, NULL);
	if (connSock == INVALID_SOCKET)
	{
		throw NetworkException("The listener failed while accepting a connection.",
		                       __FUNCTION__);
	}

	return new TCPConnection(connSock);
}
