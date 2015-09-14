#ifndef __stdafx_h__
#define __stdafx_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <stdio.h>
#include <nark/inet/SocketStream.hpp>

#ifdef EMPLOYEE_USE_BDB
#include <db_cxx.h>
#include <nark/bdb_util/dbmap.hpp>
#include <nark/bdb_util/kmapdset.hpp>
#else
#include <map>
#include <vector>
#endif

#endif // __stdafx_h__
