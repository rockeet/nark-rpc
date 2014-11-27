/* vim: set tabstop=4 : */
#define ArgCount BOOST_PP_ITERATION()
#define ArgDeclare(z, n, d)   Arg##n a##n d
#define PP_ArgListRef(z, n, d)  typename ArgVal<Arg##n>::xref_t a##n d
#define PP_ArgListVal(z, n, d)  typename ArgVal<Arg##n>::val_t a##n d

#define PP_ArgListDeRef(z, n, d)  ArgVal<Arg##n>::deref(d##n)
//#define PP_ArgListGetVal(z, n, d)  ArgVal<Arg##n>::val(d##n)

#define FunctionType rpc_ret_t (ThisType::*)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg))

template<class Client, class ThisType BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, class Arg)>
class client_packet
  < Client
  , arglist_ref<FunctionType>
  , FunctionType
  >
: public client_packet_io
  < Client
  , FunctionType
  , void (ThisType::*)(const client_packet_base&BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, Arg))
  >
{
public:
	typedef arglist_ref<FunctionType> arglist_ref_t;
	arglist_ref_t  refs;

	client_packet(ThisType* self BOOST_PP_ENUM_TRAILING(ArgCount, PP_ArgListRef, BOOST_PP_EMPTY()))
		: refs(*self BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, a))
	{}

	explicit client_packet(arglist_ref_t& argrefs) : refs(argrefs) { }

	rpc_ret_t call_byid  (rpc_client_basebase* client) { return this->t_call_byid  (client, refs); }
	rpc_ret_t call_byname(rpc_client_basebase* client) { return this->t_call_byname(client, refs); }
	void send_id_args(rpc_client_basebase* client) { return this->t_send_id_args(client, refs); }
	void send_nm_args(rpc_client_basebase* client) { return this->t_send_nm_args(client, refs); }
	void read_args(rpc_client_basebase* client) { return this->t_read_args(client, refs); }

	virtual void on_return() {
		assert(NULL != this->on_ret); // on_ret is a function pointer
		rpc_client_basebase* client = refs.self->get_ext_ptr();
		this->t_read_args(client, refs);
		(refs.self.get()->*this->on_ret)(*this BOOST_PP_ENUM_TRAILING(ArgCount, PP_ArgListDeRef, refs.a));
	}
};

template<class Client, class ThisType BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, class Arg)>
class client_packet
  < Client
  , arglist_ref<FunctionType> *
  , FunctionType
  >
: public client_packet_io
  < Client
  ,	FunctionType
  , void (ThisType::*)(const client_packet_base&BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, Arg))
  >
{
public:
	typedef arglist_ref<FunctionType> arglist_ref_t;
	arglist_ref_t* refs;

	explicit client_packet(void* refs) : refs((arglist_ref_t*)refs) {}

	rpc_ret_t call_byid  (rpc_client_basebase* client) { return this->t_call_byid  (client, *refs); }
	rpc_ret_t call_byname(rpc_client_basebase* client) { return this->t_call_byname(client, *refs); }
	void send_id_args(rpc_client_basebase* client) { return this->t_send_id_args(client, *refs); }
	void send_nm_args(rpc_client_basebase* client) { return this->t_send_nm_args(client, *refs); }
	void read_args(rpc_client_basebase* client) { return this->t_read_args(client, *refs); }
	virtual void on_return() {
		assert(NULL != this->on_ret); // on_ret is a function pointer
		rpc_client_basebase* client = refs->self->get_ext_ptr();
		this->t_read_args(client, *refs);
		(refs->self.get()->*this->on_ret)(*this BOOST_PP_ENUM_TRAILING(ArgCount, PP_ArgListDeRef, refs->a));
	}
};

// hold args references
template<class Client, class ThisType BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, class Arg)>
class client_packet
  < Client
  , arglist_val<FunctionType>
  , FunctionType
  >
