/* vim: set tabstop=4 : */

template<BOOST_PP_ENUM_PARAMS(ArgCount, class Arg)>
class client_fun_stub<rpc_ret_t (*)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg))>
{
	client_fun_stub_interface* m_impl;
	void* m_client;
	typedef boost::tuples::tuple<BOOST_PP_ENUM(ArgCount, TupleArgListRef, ~)> arglist_ref;

public:
	client_fun_stub() : m_impl(0) {}

	void bind(void* client, client_fun_stub_interface* impl)
	{
		m_client = client;
		m_impl = impl;
	}

	rpc_ret_t operator()(BOOST_PP_ENUM(ArgCount, ArgDeclare, ~))
	{
		arglist_ref args BOOST_PP_LPAREN_IF(ArgCount) BOOST_PP_ENUM_PARAMS(ArgCount, a) BOOST_PP_RPAREN_IF(ArgCount);
		return m_impl->do_call(m_client, &args);
	}

	void async(BOOST_PP_ENUM(ArgCount, ArgDeclare, ~))
	{
		arglist_ref args BOOST_PP_LPAREN_IF(ArgCount) BOOST_PP_ENUM_PARAMS(ArgCount, a) BOOST_PP_RPAREN_IF(ArgCount);
		m_impl->send_args(m_client, &args, rpc_call_asynch_ordered);
	}

	rpc_ret_t reap(BOOST_PP_ENUM(ArgCount, ArgDeclare, ~))
	{
		arglist_ref args BOOST_PP_LPAREN_IF(ArgCount) BOOST_PP_ENUM_PARAMS(ArgCount, a) BOOST_PP_RPAREN_IF(ArgCount);
		rpc_ret_t retv;
		m_impl->load_args(m_client, &retv, &args);
		return retv;
	}
};

template<BOOST_PP_ENUM_PARAMS(ArgCount, class Arg) BOOST_PP_COMMA_IF(ArgCount) class Client>
class client_fun_stub_impl<rpc_ret_t (*)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg)), Client>
	: public client_fun_stub_interface
{
	typedef boost::tuples::tuple<BOOST_PP_ENUM(ArgCount, TupleArgListRef, ~)> arglist_ref;
public:
	explicit client_fun_stub_impl(const std::string& name) : client_fun_stub_interface(name) {}

	rpc_ret_t do_call(void* vpclient, void* pArgs)
	{
		rpc_ret_t retv;
		send_args(vpclient, pArgs, rpc_call_synch);
		load_args(vpclient, &retv, pArgs);
		return retv;
	}
//#define TupleArgIO(z, n, io)  io get<n>(args)
	void send_args(void* vpclient, void* pArgs, unsigned how_call)
	{
		using boost::tuples::get;
		Client* client = (Client*)(vpclient);
		arglist_ref& args = *(arglist_ref*)(pArgs);
		if (m_callid.t)
			client->m_co_output.get() << m_callid;
		else {
			client->m_co_output.get() << var_uint32_t(0);
			client->m_co_output.get() << m_name;
			m_callid.t = client->add_stub(this);
		}
		client->m_co_output.get() << var_uint32_t(how_call);
		client->m_co_output args;//BOOST_PP_REPEAT(ArgCount, TupleArgIO, <<);
		client->m_co_output.flush();
	}
	void load_args(void* vpclient, rpc_ret_t* retv, void* pArgs)
	{
		using boost::tuples::get;
		Client* client = (Client*)(vpclient);
		arglist_ref& args = *(arglist_ref*)(pArgs);
		client->m_co_input.get() >> *retv;
		client->m_co_input >> args;//BOOST_PP_REPEAT(ArgCount, TupleArgIO, >>);
	}
//#undef TupleArgIO
};


