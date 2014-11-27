/* vim: set tabstop=4 : */
#include "server.hpp"
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <nark/io/IOException.hpp>
#include <nark/inet/SocketStream.hpp>

/**
 @brief 本地接口第一次调用远程对象的方法时，在 rpc server 上创建远程对象

  remote_object 就是一个 id，状态只在 rpc server 上保持
 */

namespace nark { namespace rpc {

class rpc_server_base::MyMutex {
public:
	boost::mutex sessionList;
	boost::mutex stubTable;
	boost::mutex globaleScopeObjects;
	boost::mutex globaleScopeFactory;
	boost::mutex sessionScopeFactory;
};

server_stub_i::~server_stub_i()
{
}

remote_object* session_base::create_object()
{
	assert(0);
	return 0;
}

session_base::session_base(class rpc_server_base* owner)
 : m_owner(owner)
 , m_stubTable(&owner->m_stubTable)
 , m_globaleScopeObjects(&owner->m_globaleScopeObjects)
 , m_sessionScopeFactory(&owner->m_sessionScopeFactory)
{
//	m_sessionScopeObjects.add_ptr(this);
	m_thread = NULL;
}

session_base::~session_base()
{
	m_sessionScopeObjects.destroy();
	delete m_thread;
}

void session_base::call(server_packet_base& header)
{
	switch (header.how_call) {
	default:
		assert(0);
		break;
	case rpc_call_asynch_ordered:
	//	break;
	case rpc_call_asynch_noorder: {
		//	assert(!"implemented now");
			header.stub->sync_call(this, &header);
// 			QueueItem t;
// 			t.args = stub->read_args(m_so_input);
// 			t.stub = stub;
		}
		break;
	case rpc_call_synch:
		header.stub->sync_call(this, &header);
		break;
	}
}

void session_base::start()
{
	m_thread = new boost::thread(boost::bind(&session_base::run, this));
}

void session_base::run()
{
#if BOOST_VERSION > 103301
	boost::this_thread::at_thread_exit(boost::bind(&rpc_server_base::removeSession, m_owner, this));
#endif
	try {
		this->m_id = m_sessionScopeObjects.add_ptr(this);
		assert(1 == this->m_id);
		m_bRun = true;
		while (m_bRun) {
			server_packet_base header;
			this->read_header(header);
			call(header);
		}
	}
	catch (const SocketException& exp) {
		printf("catch: %s, close the session !\n", exp.what());
	}
	catch (const EndOfFileException& exp) {
		printf("catch: %s, close the session!\n", exp.what());
	}
	catch (const std::exception& exp) {
		printf("catch: %s, close the session!\n", exp.what());
	}
#if BOOST_VERSION <= 103301
    m_owner->removeSession(this);
#endif
}


rpc_server_base::rpc_server_base(IAcceptor* acceptor)
: m_acceptor(acceptor)
{
	m_mutex = new MyMutex;
}
rpc_server_base::~rpc_server_base()
{
	for (int nRetry = 0; nRetry < 100; ++nRetry) {
		long remainSessions;
		{
			boost::mutex::scoped_lock lock(m_mutex->sessionList);
			remainSessions = m_sessionList.size();
			if (0 == remainSessions)
				break;
		}
		DEBUG_printf("%ld sessions is not still active\n", remainSessions);
	//	thread::Thread::sleep(100);
	}
	m_globaleScopeObjects.destroy();
	m_globaleScopeFactory.destroy();
	m_sessionScopeFactory.destroy();
	m_stubTable.destroy();
	delete m_mutex;
}

bool rpc_server_base::addto_factroy(ClassMeta* meta, GlobaleScope* scopeTag)
{
	return m_globaleScopeFactory.add(meta);
}
bool rpc_server_base::addto_factroy(ClassMeta* meta, SessionScope* scopeTag)
{
	return m_sessionScopeFactory.add(meta);
}

void rpc_server_base::gen_stubs(ClassMeta* meta)
{
	for (int i = 0, n = meta->stubs.size(); i != n; ++i) {
		server_stub_i* s = meta->stubs[i], *s2 = NULL;
		s->m_id = m_stubTable.add_ptr(s, s->m_name, &s2);
		assert(0 == s2);
	}
}

void rpc_server_base::removeSession(boost::intrusive_ptr<session_base> session)
{
	boost::mutex::scoped_lock lock(m_mutex->sessionList);
	m_sessionList.erase(session);
}

void rpc_server_base::start()
{
//	m_pipeline.setQueueSize(50);
//	m_pipeline.add_step();

	while (true)
	{
		// accept a server connection...

		if (session_base* session = createSession()) {
			{
				boost::mutex::scoped_lock lock(m_mutex->sessionList);
				m_sessionList.insert(session);
			}
			session->start();
		} else
			break;
	}
}

void rpc_server_base::add_servant_imp(GlobaleScope* servant, const std::string& name)
{
	boost::mutex::scoped_lock lock(m_mutex->globaleScopeObjects);
	assert(servant);
	GlobaleScope* servant2 = NULL;
	long id = m_globaleScopeObjects.add_ptr(servant, name, &servant2);
	servant->setID(id);
	servant->set_ext_ptr(this);
}

//! do not delete servant, only remove from rpc_server
void rpc_server_base::remove_servant(GlobaleScopePtr servant)
{
	boost::mutex::scoped_lock lock(m_mutex->globaleScopeObjects);
	assert(servant);
	m_globaleScopeObjects.remove(servant->getID());
}

rpc_ret_t rpc_server_base::
createNamedGlobaleObject(var_size_t* servantID, std::string& className, const std::string& name)
{
	GlobaleScope* x;
	{
		boost::mutex::scoped_lock lock(m_mutex->globaleScopeObjects);
		x = m_globaleScopeObjects.get_rawptr_byname(name);
	}
	if (0 == x) {
		const RoCreator* creator;
		{
			boost::mutex::scoped_lock lock(m_mutex->globaleScopeObjects);
			creator = m_globaleScopeFactory.FindMetaByName(className);
		}
		if (creator) {
			x = (GlobaleScope*)creator->create();
			x->m_classMeta = creator;
			GlobaleScope* y;
			long id = m_globaleScopeObjects.add_ptr(x, name, &y);
			assert(0 == y);
			x->setID(id);
			servantID->t = id;
			return 0;
		}
		else
			return -1; // no creator, class not registered
	}
	else {
		className = x->getClassName();
		servantID->t = x->getID();
		return 1;
	}
}

rpc_ret_t rpc_server_base::
createGlobaleObject(var_size_t* servantID, const std::string& className)
{
	const RoCreator* creator;
	{
		boost::mutex::scoped_lock lock(m_mutex->globaleScopeObjects);
		creator = m_globaleScopeFactory.FindMetaByName(className);
	}
	if (creator) {
		GlobaleScope* x = (GlobaleScope*)creator->create();
		long id = m_globaleScopeObjects.add_ptr(x);
		x->m_classMeta = creator;
		x->setID(id);
		x->set_ext_ptr(this);
		servantID->t = id;
		return 0;
	}
	else
		return -1;
}

rpc_ret_t rpc_server_base::
retrieveGlobaleObject(var_size_t* servantID, const std::string& name)
{
	boost::mutex::scoped_lock lock(m_mutex->globaleScopeObjects);
	GlobaleScopePtr x = m_globaleScopeObjects.get_byname(name);
	if (x) {
		servantID->t = x->getID();
		return 0;
	}
	else {
		servantID->t = 0;
		return -1;
	}
}

rpc_ret_t session_base::
createSessionObject(var_size_t* servantID, const std::string& className)
{
	SessionScopePtr x = m_owner->m_sessionScopeFactory.create(className);
	long id = m_sessionScopeObjects.add_ptr(x);
	x->setID(id);
	x->set_ext_ptr(this);
	servantID->t = id;
	return 0;
}
rpc_ret_t session_base::
createNamedSessionObject(var_size_t* servantID, std::string& className, const std::string& name)
{
	SessionScope* x = m_sessionScopeObjects.get_rawptr_byname(name);
	if (NULL == x) {
		x = m_owner->m_sessionScopeFactory.create(className);
		SessionScope* y = NULL;
		long id = m_sessionScopeObjects.add_ptr(x, name, &y);
		assert(0 == y);
		x->setID(id);
		x->set_ext_ptr(this);
		servantID->t = id;
		return 0;
	}
	else {
		className = x->getClassName();
		servantID->t = x->getID();
		return -1;
	}
}

rpc_ret_t session_base::
retrieveSessionObject(var_size_t* servantID, const std::string& name)
{
	SessionScopePtr x = m_sessionScopeObjects.get_byname(name);
	if (0 == x) {
		servantID->t = 0;
		return -1;
	}
	else {
		servantID->t = x->getID();
		return 0;
	}
}

rpc_ret_t session_base::
createGlobaleObject(var_size_t* servantID, const std::string& className)
{
	return m_owner->createGlobaleObject(servantID, className);
}

rpc_ret_t session_base::
createNamedGlobaleObject(var_size_t* servantID, std::string& className, const std::string& name)
{
	return m_owner->createNamedGlobaleObject(servantID, className, name);
}

rpc_ret_t session_base::
retrieveGlobaleObject(var_size_t* servantID, const std::string& name)
{
	return m_owner->retrieveGlobaleObject(servantID, name);
}

rpc_ret_t session_base::removeSessionObject(var_size_t servantID)
{
	SessionScopePtr x = m_sessionScopeObjects.get_byid(servantID.t);
	m_sessionScopeObjects.remove(servantID.t);
	return 0;
}

} } // namespace::nark::rpc


