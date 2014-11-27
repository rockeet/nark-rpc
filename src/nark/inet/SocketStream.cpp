/* vim: set tabstop=4 : */
#include "SocketStream.hpp"

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
//加入winsock库
typedef int socklen_t;
#pragma comment(lib, "Ws2_32.lib")
#else
#	include <sys/stat.h>
#	include <fcntl.h>
#	include <errno.h>
#	include <netdb.h>
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <unistd.h>
#	define closesocket close
#endif

#include <stdlib.h>
#include <string.h>
#include <string>
#include <nark/num_to_str.hpp>

namespace nark {

SocketException::SocketException(const char* szMsg)
  : IOException(lastError(), szMsg)
{
}

SocketException::SocketException(int errCode, const char* szMsg)
  : IOException(errCode, szMsg)
{
}

int SocketException::lastError()
{
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
	return WSAGetLastError();
#else
	return errno;
#endif
}

//////////////////////////////////////////////////////////////////////////

SocketStream::SocketStream(SOCKET socket, bool bAutoClose)
{
	this->socket = socket;
	posg = 0;
	posp = 0;
	m_bAutoClose = bAutoClose;
	m_bEof = false;
}

SocketStream::~SocketStream(void)
{
	if (m_bAutoClose)
		::closesocket(socket);
}

bool SocketStream::waitfor_again()
{
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
#else
#endif
	return false;
}

size_t SocketStream::read(void* data, size_t length)
{
	if (m_bEof)
	{
		throw EndOfFileException("socket graceful closed");
	}

	int n = ::recv(socket, (char*)data, length, 0);
	if (0 == n)
	{
		m_bEof = true;
		throw EndOfFileException("socket graceful closed");
	}
	if (-1 == n && !waitfor_again())
	{
		string_appender<> oss;
		oss << "recv packet error, socket=" << socket
			<< ", want=" << length << ", received=" << n;
			;
		throw SocketException(oss.str().c_str());
	}
	posg += n;
	return  n;
}

size_t SocketStream::write(const void* data, size_t length)
{
	int  n = ::send(socket, (const char*)data, (int)length, 0);
	if (0 == n || (-1 == n && !waitfor_again()))
	{
		string_appender<> oss;
		oss << "send packet error, socket=" << socket
			<< ", want=" << length << ", sent=" << n;
			;
		throw SocketException(oss.str().c_str());
	}
	posp += n;
	return  n;
}
//////////////////////////////////////////////////////////////////////////

SocketAcceptor::SocketAcceptor(const char* szBindAddr)
{
	const char* colon = strchr(szBindAddr, ':');
	std::string strAddr(szBindAddr, colon);
	u_short port = atoi(colon + 1);

	m_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (-1 == m_socket)
	{
		perror("error when create socket");
		std::string err = "SocketAcceptor::SocketAcceptor(";
		err += szBindAddr;
		err += ")";
		throw SocketException(err.c_str());
	}
	struct sockaddr_in addr; memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(strAddr.c_str());
	addr.sin_port = htons(port);

	int err = ::bind(m_socket, (struct sockaddr*)(&addr), sizeof(addr));
	if (0 == err) {
		printf("bind send to listen socket[%d] at port:%d success!\n", m_socket, port);
	} else {
		std::string errText = IOException::errorText(SocketException::lastError()).c_str();
		fprintf(stderr, "SocketConnection::listen, bind tcp socket error: %s", errText.c_str());
		::closesocket(m_socket);
		m_socket = -1;
		throw SocketException(errText.c_str());
	}
	err = ::listen(m_socket, 5);
	if (0 == err) {
		printf("listen success\n");
	} else {
		std::string errText = IOException::errorText(SocketException::lastError()).c_str();
		fprintf(stderr, "listen tcp error: %s", errText.c_str());
		::closesocket(m_socket);
		m_socket = -1;
		throw SocketException(errText.c_str());
	}
}

SocketStream* SocketAcceptor::accept()
{
	struct sockaddr_in from;
	socklen_t size = sizeof(from);
	SOCKET client = ::accept(m_socket, (struct sockaddr*)&from, &size);
	if (-1 == client)
	{
		std::string errText = IOException::errorText(SocketException::lastError()).c_str();
		fprintf(stderr, "error when accept:%s", errText.c_str());
		return 0;
	}
	SocketStream* stream = new SocketStream(client);
	return stream;
}

SocketStream* ConnectSocket(const char* szServerAddr)
{
	SOCKET hSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (-1 == hSocket)
		return 0;
	const char* colon = strchr(szServerAddr, ':');
	std::string strAddr(szServerAddr, colon);
	u_short port = atoi(colon + 1);

	struct sockaddr_in addr; memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(strAddr.c_str());
	addr.sin_port = htons(port);
	int err = ::connect(hSocket, (sockaddr*)&addr, sizeof(addr));
	if (0 == err) {
		DEBUG_printf("connect %s success\n", szServerAddr);
	} else {
		std::string errText = IOException::errorText(SocketException::lastError()).c_str();
		fprintf(stderr, "error when connect: %s", errText.c_str());
		::closesocket(hSocket);
		hSocket = -1;
		throw SocketException(errText.c_str());
	}
	SocketStream* stream = new SocketStream(hSocket);
	return stream;
}


} // nark
