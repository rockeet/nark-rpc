#ifndef __nark_rpc_arg_traits_h__
#define __nark_rpc_arg_traits_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/**
 @file 推导rpc参数

 将函数原型中的参数类型声明推导为 rpc_in/rpc_out/rpc_inout 类型
 -# T          推导为 rpc_in<T>
 -# const T    推导为 rpc_in<T>
 -# const T&   推导为 rpc_in<T>
 -# const T*   推导为 rpc_in<T>
 -# T&		   推导为 rpc_inout<T>
 -# T*		   推导为 rpc_out<T>
 -# remote_object 的派生类另外处理

 将函数原型推导为 arglist_val/arglist_ref 类型
 */
#include <boost/type_traits.hpp>

namespace nark { namespace rpc {

template<class T>
struct ArgVal_Aux // T = in, pass by value
{
	typedef T val_t; // used for hold the arg
	typedef rpc_in<T> xref_t; // used for recognize in/out/inout

	//! used by server side only
	static T& deref(rpc_in<T> x) { return x.r; }
	static T& val(rpc_in<T> x) { return x.r; }
};

template<class T>
struct ArgVal_Aux<const T> // T = in, pass by value
{
	typedef T val_t;
	typedef rpc_in<T> xref_t;
	static T& deref(rpc_in<T> x) { return x.r; }
	static T& val(rpc_in<T> x) { return x.r; }
};

template<class T>
struct ArgVal_Aux<T&> // T& = inout
{
	typedef T val_t;
	typedef rpc_inout<T> xref_t;
	static T& deref(rpc_inout<T> x) { return x.r; }
	static T& val(rpc_inout<T> x) { return x.r; }
};

template<class T>
struct ArgVal_Aux<const T&> // T& = in
{
	typedef T val_t;
	typedef rpc_in<T> xref_t;
	static T& deref(rpc_in<T> x) { return x.r; }
	static T& val(rpc_in<T> x) { return x.r; }
};

template<class T>
struct ArgVal_Aux<T*> // T* = out
{
	typedef T val_t;
	typedef rpc_out<T> xref_t;
	static T* deref(rpc_out<T> x) { return &x.r; }
	static T& val(rpc_out<T> x) { return x.r; }
};

template<class T>
struct ArgVal_Aux<const T*> // const T* = in
{
	typedef T val_t;
	typedef rpc_in<T> xref_t;
	static T* deref(rpc_in<T> x) { return &x.r; }
	static T& val(rpc_in<T> x) { return x.r; }
};

template<class T>
struct ArgVal_Aux<rpc_in<T> > // explicit rpc_in
{
	typedef T val_t;
	typedef rpc_in<T>xref_t;
	static rpc_in<T> deref(rpc_in<T> x) { return x; }
	static val_t& val(rpc_in<T> x) { return x.r; }
};
template<class T>
struct ArgVal_Aux<rpc_out<T> > // explicit rpc_out
{
	typedef T val_t;
	typedef rpc_out<T>xref_t;
	static rpc_out<T> deref(rpc_out<T> x) { return x; }
	static val_t& val(rpc_out<T> x) { return x.r; }
};
template<class T>
struct ArgVal_Aux<rpc_inout<T> > // explicit rpc_inout
{
	typedef T val_t;
	typedef rpc_inout<T>xref_t;
	static rpc_inout<T> deref(rpc_inout<T> x) { return x; }
	static val_t& val(rpc_inout<T> x) { return x.r; }
};

template<class T>
struct ArgVal
{
	typedef typename boost::remove_const<
			typename boost::remove_pointer<
			typename boost::remove_reference<T
		>::type>::type>::type raw_type;

	typedef typename boost::is_base_of<remote_object, raw_type>::type
		is_remote;

	typedef typename boost::mpl::if_
		< is_remote
	//	, IF_RPC_SERVER(boost::intrusive_ptr<raw_type>, raw_type*)
		, boost::intrusive_ptr<raw_type>
		, typename ArgVal_Aux<T>::val_t
		>::type val_t; //!< used by server side only

	typedef typename boost::mpl::if_
		< is_remote
	//	, IF_RPC_SERVER(boost::intrusive_ptr<raw_type>, raw_type*)&
		, boost::intrusive_ptr<raw_type>&
		, typename ArgVal_Aux<T>::xref_t
		>::type xref_t;

	//! used by server side only
	static T deref(xref_t x) { return deref0(x, is_remote()); }
	static val_t& val(xref_t x) { return val0(x, is_remote()); }
private:
	static T deref0(xref_t x, boost::mpl::true_) { return x; }
	static T deref0(xref_t x, boost::mpl::false_) { return ArgVal_Aux<T>::deref(x); }
	static xref_t val0(xref_t x, boost::mpl::true_) { return x; }
	static val_t& val0(xref_t x, boost::mpl::false_) { return ArgVal_Aux<T>::val(x); }
};
template<class T> inline void argval_sync(T*& x, T*& y) { x = y; }
template<class T> inline void argval_sync(boost::intrusive_ptr<T>& x, boost::intrusive_ptr<T>& y) { x = y; }
template<class V, class R> inline void argval_sync(const V&, const R&) {  }

template<class Function> class arglist_ref;
template<class Function> class arglist_val;

#include <boost/preprocessor/iterate.hpp>
#define BOOST_PP_ITERATION_LIMITS (0, 20)
#define BOOST_PP_FILENAME_1       <nark/rpc/pp_arglist_type.hpp>
#include BOOST_PP_ITERATE()

} } // namespace::nark::rpc



#endif // __nark_rpc_arg_traits_h__
