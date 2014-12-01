// employee_client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#endif

#include <nark/rpc/client.hpp>
#include "../employee.h"

int main0(int argc, char* argv[])
try {
	auto_ptr<SocketStream> cs(ConnectSocket("127.0.0.1:8001"));
	rpc_client<PortableDataInput, PortableDataOutput> client(cs.get());

	DbEmployeePtr dbem;
	client.create(dbem, "DbEmployeeObject");

	Employee em;
	vector<Employee> ems;
	rpc_ret_t ret;
	ret = dbem->fillData();
	if (0 == ret)
	{
		cout << "fillData success\n";
	}
	for (;;)
	{
		cout << "getByID:\n";
		ret = dbem->getByID(1, em);
		if (0 == ret)
		{
			em.print(cout);
		}
		cout << "getByName:\n";
		ret = dbem->getByName("leipeng", ems);
		if (0 == ret)
		{
			for (vector<Employee>::const_iterator i = ems.begin(); i != ems.end(); ++i)
				i->print(cout);
		}
		cout << "getByDepartment:\n";
		ret = dbem->getByDepartment("tech", ems);
		if (0 == ret)
		{
			for (vector<Employee>::const_iterator i = ems.begin(); i != ems.end(); ++i)
				i->print(cout);
		}
		else
		{
			cout << "failed, ret=" << ret << endl;
		}
		if (getchar() == EOF)
			break;
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

