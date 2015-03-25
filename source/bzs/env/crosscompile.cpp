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

char* _i64toa_s(__int64 v, char* tmp, unsigned long size, int radix)
{
    snprintf(tmp, size, "%lld", v);
    return tmp;
}

char* _ltoa_s(int v, char* tmp, unsigned long size, int radix)
{
    snprintf(tmp, size, "%d", v);
    return tmp;
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
    return 0 - *sr;
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
