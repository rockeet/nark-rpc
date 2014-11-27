/* vim: set tabstop=4 : */
#define ArgCount BOOST_PP_ITERATION()
template<class ThisType BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, class Arg)>
class server_packet<rpc_ret_t (ThisType::*)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg))>
	: public server_packet_base
{
public:
	typedef rpc_ret_t (ThisType::*function_t)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg));
	typedef arglist_val<function_t> aval_t;
	typedef arglist_ref<function_t> aref_t;
	aval_t argvals;
#define PP_ArgListDeRef(z, n, d) ArgVal<Arg##n>::deref(argvals.d##n)
	void invoke_f(function_t fun)
	{
		this->retv = (argvals.self.get()->*fun)(BOOST_PP_ENUM(ArgCount, PP_ArgListDeRef, a));
	}
#undef PP_ArgListDeRef
};
#undef ArgCount
