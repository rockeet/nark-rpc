#include <boost/version.hpp>

#if BOOST_VERSION < 103301
# include <boost/limits.hpp>
#else
# include <boost/detail/endian.hpp>
#endif

#if !defined(BOOST_BIG_ENDIAN) && !defined(BOOST_LITTLE_ENDIAN)
	#error must define byte endian
#endif

#include "MessageInputStream.hpp"
#include <nark/io/byte_swap.hpp>
#include <algorithm>
#include <assert.h>

namespace nark { namespace rpc {


void MessageHeader::convert()
{
#ifdef BOOST_LITTLE_ENDIAN
	length = byte_swap(length);
	seqid = byte_swap(seqid);
	partid = byte_swap(partid);
#endif
}

MessageInputStream::MessageInputStream(IInputStream* stream)
: stream(stream)
{
}

MessageInputStream::~MessageInputStream()
{
}

void MessageInputStream::read_hello()
{

}

void MessageInputStream::read_until(void* data, size_t length)
{

}

size_t MessageInputStream::read(void* vbuf, size_t length)
{
	if (m_readed == curheader.length)
	{
		m_readed = 0;
		MessageHeader h2;
		read_until(&h2, sizeof(h2));
		h2.convert();
		if (0 == h2.length)
		{
			throw RequestCanceledException();
		}
		if (h2.seqid == curheader.seqid)
		{
			assert(MSG_HEADER_GET_PARTID(h2.partid) == MSG_HEADER_GET_PARTID(curheader.partid) + 1);
		}
		curheader = h2;
	}
//	int ptype = MSG_HEADER_GET_TYPE(curheader.partid);
	using namespace std;
	size_t n = min(size_t(curheader.length), length);
	read_until(m_readed + (char*)vbuf, n);
	m_readed += n;

	return n;
}

bool MessageInputStream::eof()
{
	return false;
}

} } // namespace nark::rpc
