
// sample usage...
// echo.h

BEGIN_RPC_INTERFACE(Echo, GlobaleScope)
	RPC_ADD_MF(echo)
END_RPC_ADD_MF()
	RPC_DECLARE_MF_D(echo, (const std::string& msg, std::string* y))
END_RPC_INTERFACE()

