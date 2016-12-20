/*=================================================================
   Copyright (C) 2012 2013 BizStation Corp All rights reserved.

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
#ifdef LINUX
#include "crosscompile.h"
#include <stdio.h>
#include <string.h>
#include <wctype.h>
#include <wchar.h>


char16_t* _strupr16(char16_t* s)
{
    char16_t* p = s;
    while (*s)
    {
        *s = towupper(*s);
        ++s;
    }
    return p;
}

char16_t* _strlwr16(char16_t* s)
{
    char16_t* p = s;
    while (*s)
    {
        *s = towlower(*s);
        ++s;
    }
    return p;
}

char* _strupr(char* s)
{
    char* p = s;
    while (*s)
    {
        *s = toupper(*s);
        ++s;
    }
    return p;
}

char* _strlwr(char* s)
{
    char* p = s;
    while (*s)
    {
        *s = tolower(*s);
        ++s;
    }
    return p;
}

int wcsnicmp16(const char16_t* sl, const char16_t* sr, size_t n)
{
    if (n == 0)
        return 0;
    while (*sl)
    {
        wchar_t tmpl = towlower(*sl);
        wchar_t tmpr = towlower(*sr);

        if (tmpl != tmpr)
            return (int)(tmpl - tmpr);
        if (--n == 0)
            return 0;
        ++sl;
        ++sr;
    }
    return 0 - *sr;
}

int wcsncmp16(const char16_t* sl, const char16_t* sr, size_t n)
{
    if (n == 0)
        return 0;
    while (*sl)
    {
        if (*sl != *sr)
            return (int)(*sl - *sr);
        if (--n == 0)
            return 0;
        ++sl;
        ++sr;
    }
    return 0 - *sr;
}

int wmemcmp16(const char16_t* sl, const char16_t* sr, size_t n)
{
    if (n == 0)
        return 0;
    while (1)
    {
        if (*sl != *sr)
            return (int)(*sl - *sr);
        if (--n == 0)
            return 0;
        ++sl;
        ++sr;
    }
    //return 0 - *sr;
}


char16_t* wmemset16(char16_t* p, char16_t c, size_t n)
{
    char16_t* end = p + n;
    for (char16_t* tmp = p; tmp < end; ++tmp)
        *tmp = c;
    return p;
}

char16_t* wmemcpy(char16_t* dest, const char16_t* src, size_t count)
{
    return (char16_t*)memcpy(dest, src, count * sizeof(char16_t));
}

#endif // LINUX

#ifdef __APPLE__
#include <dlfcn.h>
/* buffer size need 266 bytes */
char* GetModuleFileName(char* retBuf)
{
    retBuf[0] = 0x00;
    Dl_info info;
    if (dladdr(reinterpret_cast<void*>(GetModuleFileName), &info) == 0)
        return retBuf;
    strcpy(retBuf, info.dli_fname);
    return retBuf;
}
#endif //__APPLE__


#if (defined(_WIN32) && !defined(__MINGW32__))
#include <windows.h>
#include "crosscompile.h"

static const __int64 EPOCH = ((__int64) 116444736000000000ULL);
int gettimeofday(struct timeval * tp, struct timezone * tzp)
{

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    __int64    time;

    GetSystemTime( &system_time );
    SystemTimeToFileTime( &system_time, &file_time );
    time =  ((__int64)file_time.dwLowDateTime )      ;
    time += ((__int64)file_time.dwHighDateTime) << 32;

    tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
    return 0;
}
#endif //(defined(_WIN32) && !defined(__MINGW32__))
