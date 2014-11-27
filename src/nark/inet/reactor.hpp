#ifndef __nark_rpc_reactor_h__
#define __nark_rpc_reactor_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include "../util/refcount.hpp"
#include <boost/intrusive_ptr.hpp>
#include <boost/coro>

namespace nark {

class ConnectionState : public RefCounter
{
public:
	int fd;
	void* coro;

	ConnectionState(int fd);
};

typedef boost::intrusive_ptr<ConnectionState> ConnectionStatePtr;

class Reactor
{
	int epfd;
	void* main_coro;
public:
	Reactor();
};

} // namespace nark


#endif // __nark_rpc_reactor_h__
