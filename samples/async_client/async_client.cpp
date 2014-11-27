#include "stdafx.h"
#include <iostream>
#include <nark/rpc/client.hpp>
#include <nark/io/SocketStream.hpp>

using namespace std;
using namespace nark;
using namespace nark::rpc;

#include "../test.h"
#pragma comment(lib, "Ws2_32.lib")

void printVec(const vint_vec& vec)
{
	for (int i = 0; i != vec.size(); ++i)
	{
		cout << "vec[" << i << "]=" << vec[i] << "\n";
	}
}

class AsyncImpl : public AsyncInterface
{
public:
	AsyncImpl()
	{
		this->multiVec.set_async_callback(&AsyncImpl::on_multiVec);
	}
private:
	void on_multiVec(const client_packet_base& packet, vint_vec& z, vint_vec& x, vint_vec& y)
	{
		printf("AsyncImpl::on_multiVec\n");
		printf("ret=%u, z=%u, x=%u, y=%u\n", packet.retv, z.size(), x.size(), y.size());
	}
};
RPC_TYPEDEF_PTR(AsyncImpl);

int main0(int argc, char* argv[])
try {
	auto_ptr<SocketStream> cs(ConnectSocket("127.0.0.1:8001"));
	rpc_client<PortableDataInput, PortableDataOutput> client(cs.get());

	AsyncImplPtr obj1;
	SampleRPC_Interface2Ptr obj2;

	client.create(obj1, "obj1");
	client.create(obj2, "obj2");

	int ret;
	ret = client.retrieve(obj2, "obj2");

	rpc_ret_t val = obj1->get_val(100);
	cout << "obj1->get_val(100)=" << val << "\n";

	val = obj1->get_len("hello, world!");
	cout << "obj1->get_len(\"hello, world!\")=" << val << "\n";

	std::vector<unsigned> vec;
	vec.push_back((1));
	vec.push_back((2));
	vec.push_back((3));
	vec.push_back((4));
	vec.push_back((11));
	vec.push_back((22));
	vec.push_back((33));
	vec.push_back((44));

	val = obj1->squareVec(vec);
	printVec(vec);
	val = obj1->squareVec(vec);
	printVec(vec);

	std::vector<unsigned> vec2;
	for (int i = 0; i != vec.size(); ++i)
	{
		vec2.push_back(i + 1);
	}
//	obj1->multiVec.on_return = &AsyncInterface::on_multiVec;
	for (int i = 0; i < 5; ++i)
	{
		std::vector<unsigned> vec3;
		obj1->multiVec.async(vec3, vec, vec2);
		cout << "obj1->multiVec(vec3, vec, vec2) = " << val << "\n";
		printVec(vec3);
	}
	client.wait_pending_async();
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

