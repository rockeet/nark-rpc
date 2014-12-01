/* vim: set tabstop=4 : */
#ifndef __nark_rpc_client_io_h__
#define __nark_rpc_client_io_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
# pragma warning(disable: 4819)
#endif

#include <limits.h>
#include <exception>
#include <boost/type_traits.hpp>

#include "rpc_basic.hpp"

namespace nark { namespace rpc {

template<class Input>  class client_object_input
{
	Input* p;

public:
	explicit client_object_input(Input* input = 0) : p(input) {}

	void attach(Input* input) { p = input; }
	Input& get() const { return *p; }

	template<class T> client_object_input& operator>>(boost::intrusive_ptr<T>& x)
	{
		return *this >> *x;
	}

	template<class T> client_object_input& operator &(T& x) { return *this >> x; }

	// T must derived from remote_object
	template<class T> client_object_input& operator>>(T*& x) { return *this >> *x; }
	template<class T> client_object_input& operator>>(T& x)
	{
		// do nothing...
		typename T::SFINAE_ro_self_t* for_check = 0;
		(void)(for_check);
// 		var_size_t objid;
// 		*p >> objid;
// 		if (x.getID() == 0)
// 		{
// 			if (0 == objid.t)
// 			{
// 				string_appender<> oss;
// 				oss << "load remote_object from server failed, objid = 0";
// 				throw rpc_exception(oss.str());
// 			}
// 			x.setID(objid.t);
// 		}
// 		else if (x.getID() != objid.t)
// 		{
// 			string_appender<> oss;
// 			oss << "load remote_object from server failed, server_objid = " << objid.t
// 				<< "local objid = " << x.getID();
// 			throw rpc_exception(oss.str());
// 		}
		return *this;
	}

	// do nothing...
	template<class T> client_object_input& operator &(rpc_in<T> x) { return *this; }
	template<class T> client_object_input& operator>>(rpc_in<T> x) { return *this; }

	template<class T> client_object_input& operator &(rpc_out<T> x) { *p >> x.r; return *this; }
	template<class T> client_object_input& operator>>(rpc_out<T> x) { *p >> x.r; return *this; }

	template<class T> client_object_input& operator &(rpc_inout<T> x) { *p >> x.r; return *this; }
	template<class T> client_object_input& operator>>(rpc_inout<T> x) { *p >> x.r; return *this; }
};

template<class Output>  class client_object_output
{
	Output* p;

public:
	explicit client_object_output(Output* output = 0) : p(output) {}

	void attach(Output* output) { p = output; }
	void flush() { p->flush(); }
	Output& get() const { return *p; }

	template<class T> client_object_output& operator<<(const boost::intrusive_ptr<T>& x)
	{
		return *this << *x;
	}

	template<class T> client_object_output& operator &(const T& x) { return *this << x; }

	// T must derived from remote_object
	template<class T> client_object_output& operator<<(      T* x) { return *this << *x; }
	template<class T> client_object_output& operator<<(const T* x) { return *this << *x; }
	template<class T> client_object_output& operator<<(const T& x)
	{
		typename T::SFINAE_ro_self_t* for_check = 0;
		(void)(for_check);
		*p << var_size_t(x.getID());
		return *this;
	}

	template<class T> client_object_output& operator &(rpc_in<T> x) { *p << x.r; return *this; }
	template<class T> client_object_output& operator<<(rpc_in<T> x) { *p << x.r; return *this; }

	// do nothing...
	template<class T> client_object_output& operator &(rpc_out<T> x) { return *this; }
	template<class T> client_object_output& operator<<(rpc_out<T> x) { return *this; }

	template<class T> client_object_output& operator &(rpc_inout<T> x) { *p << x.r; return *this; }
	template<class T> client_object_output& operator<<(rpc_inout<T> x) { *p << x.r; return *this; }
};

} } // namespace::nark::rpc


#endif // __nark_rpc_client_io_h__
