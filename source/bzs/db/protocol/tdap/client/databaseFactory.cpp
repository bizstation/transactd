/*=================================================================
   Copyright (C) 2013 BizStation Corp All rights reserved.

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

#pragma hdrstop

#include "database.h"
#include <bzs/rtl/exception.h>
//---------------------------------------------------------------------------


#pragma package(smart_init)

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


database* database::create()
{
	try
	{
		return new database();
	}
	catch( bzs::rtl::exception& /*e*/){}
	return NULL;
}

}//namespace client
}//namespace tdap
}//namespace protocol
}//namespace db
}//namespace bzs


#undef USETLS
#if ((defined(_WIN32) && _MSC_VER) || (__APPLE__ && !defined(__BCPLUSPLUS__)))
#define USETLS
#endif

/* At MSVC use __thread before Windows Vista,  After the DLL is loaded with LoadLibrary
   then  it causes system failure.
  
*/
#ifdef USETLS
tls_key g_tlsiID_SC1;
tls_key g_tlsiID_SC2;
tls_key g_tlsiID_SC3;

void initTlsThread()
{
	tls_setspecific(g_tlsiID_SC1, new wchar_t[256]);
	tls_setspecific(g_tlsiID_SC2, new wchar_t[45]);
	tls_setspecific(g_tlsiID_SC3, new wchar_t[45]);
}

void cleanupTls()
{
	 delete (char*)tls_getspecific(g_tlsiID_SC1);
	 delete (char*)tls_getspecific(g_tlsiID_SC2);
	 delete (char*)tls_getspecific(g_tlsiID_SC3);
}

#ifdef __APPLE__

#include <pthread.h>

void __attribute__ ((constructor)) onLoadLibrary(void);
void __attribute__ ((destructor)) onUnloadLibrary(void);

void onLoadLibrary(void)
{
	 pthread_key_create(&g_tlsiID_SC1, NULL);
	 pthread_key_create(&g_tlsiID_SC2, NULL);
	 pthread_key_create(&g_tlsiID_SC3, NULL);
}

void onUnloadLibrary(void)
{
	 cleanupTls();
	 pthread_key_delete(g_tlsiID_SC1); 
	 pthread_key_delete(g_tlsiID_SC2); 
	 pthread_key_delete(g_tlsiID_SC3); 
}



#else

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  reason, LPVOID lpReserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		if ((g_tlsiID_SC1 = TlsAlloc()) == TLS_OUT_OF_INDEXES)
			return FALSE;
		if ((g_tlsiID_SC2 = TlsAlloc()) == TLS_OUT_OF_INDEXES)
			return FALSE;
		if ((g_tlsiID_SC3 = TlsAlloc()) == TLS_OUT_OF_INDEXES)
			return FALSE;
		initTlsThread();
	}
	else if(reason == DLL_THREAD_ATTACH)
	{
		initTlsThread();
	}
	else if(reason == DLL_THREAD_DETACH)
	{
		cleanupTls();
	}
	else if (reason == DLL_PROCESS_DETACH)
	{
		cleanupTls();
		TlsFree(g_tlsiID_SC1);
		TlsFree(g_tlsiID_SC2);
		TlsFree(g_tlsiID_SC3);
	}
	return TRUE;
}
#endif
#endif //(_UNICODE && defined(_WIN32) && _MSC_VER)


