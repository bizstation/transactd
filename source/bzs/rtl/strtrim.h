#ifndef BZS_RTL_STRTRIM_H
#define BZS_RTL_STRTRIM_H
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
#include <bzs/env/compiler.h>
namespace bzs
{
namespace rtl
{

PACKAGE  char*     strtrimA(char* str);
PACKAGE  wchar_t*  wcstrim(wchar_t* str);
PACKAGE  char*     strltrimA(char* str);
PACKAGE  wchar_t*  wcsltrim(wchar_t* str);
#ifdef _UNICODE
    PACKAGE  int   wcslen_a(wchar_t* str); //half charctor length

	#define strtrim wcstrim
	#define strltrim wcsltrim
#else
	#define strtrim strtrimA
	#define strltrim strltrimA
#endif

}//namespace rtl
}//namespace bzs


#endif//BZS_RTL_STRTRIM_H
 
