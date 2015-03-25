#ifndef BZS_ENV_CROSSCOMPILE_H
#define BZS_ENV_CROSSCOMPILE_H
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
#if (defined(__FreeBSD__) || defined(__APPLE__))
#ifndef LINUX
#define LINUX
#endif
#endif

#if __MINGW32__
#include "tcharMinGW.h"
#define strcat_s(A, B, C) strcat(A, C)
#if (defined(__GNUC__) && __GNUC__ < 4) ||                                     \
    (__GNUC__ == 4 && __GNUC_MINOR__ <= 5) // 4.5 and lesser
#define strcat_s(A, B, C) strcat(A, C)
#define sprintf_s snprintf
#define swprintf_s swprintf
#define strcpy_s(A, B, C) strcpy(A, C)
#define strncpy_s(A, B, C, D) strncpy(A, C, D)
#ifdef _UNICODE
#define _stprintf_s swprintf_s
#else
#define _stprintf_s snprintf
#endif
#endif
int gettimeofday(struct timeval*, struct timezone*);
#endif // __MINGW32__

#if (defined(LINUX))
#include <ctype.h>
#include <stddef.h>
#include <linuxTypes.h>
/* c c++ runtime library */
#define _strnicmp strncasecmp
#define __int64 long long int
#define _atoi64 atoll
#define _wtoi(W) wcstol(W, NULL, 10)
#define _access access
#define sprintf_s snprintf
#define swprintf_s swprintf
#define localtime_s localtime_r
#define strncpy_s(A, B, C, D) strncpy(A, C, D)
#define strcpy_s(A, B, C) strcpy(A, C)
#define strcat_s(A, B, C) strcat(A, C)
#define _strlwr_s(A, B) _strlwr(A)
#define _timezone timezone

/*if on linux that cannot use multi byte char for meta data.*/
#define _mbsstr strstr
#define _mbsupr _strupr
#define _mbsrchr strrchr
#define _wcsupr _strupr16
#define _wcslwr _strlwr16

char* _strupr(char* s);
char* _strlwr(char* s);
char* _ltoa_s(int v, char* tmp, unsigned long size, int radix);
char* _i64toa_s(__int64 v, char* tmp, unsigned long size, int radix);
char16_t* _strupr16(char16_t* s);
char16_t* _strlwr16(char16_t* s);
size_t strlen16(const char16_t* src);
int wcsnicmp16(const char16_t* sl, const char16_t* sr, size_t n);
int wcsncmp16(const char16_t* sl, const char16_t* sr, size_t n);
char16_t* wmemset16(char16_t* p, char16_t c, size_t n);
char16_t* wmemcpy(char16_t* dest, const char16_t* src, size_t count);

/* operating system */
#ifndef _TCHAR
#define _TCHAR char
#endif
#ifndef _T
#define _T(A) A
#endif

#define PSEPARATOR _T("/")
#define PSEPARATOR_A "/"
#define PSEPARATOR_C '/'
typedef int HWND;
#define MAX_PATH 266
#define Sleep(A) usleep(A*1000)
#define LoadLibrary(A) dlopen(A, RTLD_LAZY)
#define LoadLibraryA(A) dlopen(A, RTLD_LAZY)
#define GetProcAddress(A, B) dlsym(A, B)
#define FreeLibrary(A) dlclose(A)
#define OutputDebugString(A)

/* muliti byete char */
typedef char mbchar;
typedef char char_m;

#else //!defined(LINUX)

// define _ttof for MinGW or less than visual studio 2010
#if defined(__MINGW32__) || (defined(_MSC_VER) && _MSC_VER < 1600)
#ifdef _UNICODE
#define _ttof _wtof
#else
#define _ttof atof
#endif
#endif

// define char16_t and char32_t for Visual Studio 2008 or earlier, and MinGW
#if (defined(_MSC_VER) && _MSC_VER < 1600) || defined(__MINGW32__)
typedef unsigned __int16 char16_t; // 16bit
typedef unsigned __int32 char32_t; // 32bit
#endif

/* c c++ runtime library */
#include <tchar.h>
#if defined(__BORLANDC__) || defined(__MINGW32__)
#define _ltow_s(A, B, C, D) _ltow(A, B, D)
#define _ltoa_s(A, B, C, D) _ltoa(A, B, D)
#define _ltot_s(A, B, C, D) _ltot(A, B, D)
#define _ultot_s(A, B, C, D) _ultot(A, B, D)
#define _i64tot_s(A, B, C, D) _i64tot(A, B, D)
#define _i64tow_s(A, B, C, D) _i64tow(A, B, D)
#define _i64toa_s(A, B, C, D) _i64toa(A, B, D)
#define _strlwr_s(A, B) strlwr(A)
#endif

#if defined(__BORLANDC__)
#define _strupr strupr
#define _strnicmp strnicmp
#endif

#define wcsnicmp16(A, B, C)                                                    \
    _wcsnicmp((const wchar_t*)(A), (const wchar_t*)(B), C)
#define wcsncmp16(A, B, C) wcsncmp((const wchar_t*)(A), (const wchar_t*)(B), C)
#define wmemset16 wmemset
#define strlen16(A) wcslen((const wchar_t*)(A))

/* operating system */
#define PSEPARATOR _T("\\")
#define PSEPARATOR_A "\\"
#define PSEPARATOR_C '\\'

/* muliti byete char */
typedef unsigned char mbchar;
typedef unsigned char char_m;

#endif // defined(LINUX)

#if (defined(__BORLANDC__) || defined(LINUX))
#define localtime_x(_tm, time) localtime_s(time, _tm)
#else //!__BORLANDC__
#define localtime_x(_tm, time) (localtime_s(_tm, time) == 0)
#endif //__BORLANDC__

#ifdef _WIN32
#define tls_key DWORD
#define tls_getspecific(A) TlsGetValue(A)
#define tls_setspecific(A, B) TlsSetValue(A, B)
#else
#define tls_key pthread_key_t
#define tls_getspecific(A) pthread_getspecific(A)
#define tls_setspecific(A, B) pthread_setspecific(A, B)
#endif

#endif // BZS_ENV_CROSSCOMPILE_H
