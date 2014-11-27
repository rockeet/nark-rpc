#ifndef __nark_ReactAcceptor_h__
#define __nark_ReactAcceptor_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <memory>
#include <list>
#include <vector>
#include <nark/io/IStream.hpp>
#include <nark/inet/SocketStream.hpp>

namespace nark {

class ReactAcceptor : public IAcceptor
{
	std::auto_ptr<class ReactAcceptor_imp> m_impl;
public:
	ReactAcceptor(int port);
	IDuplexStream* accept(); ///< override
};

class Request
{
	class Session* session;
public:
};

class Fiber;

class ReactServer
{
	std::vector<Fiber*> free_fiber;
	std::list<Session*> all_session;
	volatile int m_run;

	Session* wait();

public:

	void run();
	void putRequest(Request* req);
	void setAcceptTimeout(int timeoutMillisecond);
	void setClientTimeout(int timeoutMillisecond);
};

class Session
{
public:
	int fd;
	Fiber*   fiber;
	IDuplexStream* duplex;

	Session(IDuplexStream* duplex)
	{

	}
	virtual Request* decode();
};

class Fiber
{
	Session* session;
	ReactServer* processor;
	std::vector<Fiber*>* free_fiber;
public:
	void setSession(Session* session) { this->session = session; }
	void yield();
	void awake();
	void run();
	bool isFree();
};

class FiberSocketStream : public SocketStream
{
public:
	FiberSocketStream(::SOCKET sock);

protected:
	bool waitfor_again();

	Session* session;
};

} // namespace nark


#endif // __nark_ReactAcceptor_h__
