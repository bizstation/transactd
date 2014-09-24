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
#include "datetime.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <bzs/env/crosscompile.h>
#ifdef LINUX
#include <wchar.h>
#if __clang__
#include <pthread.h>
#endif
#endif

#pragma package(smart_init) // BCB package
#pragma warning(disable : 4996) // VC++ unsafe function

#undef USETLS
#if ((defined(_WIN32) && _MSC_VER) || (__APPLE__ && !defined(__BCPLUSPLUS__)))
#define USETLS
#endif

#ifdef USETLS
extern tls_key g_tlsiID_SC3;
#else
__THREAD _TCHAR __THREAD_BCB g_date[30];
#endif

inline _TCHAR* databuf()
{
#ifdef USETLS
    _TCHAR* p = (_TCHAR*)tls_getspecific(g_tlsiID_SC3);
    if (p == NULL)
    {
        p = (_TCHAR*)new wchar_t[45];
        tls_setspecific(g_tlsiID_SC3, p);
    }
    return p;
#else
    return g_date;
#endif
}

namespace bzs
{
namespace rtl
{

int GetDate(const _TCHAR* NowDate)
{
    int ret;
    ret = _ttol(NowDate + 8);
    return ret;
}

int GetYear(const _TCHAR* NowDate, bool PrevMonth)
{
    int ret;
    ret = _ttol(NowDate);
    if ((PrevMonth) && (GetMonth(NowDate, false) == 1))
        ret--;
    return ret;
}

int GetMonth(const _TCHAR* NowDate, bool PrevMonth)
{
    int ret;
    ret = _ttol(NowDate + 5);
    if (PrevMonth)
        ret--;
    if (ret == 0)
        ret = 12;

    return ret;
}

bool IsUrudosi(int Year)
{
    if ((Year % 4) == 0)
    {
        if ((Year % 400) == 0)
            return true;
        else if ((Year % 100) == 0)
            return false;
        else
            return true;
    }
    return false;
}

bool GetDatefromYearSerialDays(int year, int Days, _TCHAR* date,
                               bool nextYearRight)
{
    // The date of the day counted from the beginning of the year is returned.
    // if orver the year then return false.

    int mm;
    int dd = Days;
    for (mm = 1; mm < 14; mm++)
    {
        switch (mm)
        {
        case 2:
            if (IsUrudosi(year))
                Days -= 29;
            else
                Days -= 28;
            break;
        case 4:
        case 6:
        case 9:
        case 11:
            Days -= 30;
            break;
        default:
            Days -= 31;
        }
        if (Days <= 0)
            break;
        dd = Days;
    }
    bool ret = true;
    if ((dd > 31) || (mm == 13))
    {
        if (nextYearRight)
        {
            year += 1;
            mm = 1;
        }
        else
        { // for old program
            dd = 31;
            mm--;
        }
        ret = false;
    }
    _stprintf(date, _T("%04d/%02d/%02d"), year, mm, dd);
    return ret;
}

//---------------------------------------------------------------------------
int JDate2NumOLD(const _TCHAR* date)
{ // 1900/01/01 = 2415021
    int yy = GetYear(date, false);
    int mm = GetMonth(date, false);
    int dd = GetDate(date);
    int i;

    // The number of leap years
    int n = 2415020;
    for (i = 1900; i < yy; i += 4)
    {
        if (IsUrudosi(i))
            n += 1;
    }
    n += (yy - 1900) * 365;
    for (i = 1; i < mm; i++)
    {
        switch (i)
        {
        case 2:
            if (IsUrudosi(yy))
                n += 29;
            else
                n += 28;
            break;
        case 4:
        case 6:
        case 9:
        case 11:
            n += 30;
            break;
        default:
            n += 31;
        }
    }
    n += dd;
    return n;
}
#ifdef _WIN32
//---------------------------------------------------------------------------
int JDate2Num(const _NTCHAR* date)
{
#ifdef _UNICODE
    wchar_t buf[20];
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, date, -1, buf, 20);
#else
    char buf[20];
    wtoa(buf, date, 20);
#endif
    return JDate2Num(buf);
}
#endif
//---------------------------------------------------------------------------
int JDate2Num(const _TCHAR* date)
{ // 1980/01/01 = 2444240
    int yy = GetYear(date, false);
    if (yy < 1980)
        return JDate2NumOLD(date);

    int mm = GetMonth(date, false);
    int dd = GetDate(date);
    int i;

    // The number of leap years
    int n = 2444239;
    for (i = 1980; i < yy; i += 4)
    {
        if (IsUrudosi(i))
            n += 1;
    }
    n += (yy - 1980) * 365;
    for (i = 1; i < mm; i++)
    {
        switch (i)
        {
        case 2:
            if (IsUrudosi(yy))
                n += 29;
            else
                n += 28;
            break;
        case 4:
        case 6:
        case 9:
        case 11:
            n += 30;
            break;
        default:
            n += 31;
        }
    }
    n += dd;
    return n;
}
//---------------------------------------------------------------------------
_TCHAR* JNum2DateOLD(int n)
{ // 1900/01/01 = 2415021
    bool uru = false;
    if (n < 2415021)
        return NULL;

    int yy = 1900;
    n -= 2415020;
    while (1)
    {
        if (uru)
        {
            if (n <= 366)
                break;
        }
        else
        {
            if (n <= 365)
                break;
        }
        if (uru)
            n -= 366;
        else
            n -= 365;
        yy++;
        uru = IsUrudosi(yy);
    }
    GetDatefromYearSerialDays(yy, n, databuf());
    return databuf();
}

//---------------------------------------------------------------------------
const _TCHAR* JNum2Date(int n)
{ // 1980/01/01 = 2444240
    bool uru = true;
    if (n < 2444240)
        return JNum2DateOLD(n);

    int yy = 1980;
    n -= 2444239;
    while (1)
    {
        if (uru)
        {
            if (n <= 366)
                break;
        }
        else
        {
            if (n <= 365)
                break;
        }
        if (uru)
            n -= 366;
        else
            n -= 365;
        yy++;
        uru = IsUrudosi(yy);
    }
    GetDatefromYearSerialDays(yy, n, databuf());
    return databuf();
}

//---------------------------------------------------------------------------
long StrToLongDate(_TCHAR* strdat)
{ // Conver string date to long. ex : 20011231
    _TCHAR buf[9];
    _tcsncpy(buf, strdat, 4);
    _tcsncpy(buf + 4, strdat + 5, 2);
    _tcsncpy(buf + 6, strdat + 8, 2);
    buf[8] = 0x00;
    return _ttol(buf);
}
//---------------------------------------------------------------------------
_TCHAR* LongToStrDate(long ldat, _TCHAR* strdat)
{ // Conver long date to string dateB
    // No check long date is valid.
    _TCHAR buf[11];
    if (ldat == 0)
        _tcscpy(strdat, _T("0000/00/00"));
    else
    {
        _stprintf_s(buf, 11, _T("%08ld"), ldat);

        _tcsncpy(strdat, buf, 4);
        strdat[4] = '/';
        _tcsncpy(strdat + 5, buf + 4, 2);
        strdat[7] = '/';
        _tcsncpy(strdat + 8, buf + 6, 2);
        strdat[10] = 0x00;
    }
    return strdat;
}
//---------------------------------------------------------------------------
_TCHAR* LongToStrTime(long ltime, _TCHAR* strdat)
{ // Conver long time to string timeB
    // No check long time is valid.
    _TCHAR buf[9];
    _stprintf_s(buf, 9, _T("%ld"), ltime);
    _tcsncpy(strdat, buf, 2);
    strdat[2] = ':';
    _tcsncpy(strdat + 3, buf + 2, 2);
    strdat[5] = ':';
    _tcsncpy(strdat + 6, buf + 4, 2);
    strdat[8] = 0x00;
    return strdat;
}
//---------------------------------------------------------------------------
const char* dateTimeStr(char* buf, unsigned int bufsize)
{
    struct tm* date;
    time_t now;
    time(&now);
#ifdef __MINGW32__
    date = localtime(&now);
#else
    struct tm tmp;
    date = &tmp;
    localtime_x(date, &now);
#endif
    sprintf_s(buf, bufsize, "%04d/%02d/%02d %02d:%02d:%02d",
              date->tm_year + 1900, date->tm_mon + 1, date->tm_mday,
              date->tm_hour, date->tm_min, date->tm_sec);
    return buf;
}

} // namespace rtl
} // namespace bzs

#pragma warning(default : 4996)