: public client_packet_io
  < Client
  , FunctionType
  , void (ThisType::*)(const client_packet_base&BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, Arg))
  >
{
public:
	typedef arglist_ref<FunctionType> arglist_ref_t;
	typedef arglist_val<FunctionType> arglist_val_t;
	arglist_val_t  vals;

	client_packet(ThisType* self BOOST_PP_ENUM_TRAILING(ArgCount, ArgDeclare, BOOST_PP_EMPTY()))
		: vals(*self BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, a))
	{}
	explicit client_packet(arglist_ref_t& argrefs)
		: vals(boost::mpl::true_(), argrefs)
	{}

	rpc_ret_t call_byid(rpc_client_basebase* client) {
		arglist_ref_t refs(boost::mpl::true_(), vals);
		return this->t_call_byid(client, refs);
	}
	rpc_ret_t call_byname(rpc_client_basebase* client) {
		arglist_ref_t refs(boost::mpl::true_(), vals);
		return this->t_call_byname(client, refs);
	}
	void send_id_args(rpc_client_basebase* client) {
		arglist_ref_t refs(boost::mpl::true_(), vals);
		return this->t_send_id_args(client, refs);
	}
	void send_nm_args(rpc_client_basebase* client) {
		arglist_ref_t refs(boost::mpl::true_(), vals);
		return this->t_send_nm_args(client, refs);
	}
	void read_args(rpc_client_basebase* client) {
		arglist_ref_t refs(boost::mpl::true_(), vals);
		return this->t_read_args(client, refs);
	}
	virtual void on_return() {
		assert(NULL != this->on_ret); // on_ret is a function pointer
		arglist_ref_t refs(boost::mpl::true_(), vals);
		rpc_client_basebase* client = vals.self->get_ext_ptr();
		this->t_read_args(client, refs);
		(refs.self.get()->*this->on_ret)(*this BOOST_PP_ENUM_TRAILING(ArgCount, PP_ArgListDeRef, refs.a));
	}
};

//#undef PP_ArgListGetVal
#undef PP_ArgListDeRef

template<class ThisType BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, class Arg)>
class client_stub_ref<FunctionType>
{
	typedef void (ThisType::*myfun_t)(const client_packet_base&BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, Arg));
	typedef arglist_ref<FunctionType> arglist_ref_t;
	typedef arglist_val<FunctionType> arglist_val_t;
	typedef client_packet_fun<myfun_t> packet_fun_t;
	client_stub_i* m_meta;
	ThisType* m_self;
	myfun_t on_return;

public:
	client_stub_ref() : m_meta(0), m_self(0), on_return(0) {}

	template<class ThisDerived>
	void set_async_callback(void (ThisDerived::*pf)(const client_packet_base&
							BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, Arg)))
	{
		if (false)
		{	// only do compile-time check, this sentence will be eliminated in obj-code
			ThisType* pbase = (ThisDerived*)NULL;
			(void)pbase;
		}
		on_return = static_cast<myfun_t>(pf);
	}

	void bind(ThisType* self, client_stub_i* impl) {
		m_meta = impl;
		m_self = self;
	}

	rpc_ret_t operator()(BOOST_PP_ENUM(ArgCount, ArgDeclare, BOOST_PP_EMPTY())) {
		arglist_ref_t argrefs(m_self BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, a));
		rpc_client_basebase* client = m_self->get_ext_ptr();
		// call_byid will fall back to by name if callid is zero
		return m_meta->call_byid(client, &argrefs);
	}

	rpc_ret_t byname(BOOST_PP_ENUM(ArgCount, ArgDeclare, BOOST_PP_EMPTY())) {
		arglist_ref_t argrefs(m_self BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, a));
		rpc_client_basebase* client = m_self->get_ext_ptr();
		// always call by name
		return m_meta->call_byname(client, &argrefs);
	}

	void async(BOOST_PP_ENUM(ArgCount, ArgDeclare, BOOST_PP_EMPTY())) {
		arglist_ref_t args(m_self BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, a));
		rpc_client_basebase* client = m_self->get_ext_ptr();
		boost::intrusive_ptr<client_packet_base> packet = m_meta->valpacket_create(&args);
		packet_fun_t* p = (packet_fun_t*)(packet.get());
		assert(NULL != on_return);
		p->on_ret = on_return;
		packet->stub = this->m_meta;
		packet->how_call = rpc_call_asynch_ordered;
		if (m_self->stub->callid) // prefer by id
			m_meta->send_id_args(client, packet.get());
		else // fall back to by name
			m_meta->send_nm_args(client, packet.get());
		packet->async_packet(client);
	}
	void async_byname(BOOST_PP_ENUM(ArgCount, ArgDeclare, BOOST_PP_EMPTY())) {
		arglist_ref_t args(m_self BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, a));
		boost::intrusive_ptr<client_packet_base> packet = m_meta->valpacket_create(&args);
		rpc_client_basebase* client = m_self->get_ext_ptr();
		packet_fun_t* p = (packet_fun_t*)(packet.get());
		assert(on_return);
		p->on_ret = on_return;
		packet->stub = this->m_meta;
		packet->how_call = rpc_call_asynch_ordered;
		m_meta->send_nm_args(client, packet.get());
		packet->async_packet(client);
	}

	rpc_ret_t reap(BOOST_PP_ENUM(ArgCount, ArgDeclare, BOOST_PP_EMPTY())) {
		return this->retv;
	}
};

#undef FunctionType
#undef PP_ArgListVal
#undef PP_ArgListRef
#undef ArgDeclare
#undef ArgCount

