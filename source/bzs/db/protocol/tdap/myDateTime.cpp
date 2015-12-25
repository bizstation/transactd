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
#include "myDateTime.h"
#include <bzs/env/tstring.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <algorithm>

namespace bzs
{
namespace db
{
namespace protocol
{
namespace tdap
{
#pragma warning(disable : 4996)
#ifdef _WIN32
const wchar_t wtime_format_ms[] = L"%02d:%02d:%02d.%0*u";
const wchar_t wtime_format[] = L"%02d:%02d:%02d";
const wchar_t wdatetime_format_ms[] = L"%04d-%02d-%02d %02d:%02d:%02d.%0*u";
const wchar_t wdatetime_format[] = L"%04d-%02d-%02d %02d:%02d:%02d";
#endif
const char time_format_ms[] = "%02d:%02d:%02d.%0*u";
const char time_format[] = "%02d:%02d:%02d";
const char datetime_format_ms[] =   "%04d-%02d-%02d %02d:%02d:%02d.%0*u";
const char datetime_format[] =      "%04d-%02d-%02d %02d:%02d:%02d";

//-------------------------------------------------------------
void myDate::setValue(int v, bool btrvValue)
{
    if (btrvValue)
    {
        btrDate btrd;
        btrd.i = v;
        yy = btrd.yy;
        mm = btrd.mm;
        dd = btrd.dd;
        tmp = 0;
    }
    else
        i = v;
}

int myDate::getValue(bool btrvValue)
{
    if (btrvValue)
    {
        btrDate btrd;
        btrd.yy = yy;
        btrd.mm = mm;
        btrd.dd = dd;
        return btrd.i;
    }
    return i;
}

char* myDate::toStr(char* p, bool btrvValue)
{
    if (btrvValue)
        sprintf(p, "%04d/%02d/%02d", yy, mm, dd);
    else
        sprintf(p, "%04d-%02d-%02d", yy, mm, dd);
    return p;
}

myDate& myDate::operator=(const char* p)
{
    size_t len = strlen(p);
    i = 0;
    if (len > 3)
    {
        yy = atol(p);
        if (len > 6)
        {
            mm = atol(p + 5);
            if (len > 9)
                dd = atol(p + 8);
        }
    }
    return *this;
}

#ifdef _WIN32
wchar_t* myDate::toStr(wchar_t* p, bool btrvValue)
{
    if (btrvValue)
        swprintf_s(p, 11, L"%04d/%02d/%02d", yy, mm, dd);
    else
        swprintf_s(p, 11, L"%04d-%02d-%02d", yy, mm, dd);
    return p;
}

myDate& myDate::operator=(const wchar_t* p)
{
    size_t len = wcslen(p);
    i = 0;
    if (len > 3)
    {
        yy = _wtol(p);
        if (len > 6)
        {
            mm = _wtol(p + 5);
            if (len > 9)
                dd = _wtol(p + 8);
        }
    }
    return *this;
}
#endif


//-------------------------------------------------------------
void myTime::setValue(__int64 v, bool btrvValue)
{
    i64 = 0;
    if (btrvValue)
    {
        btrTime btrt;
        btrt.i = (int)v;
        hh = btrt.hh;
        nn = btrt.nn;
        ss = btrt.ss;
        ms = btrt.uu * 10000;
        return;
    }
    if (m_bigendian)
    {
        char* p = (char*)&i64;
        char* src = (char*)&v;
        if (m_dec)
        {
            p[0] = src[5];
            p[1] = src[4];
            p[2] = src[3];
            ms = ms >> (3 - ((m_dec + 1) / 2)) * 8;
        }
        p[3] = src[2];
        p[4] = src[1];
        p[5] = src[0];
        sign = 0;
    }else
    {//DDÅ~24Å~3600 + HHÅ~3600 + MMÅ~60 + SS
        hh = v / 3600;
        nn = (v % 3600) / 60;
        ss = v % 60;
    }
}

__int64 myTime::getValue(bool btrvValue)
{
    if (btrvValue)
    {
        btrTime btrt;
        btrt.hh = hh;
        btrt.nn = nn;
        btrt.ss = ss;
        btrt.uu = (char)(ms / 100000);
        return btrt.i;
    }

    __int64 v = 0;
    sign = 1;
    if (m_bigendian)
    {
        char* src = (char*)&i64;
        char* p = (char*)&v;
        p[2] = src[3];
        p[1] = src[4];
        p[0] = src[5];
        if (m_dec)
        {
            ms = ms << (3 - ((m_dec + 1) / 2)) * 8;
            p[3] = src[2];
            p[4] = src[1];
            p[5] = src[0];
        }
    }else
        v =  (hh * 3600) + (nn * 60) + ss;
    sign = 0;
    return v;
}

char* myTime::toStr(char* p)
{
    if (m_dec)
        sprintf(p, time_format_ms, (int)hh, (int)nn, (int)ss,
                m_dec, (unsigned int)ms);
    else
        sprintf(p, time_format, (int)hh, (int)nn, (int)ss);
    return p;
}

myTime& myTime::operator=(const char* p)
{
    i64 = 0;
    size_t len = strlen(p);
    if (len > 1)
    {
        hh = atol(p);
        if (len > 4)
        {
            nn = atol(p + 3);
            if (len > 7)
            {
                ss = atol(p + 6);
                if (m_dec && len > 9)
                {
                    char tmp[10] = { 0x00 };
                    strncpy(tmp, p + 9, (size_t)m_dec);
                    ms = atol(tmp);
                }
            }
        }
    }
    return *this;
}

#ifdef _WIN32
wchar_t* myTime::toStr(wchar_t* p)
{
    if (m_dec)
        swprintf_s(p, 17, wtime_format_ms, (int)hh, (int)nn, (int)ss,
                    m_dec, (unsigned int)ms);
    else
        swprintf_s(p, 9, wtime_format, (int)hh, (int)nn, (int)ss);
    return p;
}

myTime& myTime::operator=(const wchar_t* p)
{
    i64 = 0;
    size_t len = wcslen(p);
    if (len > 1)
    {
        hh = _wtol(p);
        if (len > 4)
        {
            nn = _wtol(p + 3);
            if (len > 7)
            {
                ss = _wtol(p + 6);
                if (m_dec && len > 9)
                {
                    wchar_t tmp[10] = { 0x00 };
                    wcsncpy(tmp, p + 9, (size_t)m_dec);
                    ms = _wtol(tmp);
                }
            }
        }
    }
    return *this;
}
#endif
//-------------------------------------------------------------
static int digit_logs[7] = {1, 10, 100, 1000, 10000, 100000, 1000000}; // digit 0 to 6
void maTime::setValue(__int64 v, bool btrvValue)
{
    if (m_bigendian && !btrvValue)
    {
        i64 = 0;
        char* p = (char*)&i64;
        char* src = (char*)&v;
        p[0] = src[5];
        p[1] = src[4];
        p[2] = src[3];
        p[3] = src[2];
        p[4] = src[1];
        p[5] = src[0];
        i64 = i64 >> (3 - ((m_dec + 1) / 2)) * 8;
        __int64 v = i64;
        ms = v % digit_logs[m_dec]; v /= digit_logs[m_dec];
        ss = v % 60; v /= 60;
        nn = v % 60; v /= 60;
        hh = v % 24; v /= 24;
        sign = 0;
    }else
        maTime::setValue(v, btrvValue);
}

__int64 maTime::getValue(bool btrvValue)
{
    if (!m_bigendian || btrvValue) return myTime::getValue(btrvValue);
    __int64 i64t = ((hh * 60 + nn) * 60 + ss) * digit_logs[m_dec] + ms;
    __int64 v = 0;
    char* src = (char*)&i64t;
    char* p = (char*)&v;
    p[2] = src[3];
    p[1] = src[4];
    p[0] = src[5];
    p[3] = src[2];
    p[4] = src[1];
    p[5] = src[0];
    v = v >> (3 - ((m_dec + 1) / 2)) * 8;
    return v;
}

maTime& maTime::operator=(const char* p)
{
    myTime::operator=(p);
    return *this;
}

#ifdef _WIN32
maTime& maTime::operator=(const wchar_t* p)
{
    myTime::operator=(p);
    return *this;
}
#endif
//-------------------------------------------------------------
void myDateTime::setValue(__int64 v)
{
    i64 = 0;
    if (m_bigendian)
    {
        char* p = (char*)&i64;
        char* src = (char*)&v;

        p[3] = src[4];
        p[4] = src[3];
        p[5] = src[2];
        p[6] = src[1];
        p[7] = src[0];
        if (i64 && m_dec)
        {
            p[0] = src[7];
            p[1] = src[6];
            p[2] = src[5];
            ms = ms >> (3 - ((m_dec + 1) / 2)) * 8;
        }

    }else
    {   // YYYYÅ~10000 + MMÅ~100 + DD  HHÅ~10000 + MMÅ~100 + SS
        __int64 yy = v / 10000000000LL;
        __int64 m = (v / 100000000LL) % 100;
        yymm = yy*13 + m;
        dd = (v / 1000000L) % 100;
        hh = (v / 10000L) % 100;
        nn = (v / 100L) % 100;
        ss = v % 100;
    }
    sign = 0;
}

__int64 myDateTime::getValue()
{
    __int64 v = 0;
    sign = 1;
    if (m_bigendian)
    {
        char* src = (char*)&i64;
        char* p = (char*)&v;
        p[4] = src[3];
        p[3] = src[4];
        p[2] = src[5];
        p[1] = src[6];
        p[0] = src[7];
        if (v && m_dec)
        {
            ms = ms << (3 - ((m_dec + 1) / 2)) * 8;
            p[5] = src[2];
            p[6] = src[1];
            p[7] = src[0];
        }

    }else
    { // YYYYÅ~10000 + MMÅ~100 + DD  HHÅ~10000 + MMÅ~100 + SS
        __int64 yy = yymm / 13;
        __int64 m =  yymm % 13;
        v = (yy * 10000000000) + (m * 100000000LL) + (dd * 1000000LL) +
             (hh * 10000LL) + (nn * 100LL) + ss;
    }
    sign = 0;
    return v;
}

myDateTime& myDateTime::operator=(const char* p)
{
    size_t len = strlen(p);
    i64 = 0;

    if (len > 6)
    {
        yymm = atol(p) * 13 + atol(p + 5);
        if (len > 9)
        {
            dd = atol(p + 8);
            if (len > 12)
            {
                hh = atol(p + 11);
                if (len > 15)
                {
                    nn = atol(p + 14);
                    if (len > 18)
                    {
                        ss = atol(p + 17);
                        if (m_dec && len > 20)
                        {
                            char tmp[10] = { 0x00 };
                            strncpy(tmp, p + 20, (size_t)m_dec);
                            ms = atol(tmp);
                        }
                    }
                }
            }
        }
    }
    return *this;
}


void myDateTime::setTime(const char* p)
{
    i64 = 0;
    size_t len = strlen(p);
    if (len > 1)
    {
        hh = atol(p);
        if (len > 4)
        {
            nn = atol(p + 3);
            if (len > 7)
            {
                ss = atol(p + 6);
                if (m_dec && len > 9)
                {
                    char tmp[10] = { 0x00 };
                    strncpy(tmp, p + 9, (size_t)m_dec);
                    ms = atol(tmp);
                }
            }
        }
    }
}


char* myDateTime::date_str(char* p) const
{
    sprintf(p, "%04d-%02d-%02d", (int)(yymm / 13),
                    (int)(yymm % 13), (int)dd);
    return p;
}

char* myDateTime::time_str(char* p, int decimals) const
{
    if (decimals)
        sprintf(p, time_format_ms, (int)hh, (int)nn, (int)ss,
                    std::min<int>(decimals, m_dec), (unsigned int)ms);
    else
        sprintf(p, time_format, (int)hh, (int)nn, (int)ss);
    return p;
}

char* myDateTime::dateTime_str(char* p, int decimals) const
{
    if (decimals)
    {
        sprintf(p, datetime_format_ms, (int)(yymm / 13), (int)(yymm % 13),
                (int)dd, (int)hh, (int)nn, (int)ss,
                            std::min<int>(decimals, m_dec), (unsigned int)ms);
    }
    else
        sprintf(p, datetime_format, (int)(yymm / 13), (int)(yymm % 13),
                (int)dd, (int)hh, (int)nn, (int)ss);
    return p;
}


#ifdef _WIN32


myDateTime& myDateTime::operator=(const wchar_t* p)
{
    size_t len = wcslen(p);
    i64 = 0;
    
    if (len > 6)
    {
        yymm = _wtol(p) * 13 + _wtol(p + 5);
        if (len > 9)
        {
            dd = _wtol(p + 8);
            if (len > 12)
            {
                hh = _wtol(p + 11);
                if (len > 15)
                {
                    nn = _wtol(p + 14);
                    if (len > 18)
                    {
                        ss = _wtol(p + 17);
                        if (m_dec && len > 20)
                        {
                            wchar_t tmp[10] = { 0x00 };
                            wcsncpy(tmp, p + 20, (size_t)m_dec);
                            ms = _wtol(tmp);
                        }
                    }
                }
            }
        }
    }
    return *this;
}

void myDateTime::setTime(const wchar_t* p)
{
    i64 = 0;
    size_t len = wcslen(p);
    if (len > 1)
    {
        hh = _wtol(p);
        if (len > 4)
        {
            nn = _wtol(p + 3);
            if (len > 7)
            {
                ss = _wtol(p + 6);
                if (m_dec && len > 9)
                {
                    wchar_t tmp[10] = { 0x00 };
                    wcsncpy(tmp, p + 9, (size_t)m_dec);
                    ms = _wtol(tmp);
                }
            }
        }
    }
}

wchar_t* myDateTime::date_str(wchar_t* p) const
{
    swprintf(p, L"%04d-%02d-%02d", (int)(yymm / 13),
                    (int)(yymm % 13), (int)dd);
    return p;
}

wchar_t* myDateTime::time_str(wchar_t* p, int decimals ) const
{
    if (decimals)
        swprintf(p, wtime_format_ms, (int)hh, (int)nn, (int)ss,
                    std::min<int>(decimals, m_dec), (unsigned int)ms);
    else
        swprintf(p, wtime_format, (int)hh, (int)nn, (int)ss);
    return p;
}

wchar_t* myDateTime::dateTime_str(wchar_t* p, int decimals) const
{
    if (decimals)
        swprintf(p, wdatetime_format_ms, (int)(yymm / 13),
                    (int)(yymm % 13), (int)dd, (int)hh, (int)nn, (int)ss,
                    std::min<int>(decimals, m_dec), (unsigned int)ms);
    else
        swprintf(p, wdatetime_format, (int)(yymm / 13),
                    (int)(yymm % 13), (int)dd, (int)hh, (int)nn, (int)ss);
    return p;
}

#endif
//-------------------------------------------------------------
void maDateTime::setValue(__int64 v)
{
    if (m_bigendian)
    {
        char* p = (char*)&i64;
        char* src = (char*)&v;
        p[3] = src[4];
        p[4] = src[3];
        p[5] = src[2];
        p[6] = src[1];
        p[7] = src[0];
        p[0] = src[7];
        p[1] = src[6];
        p[2] = src[5];
        i64 = i64 >> (3 - ((m_dec + 1) / 2)) * 8;
        __int64 v = i64;
        ms = v % digit_logs[m_dec]; v /= digit_logs[m_dec];
        ss = v % 60; v /= 60;
        nn = v % 60; v /= 60;
        hh = v % 24; v /= 24;
        dd = v % 32; v /= 32;
        __int64 mm = v % 13; v /= 13;
        __int64 yy = v;
        yymm = yy*13 + mm;
        sign = 0;
    }else
        myDateTime::setValue(v);
}

__int64 maDateTime::getValue()
{
    if (!m_bigendian) return myDateTime::getValue();
    __int64 i64t = ((((( yymm) * 32 + dd) * 24 + hh) * 60 + nn) * 60 +
                ss) * digit_logs[m_dec] + ms;
    __int64 v;
    char* src = (char*)&i64t;
    char* p = (char*)&v;
    p[4] = src[3];
    p[3] = src[4];
    p[2] = src[5];
    p[1] = src[6];
    p[0] = src[7];
    p[5] = src[2];
    p[6] = src[1];
    p[7] = src[0];
    v = v >> (3 - ((m_dec + 1) / 2)) * 8;
    return v;
}

maDateTime& maDateTime::operator=(const char* p)
{
    myDateTime::operator=(p);
    return *this;
}

#ifdef _WIN32
maDateTime& maDateTime::operator=(const wchar_t* p)
{
    myDateTime::operator=(p);
    return *this;
}
#endif

//-------------------------------------------------------------
void myTimeStamp::setValue(__int64 v)
{
    i64 = 0;
    if (m_bigendian)
    {
        char* p = (char*)&i64;
        char* src = (char*)&v;
        p[3] = src[3];
        p[4] = src[2];
        p[5] = src[1];
        p[6] = src[0];
        if (m_dec && datetime)
        {
            p[0] = src[6];
            p[1] = src[5];
            p[2] = src[4];
            ms = ms >> (3 - ((m_dec + 1) / 2)) * 8;
        }
    }else
        datetime = v;
}

__int64 myTimeStamp::getValue()
{
    if (m_bigendian)
    {
        __int64 v = 0;
        char* src = (char*)&i64;
        char* p = (char*)&v;

        p[3] = src[3];
        p[2] = src[4];
        p[1] = src[5];
        p[0] = src[6];
        if (m_dec)
        {
            ms = ms << (3 - ((m_dec + 1) / 2)) * 8;
            p[4] = src[2];
            p[5] = src[1];
            p[6] = src[0];
        }
        return v;
    }
    return datetime;
}

char* myTimeStamp::toStr(char* p)
{
    if (datetime)
    {
        time_t v = (time_t)datetime;
        struct tm* st = localtime(&v);

        if (m_dec)
            sprintf(p, datetime_format_ms, st->tm_year + 1900, st->tm_mon + 1,
                    st->tm_mday, st->tm_hour, st->tm_min, st->tm_sec,
                    m_dec, (unsigned int)ms);
        else
            sprintf(p, datetime_format, st->tm_year + 1900, st->tm_mon + 1,
                    st->tm_mday, st->tm_hour, st->tm_min, st->tm_sec);
    }else
    {
        if (m_dec)
            sprintf(p, datetime_format_ms, 0, 0, 0, 0, 0, 0, m_dec, 0);
        else
            sprintf(p, datetime_format, 0, 0, 0, 0, 0, 0);

    }
    return p;
}

myTimeStamp& myTimeStamp::operator=(const char* p)
{
    size_t len = strlen(p);
    struct tm st;
    memset(&st, 0, sizeof(tm));
    i64 = 0;
    if (len > 3)
    {
        st.tm_year = (int)atol(p) - 1900;
        if (len > 6)
        {
            st.tm_mon =  (int)atol(p + 5) - 1;
            if (len > 9)
            {
                st.tm_mday = (int)atol(p + 8);
                if (len > 12)
                {
                    st.tm_hour = (int)atol(p + 11);
                    if (len > 15)
                    {
                        st.tm_min = (int)atol(p + 14);
                        if (len > 18)
                        {
                            st.tm_sec = (int)atol(p + 17);
                            if (m_dec && len > 20)
                            {
                                char tmp[10] = { 0x00 };
                                strncpy(tmp, p + 20, (size_t)m_dec);
                                ms = atol(tmp);
                            }
                        }
                    }
                }
            }
        }
    }
    datetime = (__int64)mktime(&st); 
    return *this;
}

#ifdef _WIN32
wchar_t* myTimeStamp::toStr(wchar_t* p)
{
    if (datetime)
    {
        time_t v = (time_t)datetime;
        errno = 0;
        struct tm* st = localtime(&v);
        if (st == NULL)
        {
            //p[0] = 0x00;
            swprintf_s(p, 100, L"er:%d %s", errno,_tcserror(errno));
            return p;
        }
        if (m_dec)
            swprintf_s(p, 50, wdatetime_format_ms, st->tm_year + 1900,
                        st->tm_mon + 1, st->tm_mday, st->tm_hour, st->tm_min,
                        st->tm_sec, m_dec, (unsigned int)ms);
        else
            swprintf_s(p, 50, wdatetime_format, st->tm_year + 1900,
                        st->tm_mon + 1, st->tm_mday, st->tm_hour, st->tm_min,
                        st->tm_sec);
    }else
    {
        if (m_dec)
            swprintf_s(p, 50, wdatetime_format_ms, 0, 0, 0, 0, 0, 0, m_dec, 0);
        else
            swprintf_s(p, 50, wdatetime_format, 0, 0, 0, 0, 0, 0);
    }
    return p;
}

myTimeStamp& myTimeStamp::operator=(const wchar_t* p)
{
    size_t len = wcslen(p);
    struct tm st;
    memset(&st, 0, sizeof(tm));
    i64 = 0;
    if (len > 3)
    {
        st.tm_year = (int)_wtol(p) - 1900;
        if (len > 6)
        {
            st.tm_mon =  (int)_wtol(p + 5) - 1;
            if (len > 9)
            {
                st.tm_mday = (int)_wtol(p + 8);
                if (len > 12)
                {
                    st.tm_hour = (int)_wtol(p + 11);
                    if (len > 15)
                    {
                        st.tm_min = (int)_wtol(p + 14);
                        if (len > 18)
                        {
                            st.tm_sec = (int)_wtol(p + 17);
                            if (m_dec && len > 20)
                            {
                                wchar_t tmp[10] = { 0x00 };
                                wcsncpy(tmp, p + 20, (size_t)m_dec);
                                ms = _wtol(tmp);
                            }
                        }
                    }
                }
            }
        }
    }
    datetime = (__int64)mktime(&st); 
    return *this;
}
#endif



#pragma warning(default : 4996)    


} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

//#endif //MYDATETIME_H
