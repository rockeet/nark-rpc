#ifndef __server_param_h__
#define __server_param_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/smart_ptr.hpp>

namespace nark { namespace rpc {

#if 0
// will not used in multi thread, only in param
template<class T> class RpcRef
{
	int nRef;

	friend void intrusive_ptr_add_ref(RpcRef* p) { ++p->nRef; }
	friend void intrusive_ptr_release(RpcRef* p) {	if (0 == --p->nRef) delete p; }

public:
	RpcRef() : nRef(0) {}
	T t;
};

template<class T> class rpc_in
{
	boost::intrusive_ptr<RpcRef<T> > p;

public:
	rpc_in() : p(new RpcRef<T>) {}

	const T& get() const { return p->t; }

	template<class DataIO>
	friend void DataIO_saveObject(DataIO& dio, const rpc_in<T>& x)
	{
		// do nothing...
	}
	template<class DataIO>
	friend void DataIO_loadObject(DataIO& dio, rpc_in<T>& x)
	{
		dio >> x.r->t;
	}
};

template<class T> class rpc_out
{
	boost::intrusive_ptr<RpcRef<T> > p;

public:
	rpc_out() : p(new RpcRef<T>) {}

	T& get() { return p->t; }

	template<class DataIO>
	friend void DataIO_saveObject(DataIO& dio, const rpc_out<T>& x)
	{
		dio << x.r->t;
	}
	template<class DataIO>
	friend void DataIO_loadObject(DataIO& dio, rpc_out<T>& x)
	{
		// do nothing...
	}
};
template<class T> rpc_out<T> make_rpc_out(const T& x) { return rpc_out<T>(x); }

template<class T> class rpc_inout
{
	boost::intrusive_ptr<RpcRef<T> > p;

public:
	rpc_inout() : p(new RpcRef<T>) {}

	T& get() const { return x.r->t; }

	template<class DataIO>
	friend void DataIO_saveObject(DataIO& dio, const rpc_inout<T>& x)
	{
		dio << x.r->t;
	}
	template<class DataIO>
	friend void DataIO_loadObject(DataIO& dio, rpc_inout<T>& x)
	{
		dio >> x.r->t;
	}
};
#else
template<class T> class rpc_in
{
	T* p;
public:
	rpc_in(T& x) : p(&x) {}
	const T& get() const { return *p; }

	template<class DataIO>
	friend void DataIO_saveObject(DataIO& dio, const rpc_in<T>& x)
	{
		// do nothing...
	}
	template<class DataIO>
	friend void DataIO_loadObject(DataIO& dio, rpc_in<T>& x)
	{
		dio >> *x.r;
	}
};

template<class T> class rpc_out
{
	T* p;
public:
	rpc_out(T& x) : p(&x) {}
	T& get() const { return *p; }

	template<class DataIO>
	friend void DataIO_saveObject(DataIO& dio, const rpc_out<T>& x)
	{
		dio << *x.r;
	}
	template<class DataIO>
	friend void DataIO_loadObject(DataIO& dio, rpc_out<T>& x)
	{
		// do nothing...
	}
};

template<class T> class rpc_inout
{
	T* p;
public:
	rpc_inout(T& x) : p(&x) {}
	T& get() const { return *p; }

	template<class DataIO>
	friend void DataIO_saveObject(DataIO& dio, const rpc_inout<T>& x)
	{
		dio << *x.r;
	}
	template<class DataIO>
	friend void DataIO_loadObject(DataIO& dio, rpc_inout<T>& x)
	{
		dio >> *x.r;
	}
};
#endif // RPC_SERVER_USE_INTRUSIVE_PTR

} } // namespace::nark::rpc

#endif // __server_param_h__
