// sample usage...
// ifile.h

BEGIN_RPC_INTERFACE(FileObj, SessionScope)
	RPC_ADD_MF(open)
	RPC_ADD_MF(read)
	RPC_ADD_MF(write)
	RPC_ADD_MF(close)
END_RPC_ADD_MF()
	RPC_DECLARE_MF(open, (const std::string& fname, const std::string& mode))
	RPC_DECLARE_MF(read, (vector<char>* buffer, uint32_t length))
	RPC_DECLARE_MF(write, (const vector<char>& buffer, uint32_t* length))
	RPC_DECLARE_MF(close, ())
END_RPC_INTERFACE()
