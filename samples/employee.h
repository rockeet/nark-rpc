#include <nark/io/MemStream.hpp>

using namespace std;
using namespace nark;
using namespace nark::rpc;

struct Employee
{
	typedef unsigned key_type; // for id, used by kmapdset

	std::string name, department;
	unsigned id;

	void print(ostream& os) const
	{
		os  << "id=" << dec << id << "[" << hex << id << "]"
			<< ", name=" << name << ", department=" << department << endl;
	}
#ifdef _DEBUG
	// for set break point and step into
	friend
	void DataIO_loadObject(PortableDataInput<MinMemIO>& dio, Employee& x)
	{
		dio >> x.name;
		dio >> x.department;
		dio >> x.id;
	}
	friend
	void DataIO_loadObject(PortableDataInput<MemIO>& dio, Employee& x)
	{
		dio >> x.name;
		dio >> x.department;
		dio >> x.id;
	}
	template<class DataIO>
	friend
	void DataIO_loadObject(DataIO& dio, Employee& x)
	{
		dio >> x.name;
		dio >> x.department;
		dio >> x.id;
	}
	template<class DataIO>
	friend
	void DataIO_saveObject(DataIO& dio, const Employee& x)
	{
		dio << x.name;
		dio << x.department;
		dio << x.id;
	}
	friend
	void DataIO_loadObject(PortableDataInput<InputBuffer*>& dio, Employee& x)
	{
		dio >> x.name;
		dio >> x.department;
		dio >> x.id;
	}
	friend
	void DataIO_saveObject(PortableDataOutput<OutputBuffer*>& dio, const Employee& x)
	{
		dio << x.name;
		dio << x.department;
		dio << x.id;
	}
#else
	DATA_IO_LOAD_SAVE(Employee, &name&department&id)
#endif
};

BEGIN_RPC_INTERFACE(DbEmployee, GlobaleScope)
	RPC_ADD_MF(fillData)
	RPC_ADD_MF(getByID)
	RPC_ADD_MF(getByName)
	RPC_ADD_MF(getByDepartment)
END_RPC_ADD_MF()
    RPC_DECLARE_MF(fillData, ())
    RPC_DECLARE_MF(getByID, (unsigned id, Employee& em))
    RPC_DECLARE_MF(getByName, (const std::string& name, std::vector<Employee>& ems)) // maybe multi employee has same
    RPC_DECLARE_MF(getByDepartment, (const std::string& name, std::vector<Employee>& ems))
END_RPC_INTERFACE()
