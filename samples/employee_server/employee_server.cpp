// employee_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#endif

#include <nark/rpc/server.hpp>
#include <nark/inet/SocketStream.hpp>

#include "../employee.h"

// not need auto create
class DbEmployeeImp : public DbEmployee
{
private:
#ifdef EMPLOYEE_USE_BDB
	DbEnv m_env;
	auto_ptr<dbmap<unsigned, Employee> > m_byID;
	auto_ptr<kmapdset<dbt_string, Employee> > m_byName, m_byDepartment;
#else
	int m_env; // dummy
	std::map<unsigned, Employee> m_byID;
	std::map<string, vector<Employee> > m_byName, m_byDepartment;
#endif

public:
	DbEmployeeImp()	: m_env(0)
	{
#ifdef EMPLOYEE_USE_BDB
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
#endif
	}

	rpc_ret_t fillData()
	{
		Employee e;
		e.id = 1;
		e.name = "leipeng";
		e.department = "tech";
#ifdef EMPLOYEE_USE_BDB
		m_byID->insert(e.id, e);
		m_byName->insert(e.name, e);
		m_byDepartment->insert(e.department, e);
#else
		m_byID[e.id] = e;
		m_byName[e.name].push_back(e);
		m_byDepartment[e.department].push_back(e);
#endif
	// 	BOOST_CURRENT_FUNCTION/BOOST_STATIC_CONSTANT
	// 	WM_ACTIVATETOPLEVEL
	// 	CreateFile
	// 	abcdefghijklmnopqrstuvwxyz
		return 0;
	}

	rpc_ret_t getByID(unsigned id, Employee& em)
	{
#ifdef EMPLOYEE_USE_BDB
		dbmap<unsigned, Employee>::iterator iter = m_byID->find(id);
		if (iter.exist())
		{
			em = iter->second;
			return 0;
		}
#else
		std::map<unsigned, Employee>::iterator iter = m_byID.find(id);
		if (m_byID.end() == iter) {
			em = iter->second;
			return 0;
		}
#endif
		return 1; // not found
	}

	rpc_ret_t getByName(const string& name, std::vector<Employee>& em)
	{
#ifdef EMPLOYEE_USE_BDB
		kmapdset<dbt_string, Employee>::iterator iter = m_byName->find(name);
		if (iter.exist())
		{
			em.swap(iter.get_mutable().second);
			return 0;
		}
#else
		std::map<string, vector<Employee> >::iterator iter = m_byName.find(name);
		if (m_byName.end() != iter) {
			em = iter->second;
			return 0;
		}
#endif
		return 1; // not found
	}

	rpc_ret_t getByDepartment(const string& department, std::vector<Employee>& em)
	{
#ifdef EMPLOYEE_USE_BDB
		kmapdset<dbt_string, Employee>::iterator iter = m_byDepartment->find(department);
		if (iter.exist())
		{
			em.swap(iter.get_mutable().second);
			return 0;
		}
#else
		std::map<string, vector<Employee> >::iterator iter = m_byDepartment.find(department);
		if (m_byDepartment.end() != iter) {
			em = iter->second;
			return 0;
		}
#endif
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

