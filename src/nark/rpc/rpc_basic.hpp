/* vim: set tabstop=4 : */
#ifndef __tlib_rpc_rpc_basic_h__
#define __tlib_rpc_rpc_basic_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif


#include <nark/io/DataIO.hpp>
#include <nark/io/StreamBuffer.hpp>
#include <nark/io/access_byid.hpp>

namespace nark { namespace rpc {

#define FEBIRD_RPC_HELLO "nark rpc hello"

#define RPC_TYPEDEF_PTR(Class)  typedef boost::intrusive_ptr<Class> Class##Ptr

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define BEGIN_RPC_INTERFACE(InterfaceName, Scope) \
	typedef boost::intrusive_ptr<class InterfaceName> InterfaceName##Ptr; \
	class InterfaceName : public Scope { \
	public:	BEGIN_RPC_ADD_MF(InterfaceName)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define END_RPC_INTERFACE() };
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

enum {
	rpc_call_asynch_noorder	= 1,
	rpc_call_asynch_ordered = 2,
	rpc_call_synch	        = 3
};

#define FEBIRD_RPC_GEN_full_mf_name(ClassName)	\
	static std::string full_mf_name(const char* mf_name)	\
	{														\
		const static char szPrefix[] = ClassName;			\
		std::string name;									\
		name.reserve(sizeof(szPrefix)+2+strlen(mf_name));	\
		name.append(szPrefix);								\
		name.append("::");									\
		name.append(mf_name);								\
		return name;										\
	}														\
	const char* getClassName() const { return ClassName; }	\
	static const char* s_getClassName() { return ClassName; }
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

typedef int32_t rpc_ret_t;

/**
 @brief 输入参数
 在 client 仅发送，不接收 @see client_io.hpp
 在 server 仅接收，不发送 @see server_io.hpp
 */
template<class T> struct rpc_in
{
	T& r;
	rpc_in(const T* x) : r(const_cast<T&>(*x)) {}
	rpc_in(const T& x) : r(const_cast<T&>(x)) {}
	rpc_in(T* x) : r(const_cast<T&>(*x)) {}
	rpc_in(T& x) : r(x) {}
	const T* operator->() const { return &r; }
	T* operator->() { return &r; }
};

/**
 @brief 输出参数
 在 client 仅接收，不发送 @see client_io.hpp
 在 server 仅发送，不接收 @see server_io.hpp
 */
template<class T> struct rpc_out
{
	T& r;
	rpc_out(T& x) : r(x) {}
	rpc_out(T* x) : r(*x) {}
	const T* operator->() const { return &r; }
	T* operator->() { return &r; }
};

/**
 @brief 输入/输出参数
 在 client/server 既发送，又接收 @see client_io.hpp, server_io.hpp
 */
template<class T> struct rpc_inout
{
	T& r;
	rpc_inout(T& x) : r(x) {}
	const T* operator->() const { return &r; }
	T* operator->() { return &r; }
};

class FEBIRD_DLL_EXPORT rpc_exception : public std::exception
{
protected:
	std::string m_message;
public:
	explicit rpc_exception(const char* szMsg = "nark::rpc::rpc_exception");
	explicit rpc_exception(const std::string& szMsg);
	virtual ~rpc_exception() throw();

	const char* what() const throw();
};

typedef class remote_object* (*create_fun_t)();
struct FEBIRD_DLL_EXPORT RoCreator // : public RefCounter
{
	create_fun_t create;
	unsigned     classID;
	std::string  className;

	virtual ~RoCreator();
};

class rpc_uint_ptr_auto_cast {
	uintptr_t p; // pointer
public:
	template<class Target> operator Target*() const {
		return reinterpret_cast<Target*>(p);
	}
	explicit rpc_uint_ptr_auto_cast(uintptr_t p_) : p(p_) {}
};

//! all states is store on server, serialization will only pass the objectID
class FEBIRD_DLL_EXPORT remote_object : public RefCounter
{
public:
	typedef remote_object SFINAE_ro_self_t; // for SFINAE
	typedef remote_object* i_am_remote_object_tag;

	uint32_t getID() const { return m_id; }
	void setID(uint32_t id) { m_id = id; }

	remote_object() { m_id = 0; m_flag_ptr = 0; }
	virtual ~remote_object();

	void set_flags(unsigned flags) {
		assert((flags & ~3u) == 0);
		m_flag_ptr = (m_flag_ptr & ~(uintptr_t)3u) | (flags & 3u);
	}
	uintptr_t test_flags(uintptr_t flags) const {
		assert((flags & ~3u) == 0);
		return m_flag_ptr & flags;
	}
	rpc_uint_ptr_auto_cast get_ext_ptr() const {
		return rpc_uint_ptr_auto_cast(m_flag_ptr & ~(uintptr_t)3);
	}
	void set_ext_ptr(void* p) {
		m_flag_ptr = (uintptr_t)p | (m_flag_ptr & 3u);
	}

	virtual const char* getClassName() const = 0;

	const RoCreator* m_classMeta;
protected:
	uintptr_t m_flag_ptr;
	uint32_t  m_id;

	DATA_IO_DISABLE_LOAD_SAVE(remote_object)
};
typedef boost::intrusive_ptr<remote_object> remote_object_ptr;

class FEBIRD_DLL_EXPORT GlobaleScope : public remote_object
{
public:
	typedef GlobaleScope SFINAE_gs_self_t; // for SFINAE
};
typedef boost::intrusive_ptr<GlobaleScope> GlobaleScopePtr;
class FEBIRD_DLL_EXPORT SessionScope : public remote_object
{
public:
	typedef SessionScope SFINAE_ss_self_t; // for SFINAE
};
typedef boost::intrusive_ptr<SessionScope> SessionScopePtr;

class FEBIRD_DLL_EXPORT ObjectFactoryBase
{
protected:
	AccessByNameID<RoCreator*> m_map;

public:
	//! 使用 classID 创建对象
	remote_object* create(unsigned classID) const;
	//! 使用 className 创建对象
	remote_object* create(const std::string& className) const;

	//! @brief 将类信息加入工厂
	//! 这个函数会为 meta 分配一个 id，该 id 在整个生存期都有效
	bool add(RoCreator* meta);

	const RoCreator* FindMetaByName(const std::string& className) const;
	const RoCreator* FindMetaByID(unsigned classID) const {
		return m_map.get_byid(classID);
	}
	void destroy() { m_map.destroy(); }
};

template<class ObjectBase>
class ObjectFactory : public ObjectFactoryBase
{
public:
	//! 使用 classID 创建对象
	ObjectBase* create(unsigned classID) const {
		return (ObjectBase*)ObjectFactoryBase::create(classID);
	}
	//! 使用 className 创建对象
	ObjectBase* create(const std::string& className) const {
		return (ObjectBase*)ObjectFactoryBase::create(className);
	}
};


} } // namespace nark::rpc

#endif // __tlib_rpc_rpc_basic_h__
