
//////////////////////////////////////////////////////////////////////////
// sample usage...
// test.h

typedef std::vector<unsigned> vint_vec;

class AsyncInterface : public GlobaleScope
{
public:
	BEGIN_RPC_ADD_MF(AsyncInterface)
		RPC_ADD_MF(get_val)
		RPC_ADD_MF(get_len)
		RPC_ADD_MF(squareVec)
		RPC_ADD_MF(multiVec)
	END_RPC_ADD_MF()

	RPC_DECLARE_MF(get_val, (rpc_in<int> x))
	RPC_DECLARE_MF(get_len, (const std::string& x))
	RPC_DECLARE_MF(squareVec, (vint_vec& x))
	RPC_DECLARE_MF(multiVec, (vint_vec& z, vint_vec& x, vint_vec& y))
};
RPC_TYPEDEF_PTR(AsyncInterface);

// more convenient way than AsyncInterface...
BEGIN_RPC_INTERFACE(SampleRPC_Interface2, SessionScope)
	RPC_ADD_MF(get_val)
	RPC_ADD_MF(get_len)
END_RPC_ADD_MF()
	RPC_DECLARE_MF(get_val, (rpc_in<int> x))
	RPC_DECLARE_MF(get_len, (const std::string& x))
END_RPC_INTERFACE()
