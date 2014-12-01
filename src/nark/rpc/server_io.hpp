/* vim: set tabstop=4 : */
#ifndef __nark_rpc_server_io_h__
#define __nark_rpc_server_io_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
# pragma warning(disable: 4819)
#endif

#include <limits.h>
#include <boost/type_traits.hpp>

#include <nark/util/access_byid.hpp>

#include "rpc_basic.hpp"
#include "arg_traits.hpp"

namespace nark { namespace rpc {

void FEBIRD_DLL_EXPORT
incompitible_class_cast(remote_object* x, const char* szRequestClassName);

template<class Input>  class server_object_input
{
	AccessByNameID<GlobaleScopePtr>* m_globale;
	AccessByNameID<SessionScopePtr>* m_session;
	Input* p;

	template<class T, class U>
	void load(T*& x, AccessByNameID<boost::intrusive_ptr<U> >* byid)
	{
		typename T::SFINAE_ro_self_t* for_check = 0;
		(void)(for_check);
		var_size_t objid;
		*p >> objid;
		std::string err;
		if (!byid->check_id(objid.t, T::s_getClassName(), err)) {
			throw rpc_exception(err);
		}
		U* y = byid->get_rawptr_byid(objid.t);
		x = dynamic_cast<T*>(y);
		if (0 == x)
			incompitible_class_cast(y, T::s_getClassName());
	}
	template<class T> void load(T*& x, GlobaleScope*) { load(x, m_globale); }
	template<class T> void load(T*& x, SessionScope*) { load(x, m_session); }

public:
	server_object_input( AccessByNameID<GlobaleScopePtr>* globale
					   , AccessByNameID<SessionScopePtr>* session
					   , Input* input)
	: m_globale(globale), m_session(session), p(input) {}

	Input& get() const { return *p; }

	template<class T> server_object_input& operator &(rpc_in<T> x) { *p >> x.r; return *this; }
	template<class T> server_object_input& operator>>(rpc_in<T> x) { *p >> x.r; return *this; }

	template<class T> server_object_input& operator &(rpc_out<T> x) { return *this; }
	template<class T> server_object_input& operator>>(rpc_out<T> x) { return *this; }

	template<class T> server_object_input& operator &(rpc_inout<T> x) { *p >> x.r; return *this; }
	template<class T> server_object_input& operator>>(rpc_inout<T> x) { *p >> x.r; return *this; }

	template<class T> server_object_input& operator &(T*& x) { load(x, x);	return *this; }
	template<class T> server_object_input& operator>>(T*& x) { load(x, x);	return *this; }

	template<class T> server_object_input& operator &(boost::intrusive_ptr<T>& x)
	{
		return *this >> x;
	}
	template<class T> server_object_input& operator>>(boost::intrusive_ptr<T>& x)
	{
		T* y = 0;
		load(y, y);
		x.reset(y);
		return *this;
	}
};

template<class Output> class server_object_output
{
	AccessByNameID<GlobaleScopePtr>* m_globale;
	AccessByNameID<SessionScopePtr>* m_session;
	Output* p;

	template<class T, class U>
	void save(T* x, AccessByNameID<boost::intrusive_ptr<U> >* byid, const char* szBaseName)
	{
		// do nothing...
		typename T::SFINAE_ro_self_t* for_check = 0;
		(void)(for_check);
// 		var_size_t objid(x->getID());
// 		if (!byid->is_valid(objid.t))
// 		{
// 			string_appender<> oss;
// 			oss << "not found " << szBaseName << ": " << x->getClassName() << " :id=" << objid.t;
// 			throw rpc_exception(oss.str());
// 		}
// 		*p << objid;
	}
	template<class T> void save(T* x, GlobaleScope*) { save(x, m_globale, "GlobaleScope"); }
	template<class T> void save(T* x, SessionScope*) { save(x, m_session, "SessionScope"); }

public:
	server_object_output( AccessByNameID<GlobaleScopePtr>* globale
						, AccessByNameID<SessionScopePtr>* session
						, Output* output)
	: m_globale(globale), m_session(session), p(output) {}

	void flush() { p->flush(); }

	Output& get() const { return *p; }

	template<class T> server_object_output& operator &(rpc_in<T> x) { return *this; }
	template<class T> server_object_output& operator<<(rpc_in<T> x) { return *this; }

	template<class T> server_object_output& operator &(rpc_out<T> x) { *p << x.r; return *this; }
	template<class T> server_object_output& operator<<(rpc_out<T> x) { *p << x.r; return *this; }

	template<class T> server_object_output& operator &(rpc_inout<T> x) { *p << x.r; return *this; }
	template<class T> server_object_output& operator<<(rpc_inout<T> x) { *p << x.r; return *this; }

	template<class T> server_object_output& operator &(const T& x) { return *this << x; }
	template<class T> server_object_output& operator<<(T* const& x) // only for derived from remote_object
	{
		save(x, x);
		return *this;
	}

	template<class T> server_object_output& operator &(const boost::intrusive_ptr<T>& x) { return *this << x; }
	template<class T> server_object_output& operator<<(const boost::intrusive_ptr<T>& x) // only for derived from remote_object
	{
		save(x.get(), x.get());
		return *this;
	}
};


} } // namespace::nark::rpc


#endif // __nark_rpc_server_io_h__
