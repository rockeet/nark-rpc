// employee_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#pragma comment(lib, "Ws2_32.lib")

#include <nark/rpc/server.hpp>
#include <nark/io/SocketStream.hpp>

#include "../employee.h"

// not need auto create
class DbEmployeeImp : public DbEmployee
{
private:
	DbEnv m_env;
	auto_ptr<dbmap<unsigned, Employee> > m_byID;
	auto_ptr<kmapdset<dbt_string, Employee> > m_byName, m_byDepartment;

public:
	DbEmployeeImp()	: m_env(0)
	{
		u_int32_t envFlags = 0
			|DB_CREATE
			|DB_THREAD
			|DB_INIT_MPOOL
			;
	//	printf("open env\n");
		m_env.open("db", envFlags, 0);
	//	printf("open env success\n");
		m_byID.reset(new dbmap<unsigned, Employee>(&m_env, "byid"));
		m_byName.reset(new kmapdset<dbt_string, Employee>(&m_env, "byname"));
		m_byDepartment.reset(new kmapdset<dbt_string, Employee>(&m_env, "bydepartment"));
	}

	rpc_ret_t fillData()
	{
		Employee e;
		e.id = 1;
		e.name = "leipeng";
		e.department = "tech";
		m_byID->insert(e.id, e);
		m_byName->insert(e.name, e);
		m_byDepartment->insert(e.department, e);
	// 	BOOST_CURRENT_FUNCTION/BOOST_STATIC_CONSTANT
	// 	WM_ACTIVATETOPLEVEL
	// 	CreateFile
	// 	abcdefghijklmnopqrstuvwxyz
		return 0;
	}

	rpc_ret_t getByID(unsigned id, Employee& em)
	{
		dbmap<unsigned, Employee>::iterator iter = m_byID->find(id);
		if (iter.exist())
		{
			em = iter->second;
			return 0;
		}
		return 1; // not found
	}

	rpc_ret_t getByName(const string& name, std::vector<Employee>& em)
	{
		kmapdset<dbt_string, Employee>::iterator iter = m_byName->find(name);
		if (iter.exist())
		{
			em.swap(iter.get_mutable().second);
			return 0;
		}
		return 1; // not found
	}

	rpc_ret_t getByDepartment(const string& department, std::vector<Employee>& em)
	{
		kmapdset<dbt_string, Employee>::iterator iter = m_byDepartment->find(department);
		if (iter.exist())
		{
			em.swap(iter.get_mutable().second);
			return 0;
		}
		return 1; // not found
	}
};

int main0(int argc, char* argv[])
{
	try {
		SocketAcceptor acceptor("0.0.0.0:8001");
		rpc_server<PortableDataInput, PortableDataOutput> server(&acceptor);

		server.add_servant(
			new DbEmployeeImp,
			"DbEmployeeObject",
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

