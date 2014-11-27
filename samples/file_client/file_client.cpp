#include "stdafx.h"

#ifdef _MSC_VER
	#pragma warning(disable: 4251) // needs to have dll-interface
	#pragma warning(disable: 4267) // conversion from 'size_t' to 'long', possible loss of data
	#pragma warning(disable: 4244) // conversion from '__w64 const unsigned int' to 'unsigned int', possible loss of data
	#pragma comment(lib, "Ws2_32.lib")
#endif

#include <stdio.h>
#include <nark/rpc/client.hpp>
#include <nark/io/SocketStream.hpp>

using namespace std;
using namespace nark;
using namespace nark::rpc;

#include "../ifile.h"

int main0(int argc, char* argv[])
try {
	auto_ptr<SocketStream> cs(ConnectSocket("127.0.0.1:8001"));
	rpc_client<PortableDataInput, PortableDataOutput> client(cs.get());
	FileObjPtr file;
	client.create(file);
	string fname = "test.txt";
	vector<char> buffer;
	int ret = file->open(fname, "w+");
	if (0 == ret)
	{
		printf("open file for write successed\n");
		string txt = "Hello, world, this is a test file\n";
		buffer.resize(txt.size());
		std::copy(txt.begin(), txt.end(), buffer.begin());
		uint32_t nWritten;
		file->write(buffer, &nWritten);
		printf("write=%d, written=%d\n", buffer.size(), nWritten);
		file->close();
	}
	ret = file->open("test.txt", "r");
	if (0 == ret)
	{
		printf("open file for read successed\n");
		uint32_t nRead = buffer.size();
		file->read(&buffer, nRead);
		printf("read=%d, readed=%d\n", nRead, buffer.size());
		string txt;
		txt.resize(buffer.size());
		std::copy(buffer.begin(), buffer.end(), txt.begin());
		printf("readed text=%s\n", txt.c_str());
	}
	else
	{
		printf("open file for read failed\n");
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

