/*=================================================================
   Copyright (C) 2014 BizStation Corp All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.
=================================================================*/
#include "connectionPool.h"


#ifdef __BCPLUSPLUS__
#   ifdef _WIN64
#		define BZS_LINK_BOOST_SYSTEM
#		define BZS_LINK_BOOST_THREAD
#		define BZS_LINK_BOOST_CHRONO
		namespace boost{void tss_cleanup_implemented(){}}
#	else
#		define BZS_LINK_BOOST_THREAD
		namespace boost{extern "C" void tss_cleanup_implemented(){}}
#	endif
#	include <bzs/env/boost_bcb_link.h>
#endif


namespace bzs
{
namespace db
{
namespace protocol
{
namespace tdap
{
namespace client
{

stdCconnectionPool cpool;

short __STDCALL dllUnloadCallbackFunc()
{
	cpool.reset(0);
	cpool.m_regitfunc = NULL;
	return 0;
}

void releaseConnection(stdDbmCconnectionPool* pool)
{
	pool->releaseOne();
}






}//namespace client
}//namespace tdap
}//namespace protocol
}//namespace db
}//namespace bzs


