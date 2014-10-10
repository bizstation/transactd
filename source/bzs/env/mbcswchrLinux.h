#ifndef BZS_ENV_MBCSWCHRLINUX_H
#define BZS_ENV_MBCSWCHRLINUX_H
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

#include <charsetConvert.h>

#define CP_UTF8 65001
#define CP_ACP CP_UTF8
#define GetACP() CP_ACP

#define MBC_CHARSETNAME "SHIFT-JIS" // mbc charctor set
#define UTF8_CHARSETNAME "UTF-8"

namespace bzs
{
namespace env
{

void initCvtProcess();
void deinitCvtProcess();
cvt& getCvt(int index);

inline cvt& mbcscvt()
{
    return getCvt(0);
}

inline cvt& wchrcvt()
{
    return getCvt(1);
}

inline cvt& u8mbcvt()
{
    return getCvt(2);
}

inline cvt& mbu8cvt()
{
    return getCvt(3);
}

inline cvt& u8wccvt()
{
    return getCvt(4);
}

inline cvt& wcu8cvt()
{
    return getCvt(5);
}

inline int WideCharToMultiByte(unsigned int codepage, unsigned int dwFlags,
                               const char16_t* lpWideCharStr, int cchWideChar,
                               char* lpMultiByteStr, int cchMultiByte,
                               const char* lpDefaultChar,
                               int* lpUsedDefaultChar)
{
    if (codepage == CP_UTF8)
        return wcu8cvt().conv(lpWideCharStr, (size_t)(cchWideChar + cchWideChar),
                            lpMultiByteStr, (size_t)cchMultiByte);
    return wchrcvt().conv(lpWideCharStr, (size_t)(cchWideChar + cchWideChar),
                        lpMultiByteStr, (size_t)cchMultiByte);
}

inline int MultiByteToWideChar(unsigned int codepage, unsigned int dwFlags,
                               const char* lpMultiByteStr, int cchMultiByte,
                               char16_t* lpWideCharStr, int cchWideChar)
{
    if (codepage == CP_UTF8)
        return u8wccvt().conv(lpMultiByteStr, (size_t)cchMultiByte, lpWideCharStr,
                            (size_t)(cchWideChar + cchWideChar));
    return mbcscvt().conv(lpMultiByteStr, (size_t)cchMultiByte, lpWideCharStr,
                        (size_t)(cchWideChar + cchWideChar));
}

inline int u8tombc(const char* u8, int u8size, char* mbc, int mbcsize)
{
    return u8mbcvt().conv(u8, (size_t)u8size, mbc, (size_t)mbcsize);
}

inline int mbctou8(const char* mbc, int mbcsize, char* u8, int u8size)
{
    return mbu8cvt().conv(mbc, (size_t)mbcsize, u8, (size_t)u8size);
}

} // namespace env
} // namespace bzs

// Definition dummy
#define MB_PRECOMPOSED 0
#define WC_COMPOSITECHECK 0

using bzs::env::WideCharToMultiByte;
using bzs::env::MultiByteToWideChar;

// Surrogate support
#if !defined IS_HIGH_SURROGATE
#define IS_HIGH_SURROGATE(t) ((t >= 0xD800) && (t <= 0xDBEF))
#endif

#endif // BZS_ENV_MBCSWCHRLINUX_H
