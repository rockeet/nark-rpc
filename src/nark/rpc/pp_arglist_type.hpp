/* vim: set tabstop=4 : */
#define ArgCount BOOST_PP_ITERATION()
#define PP_ArgListRef(z, n, d)  typename ArgVal<Arg##n>::xref_t a##n d
#define PP_ArgListVal(z, n, d)  typename ArgVal<Arg##n>::val_t a##n d
#define PP_ArgListDeRef(z, n, d)  d##n.r
#define PP_InitMemList(z, n, d) a##n(d##n)
#define PP_InitMemList2(z, n, d) a##n(ArgVal<Arg##n>::val(d##n))
#define PP_InitMemList3(z, n, d) argval_sync(a##n,d##n);

template<class ThisType BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, class Arg)>
struct arglist_val<rpc_ret_t (ThisType::*)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg))>
{
//	IF_RPC_SERVER(boost::intrusive_ptr<ThisType>, ThisType*) self;
	boost::intrusive_ptr<ThisType> self;
	BOOST_PP_REPEAT(ArgCount, PP_ArgListVal, ;)

	arglist_val(//IF_RPC_SERVER(boost::intrusive_ptr<ThisType>, ThisType*) self
		boost::intrusive_ptr<ThisType> self
		BOOST_PP_ENUM_TRAILING(ArgCount, PP_ArgListRef, BOOST_PP_EMPTY()))
	: self(self)BOOST_PP_ENUM_TRAILING(ArgCount, PP_InitMemList, a)
	{}
	template<class ArgListRef>
	arglist_val(boost::mpl::true_ placeHolder, ArgListRef& y)
	: self(y.self)BOOST_PP_ENUM_TRAILING(ArgCount, PP_InitMemList2, y.a)
	{}
	arglist_val() {}

	template<class ArgListRef> void sync(ArgListRef& refs) {
	//	assert(0); // now disabled
		self = refs.self;
		BOOST_PP_REPEAT(ArgCount, PP_InitMemList3, refs.a)
	}
};
template<class ThisType BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, class Arg)>
struct arglist_ref<rpc_ret_t (ThisType::*)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg))>
{
//	IF_RPC_SERVER(boost::intrusive_ptr<ThisType>, ThisType*) &self;
	boost::intrusive_ptr<ThisType> self;
	BOOST_PP_REPEAT(ArgCount, PP_ArgListRef, ;)

	arglist_ref(//IF_RPC_SERVER(boost::intrusive_ptr<ThisType>, ThisType*) &self
		boost::intrusive_ptr<ThisType> self
		BOOST_PP_ENUM_TRAILING(ArgCount, PP_ArgListRef, BOOST_PP_EMPTY()))
	: self(self)BOOST_PP_ENUM_TRAILING(ArgCount, PP_InitMemList, a)
	{}
	template<class ArgListVal>
	arglist_ref(boost::mpl::true_ placeHolder, ArgListVal& y)
	: self(y.self)BOOST_PP_ENUM_TRAILING(ArgCount, PP_InitMemList, y.a)
	{}
#define PP_IO_ArgList(z, n, load_save) load_save a##n
	template<class DataIO> void load(DataIO& dio) {
		dio >> self BOOST_PP_REPEAT(ArgCount, PP_IO_ArgList, >>);
	}
	template<class DataIO> void save(DataIO& dio) const {
		dio << self BOOST_PP_REPEAT(ArgCount, PP_IO_ArgList, <<);
	}
#undef PP_IO_ArgList
};

#undef PP_InitMemList3
#undef PP_InitMemList2
#undef PP_InitMemList
#undef PP_ArgListDeRef
#undef PP_ArgListVal
#undef PP_ArgListRef
#undef ArgCount


