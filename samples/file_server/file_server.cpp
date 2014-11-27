// test_rpc_server.cpp : Defines the entry point for the console application.
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

#include "../ifile.h"

BEGIN_RPC_IMP_INTERFACE(FileObjImp, FileObj)
FileObjImp()
{
	fp = 0;
}

~FileObjImp()
{
	if (fp)
	{
		fclose(fp);
	}
}

rpc_ret_t open(const std::string& fname, const std::string& mode)
{
	fp = fopen(fname.c_str(), mode.c_str());
	if (0 == fp)
		return errno;
	return 0;
}

rpc_ret_t read(vector<char>* buffer, uint32_t length)
{
	buffer->resize(length);
	size_t nRead = fread(&*buffer->begin(), 1, length, fp);
	buffer->resize(nRead);
	return 0;
}

rpc_ret_t write(const vector<char>& buffer, uint32_t* length)
{
	*length = fwrite(&*buffer.begin(), 1, buffer.size(), fp);
	return 0;
}

rpc_ret_t close()
{
	fclose(fp);
	fp = 0;
	return 0;
}

private:
	FILE* fp;
END_RPC_IMP_INTERFACE()

int main0(int argc, char* argv[])
{
	try {
		SocketAcceptor acceptor("0.0.0.0:8001");
		rpc_server<PortableDataInput, PortableDataOutput> server(&acceptor);

		// FileObjImp will auto created by client call
		RPC_SERVER_AUTO_CREATE(server, FileObjImp);

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

