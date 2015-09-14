// echo_client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#endif

#include <stdio.h>
#include <nark/rpc/client.hpp>
#include <nark/inet/SocketStream.hpp>
#include <iostream>

using namespace std;
using namespace nark;
using namespace nark::rpc;

#include "../echo.h"

int main0(int argc, char* argv[])
try {
	auto_ptr<SocketStream> cs(ConnectSocket("127.0.0.1:8001"));
	rpc_client<PortableDataInput, PortableDataOutput> client(cs.get());
	EchoPtr ec = client.create("echo");
	while (!cin.eof())
	{
		string msg, y;
		cin >> msg;
		ec->echo(msg, &y);
		cout << "msg:" << msg << endl;
		cout << "y__:" << y << endl;
	}
	return 0;
}
catch (const std::exception& exp)
{
	printf("exception: what=%s\n", exp.what());
	return 1;
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

