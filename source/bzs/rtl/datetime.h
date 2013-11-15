#ifndef BZS_RT_DATETIME_H
#define BZS_RT_DATETIME_H
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
#include <bzs/env/tstring.h>

namespace bzs
{
namespace rtl
{
//---------------------------------------------------------------------------
PACKAGE int   GetYear(const _TCHAR* NowDate, bool PrevMonth);
PACKAGE int   GetMonth(const _TCHAR* NowDate, bool PrevMonth);
PACKAGE int   GetDate(const _TCHAR* NowDate);
PACKAGE bool  IsUrudosi(int Year);
PACKAGE bool  GetDatefromYearSerialDays(int year, int Days, _TCHAR* date
											, bool nextYearRight=false);

PACKAGE int   JDate2Num(const _TCHAR* date);

#ifdef _WIN32
PACKAGE int   JDate2Num(const _NTCHAR* date);
#endif

PACKAGE const _TCHAR* JNum2Date(int ldate);

PACKAGE long 	StrToLongDate(_TCHAR* pStrDate);
PACKAGE _TCHAR* LongToStrDate(long , _TCHAR* );
PACKAGE _TCHAR* LongToStrTime(long , _TCHAR* );
PACKAGE const char* dateTimeStr(char* buf, unsigned int bufsize);

}//namespace rtl
}//namespace bzs

#endif//BZS_RT_DATETIME_H
