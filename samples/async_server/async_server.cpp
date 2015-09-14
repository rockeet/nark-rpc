// test_rpc_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#ifdef _MSC_VER
#pragma warning(disable: 4251) // needs to have dll-interface
#pragma warning(disable: 4267) // conversion from 'size_t' to 'long', possible loss of data
#pragma warning(disable: 4244) // conversion from '__w64 const unsigned int' to 'unsigned int', possible loss of data
#pragma comment(lib, "Ws2_32.lib")
#endif

#include <nark/rpc/server.hpp>
#include <nark/inet/SocketStream.hpp>
#include <iostream>

using namespace nark;
using namespace nark::rpc;

#include "../test.h"

// use macro for convenient
BEGIN_RPC_IMP_INTERFACE(SampleRPC_Imp1, AsyncInterface)
	rpc_ret_t get_val(rpc_in<int> x)
	{
		std::cout << "AsyncInterface::get_val(rpc_in<int> x=" << x.r << ")\n";
		return x.r;
	}
	rpc_ret_t get_len(const std::string& x)
	{
		std::cout << "AsyncInterface::get_len(const std::string& x=\"" << x << "\")\n";
		return x.size();
	}
	rpc_ret_t squareVec(vint_vec& x)
	{
		for (vint_vec::iterator i = x.begin(); i != x.end(); ++i)
		{
			*i *= *i;
		}
		return x.size();
	}
	rpc_ret_t multiVec(vint_vec& z, vint_vec& x, vint_vec& y)
	{
		z.clear();
		for (size_t i = 0; i != x.size(); ++i)
		{
			z.push_back(x[i] * y[i]);
		}
		return 0x12345678;
	}
END_RPC_IMP_INTERFACE()

// don't use macro for more control
class SampleRPC_Imp2 : public SampleRPC_Interface2
{
	rpc_ret_t get_val(rpc_in<int> x)
	{
		std::cout << BOOST_CURRENT_FUNCTION << "x=" <<x.r << "\n";
		return x.r;
	}
	rpc_ret_t get_len(const std::string& x)
	{
		std::cout << BOOST_CURRENT_FUNCTION << "x=" << x << "\n";
		return x.size();
	}
public:
	// if use macro, such as SampleRPC_Imp1, app can not use custom create
	static remote_object* create()
	{
		SampleRPC_Imp2* p = new SampleRPC_Imp2;
		// set p
		//
		return p;
	}
};

int main0(int argc, char* argv[])
{
	try {
		SocketAcceptor acceptor("0.0.0.0:8001");
		rpc_server<PortableDataInput, PortableDataOutput> server(&acceptor);

		// register rpc implementation class...
		RPC_SERVER_AUTO_CREATE(server, SampleRPC_Imp1);
		server.auto_create((SampleRPC_Imp2*)0, &SampleRPC_Imp2::create);

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

