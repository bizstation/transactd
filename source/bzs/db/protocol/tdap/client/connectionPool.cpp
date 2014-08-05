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

#if (__BCPLUSPLUS__)
#   ifdef _WIN64
#		ifdef _RTLDLL
#	    	pragma comment(lib, "libboost_thread-bcb64-mt-1_50.a")
#   	 	pragma comment(lib, "libboost_system-bcb64-mt-1_50.a")
#    		pragma comment(lib, "libboost_chrono-bcb64-mt-1_50.a")
#   	else
#	    	pragma comment(lib, "libboost_thread-bcb64-mt-s-1_50.a")
#   	 	pragma comment(lib, "libboost_system-bcb64-mt-s-1_50.a")
#    		pragma comment(lib, "libboost_chrono-bcb64-mt-s-1_50.a")
#   	endif
		namespace boost{void tss_cleanup_implemented(){}}
#   else
#		ifdef _RTLDLL
#	    	pragma comment(lib, "libboost_thread-bcb-mt-1_39.lib")
#   	else
#	    	pragma comment(lib, "libboost_thread-bcb-mt-s-1_39.lib")
#   	endif
		namespace boost{extern "C" void tss_cleanup_implemented(){}}
#   endif
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


