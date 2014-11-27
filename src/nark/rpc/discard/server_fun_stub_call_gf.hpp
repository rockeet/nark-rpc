/* vim: set tabstop=4 : */

template<BOOST_PP_ENUM_PARAMS(ArgCount, class Arg)>
class server_fun_stub_call<rpc_ret_t (*)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg))>
{
public:
	typedef boost::tuples::tuple<
		BOOST_PP_ENUM(ArgCount, TupleArgVal, ~)
		>
	arglist_val;
	typedef boost::tuples::tuple<
		BOOST_PP_ENUM(ArgCount, TupleArgRef, ~)
		>
	arglist_ref;
	typedef rpc_ret_t (*function_t)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg));

#if ArgCount == 0
	rpc_ret_t call(function_t fun, arglist_val& args) const { return fun(); }
	template<class DataIO> void load(DataIO& dio, arglist_val& args) { }
	template<class DataIO> void save(DataIO& dio, arglist_val& args) { }
#else
#define GetTupleArgListRef(z, n, _) get<n>(refs)
#define GetTupleArgListVal(z, n, _) ArgVal<Arg##n>::deref(get<n>(args))
	rpc_ret_t call(function_t fun, arglist_val& args) const
	{
		using boost::tuples::get;
		arglist_ref refs(BOOST_PP_ENUM(ArgCount, GetTupleArgListVal, ~));
		return fun(BOOST_PP_ENUM(ArgCount, GetTupleArgListRef, ~));
	}
	template<class DataIO>
	void load(DataIO& dio, arglist_val& args)
	{
		using boost::tuples::get;
		arglist_ref refs(BOOST_PP_ENUM(ArgCount, GetTupleArgListVal, ~));
		dio >> refs;
	}
	template<class DataIO>
	void save(DataIO& dio, arglist_val& args)
	{
		using boost::tuples::get;
		arglist_ref refs(BOOST_PP_ENUM(ArgCount, GetTupleArgListVal, ~));
		dio << refs;
	}
#undef GetTupleArgListVal
#undef GetTupleArgListRef
#endif
};


