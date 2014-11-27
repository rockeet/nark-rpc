/* vim: set tabstop=4 : */
#include "client.hpp"
#include <boost/bind.hpp>
#include <nark/num_to_str.hpp>

namespace nark { namespace rpc {

client_stub_i::client_stub_i()
 : m_callid(0)
{
}

client_stub_i::~client_stub_i()
{
}

/**
 @brief rpc_client_base

 每个不同的客户端类型可能会有多个实例，这些实例的stub->m_callid 会因为 server 的不同而不同。
 因此不能由多个 client 共享同一个static client_stub<Client,Function>，而必须 new 新的 stub
 */
rpc_client_base::rpc_client_base(IDuplexStream* duplex)
	: m_minseqid(0)
	, m_sequence_id(0)
	, m_duplex(duplex)
{
	intrusive_ptr_add_ref(this);
}

rpc_client_base::~rpc_client_base()
{
	while (!m_stubTable.empty())
	{
		delete m_stubTable.begin()->second;
		m_stubTable.erase(m_stubTable.begin());
	}
}

// 	void async_packet(boost::intrusive_ptr<client_packet_base> p)
// 	{
// 		boost::mutex::scoped_lock lock(m_mutex_pendings);
// 		while (m_pendings.full())
// 			m_cond_pendings.wait(m_mutex_pendings);
// 		m_pendings.push_back(p);
// 	}

/**
 @brief 将 client_packet 放入待决队列中，以便随后返回
 */
void rpc_client_base::async_packet(client_packet_base* p)
{
	boost::mutex::scoped_lock lock(m_mutex_pendings);
	m_pendings.insert(std::make_pair(p->seqid, p));
}

void rpc_client_base::wait_pending_async()
{
	boost::intrusive_ptr<client_packet_base> packet;
	bool isEmpty;
	{
		boost::mutex::scoped_lock lockP(m_mutex_pendings);
		isEmpty = m_pendings.empty();
	}
	while (!isEmpty)
	{
		{
			boost::mutex::scoped_lock lock(m_mutex_read);
			unsigned seqid = read_seqid();
			{
				boost::mutex::scoped_lock lockP(m_mutex_pendings);
				pending_invokation_set::iterator iter = m_pendings.find(seqid);
				if (m_pendings.end() != iter)
				{
					packet = iter->second;
					m_pendings.erase(iter);
					isEmpty = m_pendings.empty();
				}
			}
		}
		assert(packet);
		packet->on_return();
	}
}

void rpc_client_base::wait_async_return_once()
{
	boost::intrusive_ptr<client_packet_base> packet;
	{
		boost::mutex::scoped_lock lock(m_mutex_read);
		unsigned seqid = read_seqid();
		{
			boost::mutex::scoped_lock lockP(m_mutex_pendings);
			pending_invokation_set::iterator iter = m_pendings.find(seqid);
			if (m_pendings.end() != iter)
			{
				packet = iter->second;
				m_pendings.erase(iter);
			}
		//	m_cond_pendings.notify_all();
		}
	//	packet->read_args(this);
	}
	assert(packet);
	packet->on_return();
}

/**
 @brief 轮询等待未返回的rpc调用
 */
void rpc_client_base::wait_async_return()
{
	m_run = true;
	while (m_run)
	{
		wait_async_return_once();
	}
}

//! 开启异步调用的等待线程
void rpc_client_base::start_async(int nThreads)
{
	assert(nThreads > 0);
	m_run = 1;
	m_threads.resize(nThreads);
	for (int i = 0; i != nThreads; ++i)
	{
		m_threads[i] = new boost::thread(boost::bind(&rpc_client_base::wait_async_return, this));
	}
}

//! 查询 GlobaleScope object
bool rpc_client_base::retrieve_1(GlobaleScope& x, const std::string& instanceName)
{
	var_size_t proxyID;
	rpc_ret_t ret = this->retrieveGlobaleObject(&proxyID, instanceName);
	x.setID(proxyID.t);
	return (0 == ret);
}

//! 查询 SessionScope object
bool rpc_client_base::retrieve_1(SessionScope& x, const std::string& instanceName)
{
	var_size_t proxyID;
	rpc_ret_t ret = this->retrieveSessionObject(&proxyID,instanceName);
	x.setID(proxyID.t);
	return (0 == ret);
}
//! 创建 GlobaleScope object
void rpc_client_base::create_1(GlobaleScope& x, const std::string& instanceName)
{
	var_size_t proxyID;
	std::string className = x.getClassName();
	rpc_ret_t ret = this->createNamedGlobaleObject(&proxyID,className, instanceName);
	if (0 != ret && className != x.getClassName())
	{
		string_appender<> oss;
		oss << "instanceName=" << instanceName << " existed in server, "
			<< "its class=" << className;
		throw std::logic_error(oss.str());
	}
	x.setID(proxyID.t);
}
//! 创建 SessionScope object
void rpc_client_base::create_1(SessionScope& x, const std::string& instanceName)
{
	var_size_t proxyID;
	std::string className = x.getClassName();
	rpc_ret_t ret = this->createNamedSessionObject(&proxyID,className, instanceName);
	if (0 != ret && className != x.getClassName())
	{
		string_appender<> oss;
		oss << "instanceName=" << instanceName << " existed in server, "
			<< "its class=" << className;
		throw std::logic_error(oss.str());
	}
	x.setID(proxyID.t);
}
void rpc_client_base::create_0(GlobaleScope& x)
{
	var_size_t proxyID;
	rpc_ret_t ret = this->createGlobaleObject(&proxyID, x.getClassName());
	(void)ret;
	x.setID(proxyID.t);
}
void rpc_client_base::create_0(SessionScope& x)
{
	var_size_t proxyID;
	rpc_ret_t ret = this->createSessionObject(&proxyID,x.getClassName());
	(void)ret;
	x.setID(proxyID.t);
}

} } // namespace nark::rpc

