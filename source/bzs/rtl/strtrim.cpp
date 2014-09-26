/*=================================================================
   Copyright (C) 2000 2013 BizStation Corp All rights reserved.

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
#include <bzs/env/tstring.h>
#pragma hdrstop

#include "strtrim.h"

#pragma package(smart_init)

namespace bzs
{
namespace rtl
{

char* strtrimA(char* str)
{ // trim right space
    if (str == NULL)
        return NULL;
    int k = (int)strlen(str) - 1;

    for (; k >= 0; --k)
    {
        if (str[k] == 0x20)
            str[k] = 0x00;
        else
            break;
    }
    return str;
}

wchar_t* wcstrim(wchar_t* str)
{
    // trim right space
    if (str == NULL)
        return NULL;
    int k = (int)wcslen(str) - 1;

    for (; k >= 0; k--)
    {
        if (str[k] == 0x20)
            str[k] = 0x00;
        else
            break;
    }
    return str;
}

char* strltrimA(char* str)
{ // trim left space
    if (str == NULL)
        return NULL;

    size_t i;
    char* point = NULL;
    size_t len = strlen(str);
    for (i = 0; i < len; i++)
    {
        if (str[i] == 0x20)
            point = str + i + 1;
        else
            break;
    }
    if (point)
        memmove(str, point, len - i + 1);
    return str;
}

wchar_t* wcsltrim(wchar_t* str)
{ // trim left space
    if (str == NULL)
        return NULL;

    size_t i;
    wchar_t* point = NULL;
    size_t len = wcslen(str);
    for (i = 0; i < len; i++)
    {
        if (str[i] == 0x20)
            point = str + i + 1;
        else
            break;
    }
    if (point)
        memmove(str, point, (len - i + 1) * sizeof(wchar_t));
    return str;
}

#ifdef _UNICODE
int wcslen_a(wchar_t* str)
{ // How meny charctors of multibyte charctor.
    int retVale = 0;
    size_t len = wcslen(str);
    if (len)
    {
        LPWORD charType = new WORD[len + 1];
        if (GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE3, str, -1, charType))
        {
            for (size_t i = 0; i < len; i++)
            { // If Zenkaku_katakana or Hiragana then return not Zenkaku. Kanji
                // is C3_IDEOGRAPH.
                if (!(charType[i] & C3_HALFWIDTH) &&
                    ((charType[i] & C3_KATAKANA) ||
                     (charType[i] & C3_HIRAGANA) ||
                     (charType[i] & C3_FULLWIDTH) ||
                     (charType[i] & C3_IDEOGRAPH) || (charType[i] & C3_SYMBOL)))
                    retVale += 2;
                else
                    retVale += 1;
            }
        }
        delete[] charType;
    }
    return retVale;
}
#endif

} // namespace rtl
} // namespace bzs
