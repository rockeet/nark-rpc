#define ArgCount BOOST_PP_ITERATION()
#define TupleArg(z, n, _)  typename rpc_arg<Arg##n>::type

template<BOOST_PP_ENUM_PARAMS(ArgCount, class Arg)>
class server_fun_stub_call<rpc_ret_t (*)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg))>
{
public:
	typedef BOOST_PP_IF(ArgCount, BOOST_PP_IDENTITY(typename), BOOST_PP_EMPTY)() boost::mpl::vector<BOOST_PP_ENUM_PARAMS(ArgCount, Arg)>::type argvec_t;
	typedef boost::tuples::tuple<BOOST_PP_ENUM(ArgCount, TupleArg, ~)>	arglist_type;
	typedef rpc_ret_t (*function_t)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg));

	template<int N>
	typename rpc_arg<typename boost::mpl::at_c<argvec_t, N>::type>::reference
		get_arg(arglist_type& args) const
	{
		return rpc_arg<typename boost::mpl::at_c<argvec_t, N>::type>::get(args.template get<N>());
	}

#define GetFunArg(z, n, _) get_arg<n>(args)
	rpc_ret_t do_call(function_t fun, arglist_type& args) const
	{
		return fun(BOOST_PP_ENUM(ArgCount, GetFunArg, ~));
	}
#undef GetFunArg
};

template<class ThisType BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, class Arg)>
class server_fun_stub_call<rpc_ret_t (ThisType::*)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg))>
{
public:
	typedef boost::tuples::tuple<
		typename rpc_arg<ThisType>::type
		BOOST_PP_ENUM_TRAILING(ArgCount, TupleArg, ~)
		>
	arglist_type;
	typedef typename boost::mpl::vector<ThisType BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, Arg)>::type argvec_t;

	typedef rpc_ret_t (ThisType::*function_t)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg));

	template<int N>
	typename rpc_arg<typename boost::mpl::at_c<argvec_t, N>::type>::reference
		get_arg(arglist_type& args) const
	{
		return rpc_arg<typename boost::mpl::at_c<argvec_t, N>::type>::get(args.template get<N>());
	}

#define GetFunArg(z, n, _) get_arg<n+1>(args)
	rpc_ret_t do_call(function_t fun, arglist_type& args) const
	{
		return (get_arg<0>(args).*fun)(BOOST_PP_ENUM(ArgCount, GetFunArg, ~));
	}
#undef GetFunArg
};


#undef ArgCount
#undef TupleArg
