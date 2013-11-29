/*=================================================================
   Copyright (C) 2000-2013 BizStation Corp All rights reserved.

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

#include "btrDate.h"
#include <bzs/rtl/datetime.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <bzs/env/compiler.h>
#ifdef LINUX
#include <bzs/env/mbcswchrLinux.h>
#endif

#pragma package(smart_init)

#undef USETLS
#if ((defined(_WIN32) && _MSC_VER) || __APPLE__)
#define USETLS
#endif


#ifdef USETLS
	extern tls_key g_tlsiID_SC2;
#else
	__THREAD _TCHAR __THREAD_BCB g_date2[45];
#endif

inline _TCHAR* databuf()
{
	#ifdef USETLS
		_TCHAR* p = (_TCHAR*)tls_getspecific(g_tlsiID_SC2);
		if (p == NULL)
		{
			p = (_TCHAR*)new wchar_t[45];
			tls_setspecific(g_tlsiID_SC2, p);
		}
		return p; 
	#else
		return g_date2;
	#endif
}


namespace bzs
{
	using namespace rtl;
namespace db
{
namespace protocol
{
namespace tdap
{




#define dateToi64(p) (((__int64)JDate2Num(p) - 1721426)*86400) + _timezone
#define timeToi64(t) t.ss+t.nn*60+t.hh*3600

__int64 btrTimeStamp::getDateTimeInt(int& time)
{
    // seconds from 0000-01-01 00:00:00
    __int64 sec = i64 / 10000000 - _timezone;
    time = sec % 86400;
    // 1721426 days differs from JDate2Num.
    __int64 days = sec / 86400 + 1721426;
    return days;
}


btrTimeStamp::btrTimeStamp(unsigned __int64 i):i64(i){};

btrTimeStamp::btrTimeStamp(const char* p)
{
    fromString(p);
}

btrTimeStamp::btrTimeStamp(btrDate d, btrTime t):i64(0)
{
	i64 += timeToi64(t);
    i64 += dateToi64(btrdtoa(d, (_TCHAR*)NULL));
    i64 *= 10000000;

}

#ifdef _WIN32
btrTimeStamp::btrTimeStamp(const wchar_t* p)
{
    fromString(p);
}

wchar_t* btrTimeStamp::toString(wchar_t* p)
{
    int time;
    int v = (int)getDateTimeInt(time);
    #ifdef _UNICODE
        swprintf_s(p, 20, _T("%s %02d:%02d:%02d"), JNum2Date(v), time/3600,time%3600/60,time%3600%60);
        return p;
    #else
        char buf[30];
        sprintf_s(buf, 30, _T("%s %02d:%02d:%02d"), JNum2Date(v), time/3600,time%3600/60,time%3600%60);
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buf, -1, p, 20);
        return p;
    #endif

}

void btrTimeStamp::fromString(const wchar_t* p)
{
    i64 = 0;
    const  wchar_t* p2 = wcsstr(p, L" ");
    
    if (p2)
    {
        btrTime t = atobtrt(p2+1);
        i64 += timeToi64(t);
    }
    i64 += dateToi64(p);
    i64 *= 10000000;
}
#endif //_WIN32

char* btrTimeStamp::toString(char* p)
{
    int time;
    int v = (int)getDateTimeInt(time);
    #ifndef _UNICODE
        sprintf_s(p, 20, _T("%s %02d:%02d:%02d"), JNum2Date(v), time/3600,time%3600/60,time%3600%60);
        return p;
    #else
        wchar_t buf[30];
        swprintf_s(buf, 30, _T("%s %02d:%02d:%02d"), JNum2Date(v), time/3600,time%3600/60,time%3600%60);
        wtoa(p, buf, 20);
        return p;
    #endif
}

void btrTimeStamp::fromString(const char* p)
{
    i64 = 0;
    const char* p2 = strstr(p, " ");

    if (p2)
    {
        btrTime t = atobtrt(p2+1);
        i64 += timeToi64(t);
    }
    i64 += dateToi64(p);
    i64 *= 10000000;
}



bdate::bdate(int date)
{
    m_date.i = date;
}

bdate::bdate(const _TCHAR* date)
{
    btrDate bt;
    bt.yy = (short)GetYear(date, false);
    bt.mm = (char)GetMonth(date, false);
    bt.dd = (char)bzs::rtl::GetDate(date);
    m_date = bt;
}

const _TCHAR* bdate::year_str()
{
    _TCHAR* p = databuf();
	_stprintf_s(p , 30, _T("%d"), (int)m_date.yy);
	return p;
}

const _TCHAR* bdate::month_str()
{
    _TCHAR* p = databuf();
	_stprintf_s(p, 30,_T("%d"), (int)m_date.mm);
    return p;
}

const _TCHAR* bdate::date_str()
{
    _TCHAR* p = databuf();
	_stprintf_s(p, 30, _T("%d"), (int)m_date.dd);
    return p;
}

const _TCHAR* bdate::c_str()
{
    return btrdtoa(m_date, (_TCHAR*)NULL);
}

const char* dateFormatString(const char*){return "%04d/%02d/%02d";}
const wchar_t* dateFormatString(const wchar_t*){return L"%04d/%02d/%02d";}

const char* dateFormatString_h(const char*){return "%04d-%02d-%02d";}
const wchar_t* dateFormatString_h(const wchar_t*){return L"%04d-%02d-%02d";}

const char* timeFormatString(const char*){return "%02d:%02d:%02d";}
const wchar_t* timeFormatString(const wchar_t*){return L"%02d:%02d:%02d";}

const char* timeFormatString_h(const char*){return "%02d-%02d-%02d";}
const wchar_t* timeFormatString_h(const wchar_t*){return L"%02d-%02d-%02d";}

template <class T>
size_t _tcslen_(const T* p);

template <> size_t _tcslen_(const char* p){return strlen(p);}
template <> size_t _tcslen_(const wchar_t* p){return wcslen(p);}


template <class T>
int _ttoi_(const T* p);

template <> int _ttoi_(const char* p){return atoi(p);}
template <> int _ttoi_(const wchar_t* p){return _wtoi(p);}

template <class T>
T* formatDate(T* p, const btrDate& d, bool type_vb);

template <> char* formatDate(char* p, const btrDate& d, bool type_vb)
{
    if (type_vb)
		sprintf_s(p, 11, dateFormatString_h(p), d.yy, d.mm, d.dd);
    else
		sprintf_s(p, 11, dateFormatString(p), d.yy, d.mm, d.dd);
    return p;
}

template <> wchar_t* formatDate(wchar_t* p, const btrDate& d, bool type_vb)
{
    if (type_vb)
		swprintf_s(p, 11, dateFormatString_h(p), d.yy, d.mm, d.dd);
    else
		swprintf_s(p, 11, dateFormatString(p), d.yy, d.mm, d.dd);
    return p;
}


template <class T>
T* formatTime(T* p, const btrTime& d, bool type_vb);

template <> char* formatTime(char* p, const btrTime& t, bool type_vb)
{
    if (type_vb)
		sprintf_s(p, 9, timeFormatString_h(p), t.hh, t.nn, t.ss);
    else
		sprintf_s(p, 9, timeFormatString(p), t.hh, t.nn, t.ss);
    return p;
}

template <> wchar_t* formatTime(wchar_t* p, const btrTime& t, bool type_vb)
{
    if (type_vb)
		swprintf_s(p, 9, timeFormatString_h(p), t.hh, t.nn, t.ss);
    else
		swprintf_s(p, 9, timeFormatString(p), t.hh, t.nn, t.ss);
    return p;
}

template <class T>
btrDate atobtrd(const T* date)
{
    btrDate bt;
    bt.i=0;
	if (date &&
		((_tcslen_(date)==10) || ((_tcslen_(date) > 10) && (date[10]== 'T' || date[10]== ' '))))
	{
		bt.yy = (short)_ttoi_(date);
        bt.mm = (char)_ttoi_(date + 5);
		bt.dd = (char)_ttoi_(date + 8);
    }
	return bt;

}


template <class T>
const T* btrdtoa(const btrDate& d, T* retbuf, bool type_vb)
{
    T* p = retbuf;
	if (p == NULL)
        p = (T*)databuf();
    return formatDate(p, d, type_vb);
}

template <class T>
const T* btrttoa(const btrTime& t, T* retbuf, bool type_vb)
{
	T* p = retbuf;
	if (p == NULL)
		p = (T*)databuf();
	return formatTime(p, t, type_vb);
}

template <class T>
btrTime atobtrt(const T* p)
{
	btrTime retVal;
	retVal.i = 0;
	if (p && (_tcslen_(p) == 8))
	{
		retVal.hh = (char)_ttoi_(p);
		retVal.nn = (char)_ttoi_(p + 3);
		retVal.ss = (char)_ttoi_(p + 6);
    }else
        retVal.i = 0;
    return retVal;
}

btrDate atobtrd(const char* p){return atobtrd<char>(p);}

const char* btrdtoa(const btrDate& d, char* retbuf, bool type_vb)
{
    return btrdtoa<char>(d, retbuf, type_vb);
}

const char* btrttoa(const btrTime& t, char* retbuf, bool type_vb)
{
    return btrttoa<char>(t, retbuf, type_vb);
}

btrTime atobtrt(const char* p){return atobtrt<char>(p);}


#ifdef _WIN32
const wchar_t* btrdtoa(const btrDate& d, wchar_t* retbuf, bool type_vb)
{
    return btrdtoa<wchar_t>(d, retbuf, type_vb);
}

const wchar_t* btrttoa(const btrTime& t, wchar_t* retbuf, bool type_vb)
{
    return btrttoa<wchar_t>(t, retbuf, type_vb);
}

btrDate atobtrd(const wchar_t* p){return atobtrd<wchar_t>(p);}

btrTime atobtrt(const wchar_t* p){return atobtrt<wchar_t>(p);}

#endif

const _TCHAR* btrstoa(const btrDateTime& s, _TCHAR* retbuf, bool type_vb)
{
	_TCHAR * p= retbuf;
	const btrDate& d = s.date;
	const btrTime& t = s.time;
	if (p==NULL)
		p = databuf();;
	if (type_vb)
		_stprintf_s(p, 21, _T("%04d-%02d-%02dT%02d:%02d:%02d"), d.yy, d.mm, d.dd, t.hh, t.nn, t.ss);
	else
		_stprintf_s(p, 21, _T("%04d/%02d/%02d %02d:%02d:%02d"), d.yy, d.mm, d.dd, t.hh, t.nn, t.ss);
	return p;
}

btrDateTime atobtrs(const _TCHAR* p)
{
	btrDateTime s;
	s.i64 = 0;
	s.date = atobtrd(p);
	const _TCHAR* tmp = _tcsstr(p, _T("T"));
	if (tmp)
		s.time = atobtrt(tmp + 1);
	else if ((tmp = _tcsstr(p, _T(" ")))!=NULL)
		s.time = atobtrt(tmp + 1);
	return s;
}



int getNowDate()
{
	btrDate dtb;
#ifdef _WIN32
	SYSTEMTIME pst;
    GetLocalTime(&pst);

    dtb.yy = (short)pst.wYear;
    dtb.mm = (char)(pst.wMonth);
    dtb.dd = (char)pst.wDay;
#else
	//not _WIN32
	struct tm* date;
	time_t now;
	time(&now);
	struct tm tmp;
	date = &tmp;
	localtime_x(date, &now);

	dtb.yy = (short)date->tm_year + 1900;
    dtb.mm = (char)date->tm_mon + 1;
    dtb.dd = (char)date->tm_mday;
#endif //NOT _WIN32
	return dtb.i;

}

int getNowTime()
{
	btrTime tmb;
#ifdef _WIN32

	tmb.i = 0;
	SYSTEMTIME pst;
    GetLocalTime(&pst);
    tmb.hh = (char)pst.wHour;    tmb.nn = (char)pst.wMinute;
    tmb.ss = (char)pst.wSecond;
	tmb.uu = (char)(pst.wMilliseconds/10);
#else
	struct tm* date;
	time_t now;
	time(&now);
	struct tm tmp;
	date = &tmp;
	localtime_x(date, &now);
 	tmb.hh = (char)date->tm_hour;
 	tmb.nn = (char)date->tm_min;
    tmb.ss = (char)date->tm_sec;
	tmb.uu = 00;
#endif
    return tmb.i;
}


}//namespace tdap
}//namespace protocol
}//namespace db
}//namespace bzs
