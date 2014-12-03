/* vim: set tabstop=4 : */
#ifndef __nark_rpc_client_h__
#define __nark_rpc_client_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
# pragma warning(disable: 4819)
#endif

/**
 @file rpc Client Side
 */

#include <boost/preprocessor/punctuation.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>

#define IF_RPC_SERVER(Server, Client) Client

#define RPC_CLIENT_SIDE

#include "rpc_basic.hpp"
#include "arg_traits.hpp"
#include "client_io.hpp"
#include <nark/io/IStream.hpp>

namespace nark { namespace rpc {

//BOOST_STATIC_CONSTANT(unsigned, rpc_call_byid = 1);
//BOOST_STATIC_CONSTANT(unsigned, rpc_copy_args = 2);

//////////////////////////////////////////////////////////////////////////
//! on client side...
//!
//! rpc 接口类中需要 typedef 一个类的别名： self_t
//! 使用宏的目的只有一个：推导 FunName 的参数表，将参数表转化成 tuple
//! 参数表一般是： (T1, T2, T3) 等等
//! 因此无法变成直接转调其它函数的形式，使用 tuple，就可以在 mem_fun_binder 中转调其他函数
//!
#define RPC_DECLARE_MF_EX(FunName, ArgList, PureVirtualTagIgnored) \
	typedef rpc_ret_t (self_t::*FunName##_stub_t) ArgList; \
	client_stub_ref<FunName##_stub_t> FunName;

#define RPC_DECLARE_MF(FunName, ArgList) RPC_DECLARE_MF_EX(FunName, ArgList, =0)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define BEGIN_RPC_ADD_MF_EX(ThisClass, ClassName) \
	typedef ThisClass self_t;				\
	FEBIRD_RPC_GEN_full_mf_name(ClassName)	\
	template<class Client>					\
	void bind_client(Client* client)		\
	{	set_ext_ptr(client);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define BEGIN_RPC_ADD_MF(ThisClass) BEGIN_RPC_ADD_MF_EX(ThisClass, #ThisClass)

#define RPC_ADD_MF(FunName) \
	client->bind_stub(FunName, full_mf_name(#FunName), this);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define END_RPC_ADD_MF()  }
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

///
/// client packet hierarchy
///
/// client_packet_base
///   - client_packet_fun<CallBackFunction>
///       - client_packet_io<Client, Function, CallBackFunction>
///           - client_packet<Client, arglist>
///

///
/// client stub hierarchy
///
/// client_stub_i
///   - client_stub<Client, Function>
///

///
/// rpc_client hierarchy
//
/// rpc_client_basebase(alias of SessionScope)
///     rpc_client_base
///         rpc_client<Input, Output>

typedef SessionScope rpc_client_basebase;
class FEBIRD_DLL_EXPORT client_packet_base;

template<class Function> class client_stub_ref;
template<class Client, class ArgList, class Function> class client_packet;

class FEBIRD_DLL_EXPORT client_stub_i
{
public:
	std::string  m_className;
	std::string  m_name;
	unsigned     m_callid;

	client_stub_i();
	virtual ~client_stub_i();

	//! @brief 创建 refpacket
	//! 不创建调用时的实参副本，会改变原先的实参，用户需要保证原先的实参在异步返回时仍然有效
	//! 如果用户需要在 on_return 中使用原先传入的参数本身，将会创建 refpacket
	virtual boost::intrusive_ptr<client_packet_base> refpacket_create(void* argrefs) = 0;

	//! @brief 创建 valpacket
	//! 会创建调用时的实参副本，原先的实参不会被改变，用户的工作量最少
	//! 一般情况下会调用这个函数，它不需要用户自己管理原参数（async调用传入的参数引用）的分配与释放
	virtual boost::intrusive_ptr<client_packet_base> valpacket_create(void* argrefs) = 0;

	//! @brief 使用 stubID 来调用远程函数
	//! 如果 stubID 未知（=0），仍然会调用call_byname
	virtual rpc_ret_t call_byid(rpc_client_basebase* client, void* argrefs) = 0;

	//! 使用函数名来调用
	virtual rpc_ret_t call_byname(rpc_client_basebase* client, void* argrefs) = 0;

	//! 写入stubID和参数
	virtual void send_id_args(rpc_client_basebase* client, client_packet_base* packet) = 0;

	//! 写入stubName和参数
	virtual void send_nm_args(rpc_client_basebase* client, client_packet_base* packet) = 0;

	//! 读取参数
	virtual void read_args(rpc_client_basebase* client, client_packet_base* packet) = 0;
};

class FEBIRD_DLL_EXPORT client_packet_base : public RefCounter
{
public:
	class client_stub_i* stub;
	unsigned     seqid;
	short		 how_call;
	bool		 isbyid;
	rpc_ret_t    retv;

	virtual void send_id_args(rpc_client_basebase* client) = 0;
	virtual void send_nm_args(rpc_client_basebase* client) = 0;
	virtual void read_args(rpc_client_basebase* client) = 0;

	virtual void on_return() = 0;

	void async_packet(rpc_client_basebase* client);
};

//! @brief 定义 client_packet 的回调函数
//! 回调函数的原型是：
//! void (ThisType::*)(const client_packet_base& + original param declare)
template<class CallBackFunction>
class client_packet_fun : public client_packet_base
{
public:
	CallBackFunction on_ret;
};

//! 实现 client_packet io
template<class Client, class Function, class CallBackFunction>
class client_packet_io : public client_packet_fun<CallBackFunction>
{
public:
	typedef arglist_ref<Function> arglist_ref_t;

	rpc_ret_t t_call_byid(rpc_client_basebase* vpclient, arglist_ref_t& refs) {
		Client* client = (Client*)(vpclient);
		this->how_call = rpc_call_synch;
		if (this->stub->m_callid)
			t_send_id_args(client, refs);
		else
			t_send_nm_args(client, refs);
		boost::mutex::scoped_lock lock(client->m_mutex_read);
		var_size_t vseqid1;
		client->m_co_input.get() >> vseqid1;
		assert(this->seqid == vseqid1.t);
		t_read_args(client, refs);
		return this->retv;
	}
	rpc_ret_t t_call_byname(rpc_client_basebase* vpclient, arglist_ref_t& refs) {
		Client* client = (Client*)(vpclient);
		this->how_call = rpc_call_synch;
		t_send_nm_args(client, refs);
		boost::mutex::scoped_lock lock(client->m_mutex_read);
		var_size_t vseqid1;
		client->m_co_input.get() >> vseqid1;
		t_read_args(client, refs);
		assert(this->seqid == vseqid1.t);
		return this->retv;
	}
	void t_read_args(rpc_client_basebase* vpclient, arglist_ref_t& refs) {
		Client* client = (Client*)(vpclient);
		client->m_co_input.get() >> this->retv;
		refs.load(client->m_co_input);
		if (!this->isbyid) { // called by name, read callid
			var_size_t stubID;
			client->m_co_input.get() >> stubID;
			this->stub->m_callid = stubID;
		}
	}
	void t_send_id_args(rpc_client_basebase* vpclient, arglist_ref_t& refs) {
		this->isbyid = true;
		Client* client = (Client*)(vpclient);
		boost::mutex::scoped_lock lock(client->m_mutex_write);
		this->seqid = client->next_sequence_id();
		client->m_co_output.get()
			<< var_size_t(this->seqid)
			<< var_size_t(this->stub->m_callid)
			<< var_size_t(this->how_call)
			;
		refs.save(client->m_co_output);
		client->m_co_output.flush();
	}
	void t_send_nm_args(rpc_client_basebase* vpclient, arglist_ref_t& refs) {
		this->isbyid = false;
		Client* client = (Client*)(vpclient);
		boost::mutex::scoped_lock lock(client->m_mutex_write);
		this->seqid = client->next_sequence_id();
		client->m_co_output.get()
			<< var_size_t(this->seqid)
			<< (byte)(0)
			<< this->stub->m_name
			<< var_size_t(this->how_call)
			;
		refs.save(client->m_co_output);
		client->m_co_output.flush();
	}
};

//
//-------------------------------------------------------------------------
//
// partial specializations, use BOOST_PP..
//
#include <boost/preprocessor/iterate.hpp>
#define BOOST_PP_ITERATION_LIMITS (0, 9)
#define BOOST_PP_FILENAME_1       <nark/rpc/pp_client_stub.hpp>
#include BOOST_PP_ITERATE()
//-------------------------------------------------------------------------
//

class FEBIRD_DLL_EXPORT rpc_client_base : public SessionScope
{
protected:
	std::map<std::string, client_stub_i*> m_stubTable;
//	circular_queue<boost::intrusive_ptr<client_packet_base> > m_pendings;
//	boost::condition m_cond_pendings;
	typedef std::map<unsigned, boost::intrusive_ptr<client_packet_base> > pending_invokation_set;
	pending_invokation_set m_pendings;
	boost::mutex     m_mutex_pendings;
	unsigned	 m_minseqid;
	unsigned	 m_sequence_id;
	volatile int m_run;
	std::vector<boost::thread*> m_threads;
	IDuplexStream* m_duplex;

protected:
	bool retrieve_1(GlobaleScope& x, const std::string& instanceName);
	bool retrieve_1(SessionScope& x, const std::string& instanceName);
	void create_1(GlobaleScope& x, const std::string& instanceName);
	void create_1(SessionScope& x, const std::string& instanceName);
	void create_0(GlobaleScope& x);
	void create_0(SessionScope& x);

	typedef rpc_client_base my_self_t;
#include "rpc_interface.hpp"

public:
	boost::mutex m_mutex_read, m_mutex_write;

	explicit rpc_client_base(IDuplexStream* duplex);
	virtual ~rpc_client_base();

	void async_packet(client_packet_base* p);
	uint32_t next_sequence_id() { return ++m_sequence_id; }

	virtual unsigned read_seqid() = 0;

	void wait_pending_async();
	void wait_async_return_once();
	void wait_async_return();
	void start_async(int nThreads);
	void stop_async() { m_run = 0; }
};

/**
 @brief 实现一个客户端函数的 stub
 由 client_stub_ref 来引用，多个 client_stub_ref 可以引用同一个 client_stub
 */
template<class Client, class Function>
class client_stub : public client_stub_i
{
public:
	typedef arglist_ref<Function> aref_t;
	typedef arglist_val<Function> aval_t;
	typedef client_packet<Client, aref_t, Function> refpacket_t;
	typedef client_packet<Client, aval_t, Function> valpacket_t;

	explicit client_stub(const std::string& name) { m_name = name; }

	boost::intrusive_ptr<client_packet_base> refpacket_create(void* argrefs) {
		return new refpacket_t(*(aref_t*)argrefs);
	}
	boost::intrusive_ptr<client_packet_base> valpacket_create(void* argrefs) {
		return new valpacket_t(*(aref_t*)argrefs);
	}

	virtual rpc_ret_t call_byid(rpc_client_basebase* client, void* refs) {
		client_packet<Client, aref_t*, Function> packet(refs);
		packet.stub = this;
		return packet.t_call_byid(client, *(aref_t*)refs);
	}
	virtual rpc_ret_t call_byname(rpc_client_basebase* client, void* refs) {
		client_packet<Client, aref_t*, Function> packet(refs);
		packet.stub = this;
		return packet.t_call_byname(client, *(aref_t*)refs);
	}

	virtual void send_id_args(rpc_client_basebase* client, client_packet_base* packet) {
		packet->stub = this;
		packet->send_id_args(client);
	}
	virtual void send_nm_args(rpc_client_basebase* client, client_packet_base* packet) {
		packet->stub = this;
		packet->send_nm_args(client);
	}
	virtual void read_args(rpc_client_basebase* client, client_packet_base* packet) {
		packet->stub = this;
		packet->read_args(client);
	}
};

template<template<class IStream_> class Input,
		 template<class OStream_> class Output
		>
class rpc_client : public rpc_client_base
{
	DECLARE_NONE_COPYABLE_CLASS(rpc_client)
public:
	typedef rpc_client self_t;
	typedef Input<InputBuffer>   input_t;
	typedef Output<OutputBuffer> output_t;
	typedef client_object_input<input_t>   co_input_t;
	typedef client_object_output<output_t> co_output_t;

	co_input_t  m_co_input;
	co_output_t m_co_output;

	rpc_client(IDuplexStream* duplex)
		: rpc_client_base(duplex)
	{
		m_input.set_bufsize(8*1024);
		m_output.set_bufsize(8*1024);
		m_co_input.attach(&m_input);
		m_co_output.attach(&m_output);

		this->m_id = 1; //
		m_input.attach(m_duplex);
		m_output.attach(m_duplex);

		// 1st 'this' is as SessionObject
		// 2nd 'this' is as Client
		this->bind_client(this);
	}
	// @Override
	unsigned read_seqid() {
		var_size_t seqid;
		m_input >> seqid;
		return seqid.t;
	}

	//! @brief 查询远程对象
	//! 根据 Class 是 GlobaleScope, 还是 SessionScope, 自动决定如何查询
	template<class Class>
	bool retrieve(boost::intrusive_ptr<Class>& x, const std::string& instanceName) {
		x.reset(new Class);
		x->bind_client(this);
		return this->retrieve_1(*x, instanceName);
	}
	//! @brief 创建远程对象
	//! 根据 Class 是 GlobaleScope, 还是 SessionScope, 自动决定如何创建
	template<class Class>
	void create(boost::intrusive_ptr<Class>& x, const std::string& instanceName) {
		x.reset(new Class);
		x->bind_client(this);
		this->create_1(*x, instanceName);
	}
	//! @brief 创建匿名远程对象
	//! 根据 Class 是 GlobaleScope, 还是 SessionScope, 自动决定如何创建
	//! 匿名的远程对象只有ID，没有名字
	template<class Class>
	void create(boost::intrusive_ptr<Class>& x) {
		x.reset(new Class);
		x->bind_client(this);
		this->create_0(*x);
	}
	//! @brief 将对象 x 绑定到该客户端
	//! @note 仅由 RPC_ADD_MF 来引用
	template<class Class, class Function>
	void bind_stub(client_stub_ref<Function>& fref, const std::string& funName, Class* x) {
		client_stub_i*&rp = m_stubTable[funName];
		if (NULL == rp)
			rp = new client_stub<rpc_client, Function>(funName);
		fref.bind(x, rp);
	}

	class CreateHelper0 {
		rpc_client* m_client;
	public:
		template<class Class>
		operator boost::intrusive_ptr<Class>() const {
			boost::intrusive_ptr<Class> obj(new Class());
			obj->bind_client(m_client);
			m_client->create_0(*obj);
			return obj;
		}
		explicit CreateHelper0(rpc_client* client) : m_client(client) {}
	};
	class CreateHelper1 {
		rpc_client* m_client;
		const std::string* m_instanceName;
	public:
		template<class Class>
		operator boost::intrusive_ptr<Class>() const {
			boost::intrusive_ptr<Class> obj(new Class());
			obj->bind_client(m_client);
			m_client->create_1(*obj, *m_instanceName);
			return obj;
		}
		explicit CreateHelper1(rpc_client* client, const std::string* instanceName)
		   	: m_client(client), m_instanceName(instanceName) {}
	};
	friend class CreateHelper0;
	friend class CreateHelper1;

	//! sample usage: FileObjPtr file = client.create();
	CreateHelper0 create() { return CreateHelper0(this); }

	//! sample usage: EchoPtr echo = client.create("global_echo1");
	CreateHelper1 create(const std::string& instanceName)
	  { return CreateHelper1(this, &instanceName); }

private:
	input_t   m_input;
	output_t  m_output;
};

// must defined here, all required types are complete here
inline void client_packet_base::async_packet(rpc_client_basebase* vpclient) {
	rpc_client_base* client = static_cast<rpc_client_base*>(vpclient);
	client->async_packet(this);
}


} } // namespace nark::rpc

#endif // __nark_rpc_client_h__


