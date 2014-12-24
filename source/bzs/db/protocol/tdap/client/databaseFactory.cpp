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
#ifdef LINUX
#include <pthread.h>
#include <bzs/env/crosscompile.h>
#include <bzs/env/mbcswchrLinux.h>
#endif

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
    catch (bzs::rtl::exception& /*e*/)
    {
    }
    return NULL;
}

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#undef USETLS
#if ((defined(_WIN32) && _MSC_VER) || (__APPLE__ && !defined(__BCPLUSPLUS__)))
#define USETLS
#endif

/* At MSVC use __thread before Windows Vista,  After the DLL is loaded with
   LoadLibrary
   then  it causes system failure.

*/
#ifdef USETLS
tls_key g_tlsiID_SC1;
tls_key g_tlsiID_SC2;
tls_key g_tlsiID_SC3;

void initTlsThread()
{
    if (tls_getspecific(g_tlsiID_SC1) == NULL)
        tls_setspecific(g_tlsiID_SC1, new wchar_t[256]);
    if (tls_getspecific(g_tlsiID_SC2) == NULL)
        tls_setspecific(g_tlsiID_SC2, new wchar_t[45]);
    if (tls_getspecific(g_tlsiID_SC3) == NULL)
        tls_setspecific(g_tlsiID_SC3, new wchar_t[45]);
}

void cleanupTls()
{
    delete (char*)tls_getspecific(g_tlsiID_SC1);
    delete (char*)tls_getspecific(g_tlsiID_SC2);
    delete (char*)tls_getspecific(g_tlsiID_SC3);
    tls_setspecific(g_tlsiID_SC1, NULL);
    tls_setspecific(g_tlsiID_SC2, NULL);
    tls_setspecific(g_tlsiID_SC3, NULL);
}

void cleanupCharPtr(void* p)
{
    delete ((char*)p);
}
#endif // USETLS

#ifdef LINUX

#include <pthread.h>
void __attribute__((constructor)) onLoadLibrary(void);
void __attribute__((destructor)) onUnloadLibrary(void);

void onLoadLibrary(void)
{
    bzs::env::initCvtProcess();
#if (defined(__APPLE__) && defined(USETLS))
    if (tls_getspecific(g_tlsiID_SC1) == NULL)
        pthread_key_create(&g_tlsiID_SC1, cleanupCharPtr);
    if (tls_getspecific(g_tlsiID_SC2) == NULL)
        pthread_key_create(&g_tlsiID_SC2, cleanupCharPtr);
    if (tls_getspecific(g_tlsiID_SC3) == NULL)
        pthread_key_create(&g_tlsiID_SC3, cleanupCharPtr);

#endif
}

void onUnloadLibrary(void)
{
    bzs::env::deinitCvtProcess();
#if (defined(__APPLE__) && defined(USETLS))
    cleanupTls();
    pthread_key_delete(g_tlsiID_SC1);
    pthread_key_delete(g_tlsiID_SC2);
    pthread_key_delete(g_tlsiID_SC3);
#endif
}

#endif // LINUX

#if (defined(_WIN32) && defined(USETLS))

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
#ifdef _MSC_VER
        _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
        _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
        //_CrtSetBreakAlloc(151);
#endif

        if ((g_tlsiID_SC1 = TlsAlloc()) == TLS_OUT_OF_INDEXES)
            return FALSE;
        if ((g_tlsiID_SC2 = TlsAlloc()) == TLS_OUT_OF_INDEXES)
            return FALSE;
        if ((g_tlsiID_SC3 = TlsAlloc()) == TLS_OUT_OF_INDEXES)
            return FALSE;
        initTlsThread();
    }
    else if (reason == DLL_THREAD_ATTACH)
    {
        initTlsThread();
    }
    else if (reason == DLL_THREAD_DETACH)
    {
        cleanupTls();
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        cleanupTls();
        TlsFree(g_tlsiID_SC1);
        TlsFree(g_tlsiID_SC2);
        TlsFree(g_tlsiID_SC3);
#ifdef _MSC_VER
        OutputDebugString(_T("After tdclcpp DLL_PROCESS_DETACH \n"));
        _CrtDumpMemoryLeaks();
#endif
    }
    return TRUE;
}
#endif //(defined(_WIN32) && defined(USETLS))

