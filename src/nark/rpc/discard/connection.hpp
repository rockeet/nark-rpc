#ifndef __nark_rpc_connection_h__
#define __nark_rpc_connection_h__

#include <nark/io/IStream.hpp>

namespace nark { namespace rpc {

class Connection
{
	IInputStream*  is;
	IOutputStream* os;

public:

};


} } // namespace nark::rpc

#endif // __nark_rpc_connection_h__


