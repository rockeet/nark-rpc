// echo_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#ifdef _MSC_VER
	#pragma warning(disable: 4251) // needs to have dll-interface
	#pragma warning(disable: 4267) // conversion from 'size_t' to 'long', possible loss of data
	#pragma warning(disable: 4244) // conversion from '__w64 const unsigned int' to 'unsigned int', possible loss of data
	#pragma comment(lib, "Ws2_32.lib")
#endif

#include <stdio.h>
#include <nark/rpc/server.hpp>
#include <nark/io/SocketStream.hpp>
#include <iostream>

using namespace std;
using namespace nark;
using namespace nark::rpc;

#include "../echo.h"

rpc_ret_t Echo::echo(const std::string& msg, std::string* y)
{
	cout << "input: " << msg << endl;
	*y = "server: " + msg;
	return 0;
}

int main0(int argc, char* argv[])
{
	try {
		SocketAcceptor acceptor("0.0.0.0:8001");
		rpc_server<PortableDataInput, PortableDataOutput> server(&acceptor);

		// 加入命名的 Servant
		server.add_servant(
			new Echo,
			"echo",
			0 // 0 will not auto create GlobaleScope Object
			);

		server.start();
	}
	catch (const std::exception& exp)
	{
		printf("exception: what=%s\n", exp.what());
	}
	return 0;
}

int main(int argc, char* argv[])
{
#if defined(_WIN32) || defined(_WIN64)
	WSADATA information;
	WSAStartup(MAKEWORD(2, 2), &information);
	int ret = main0(argc, argv);
	WSACleanup();
	return ret;
#else
	return main0(argc, argv);
#endif
}

