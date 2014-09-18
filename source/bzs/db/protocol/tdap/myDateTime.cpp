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
#include <bzs/env/tstring.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <bzs/db/protocol/tdap/btrDate.h>


namespace bzs
{
namespace db
{
namespace protocol
{
namespace tdap
{

#pragma option -a-
pragma_pack1


#ifdef _WIN32

const wchar_t wtime_format_ms[] = L"%02d:%02d:%02d.%u";
const wchar_t wtime_format[] = L"%02d:%02d:%02d";
const wchar_t wdatetime_format_ms[] = L"%04d-%02d-%02d %02d:%02d:%02d.%u";
const wchar_t wdatetime_format[] = L"%04d-%02d-%02d %02d:%02d:%02d";
const wchar_t wdatetime_format_ms_i[] = L"%04d-%02d-%02d %02d:%02d:%02d.%u";
const wchar_t wdatetime_format_i[] = L"%04d-%02d-%02d %02d:%02d:%02d";

inline size_t _ttol_(const wchar_t* v){return _wtol(v);}
inline size_t _tcslen_(const wchar_t* v){return wcslen(v);}
inline wchar_t* _tcsncpy_(wchar_t* d, const wchar_t* s, size_t n){return wcsncpy(d,s,n);}

#endif

const char time_format_ms[] = "%02d:%02d:%02d.%u";
const char time_format[] = "%02d:%02d:%02d";
const char datetime_format_ms[] = "%04d-%02d-%02d %02d:%02d:%02d.%u";
const char datetime_format[] = "%04d-%02d-%02d %02d:%02d:%02d";
const char datetime_format_ms_i[] = "%04d-%02d-%02d %02d:%02d:%02d.%u";
const char datetime_format_i[] = "%04d-%02d-%02d %02d:%02d:%02d";

inline size_t _ttol_(const char* v){return atol(v);}
inline size_t _tcslen_(const char* v){return strlen(v);}
inline char* _tcsncpy_(char* d, const char* s, size_t n){return strncpy(d,s,n);}


#pragma warning(disable:4996) 
union myDate
{
	struct
	{
		unsigned int     dd :5;
		unsigned int     mm :4;
		unsigned int     yy :15;
		unsigned int     tmp:8;
	};
	
	int i;
	
	inline void setValue(int v, bool btrvValue = false)
	{
		if (btrvValue)
		{
			btrDate btrd;
			btrd.i = v;
			yy = btrd.yy;
			mm = btrd.mm;
			dd = btrd.dd;
			tmp=0;
		}
		else
			i = v;
	}
	
	inline int getValue(bool btrvValue = false)
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
	
	inline char* toStr(char* p, bool btrvValue)
	{
		if (btrvValue)
			sprintf(p, "%04d/%02d/%02d",yy, mm, dd);
		else
		    sprintf(p, "%04d-%02d-%02d",yy, mm, dd);
		return p;
	}

#ifdef _WIN32
	inline wchar_t* toStr(wchar_t* p, bool btrvValue)
	{
		if (btrvValue)
			swprintf_s(p, 11, L"%04d/%02d/%02d",yy, mm, dd);
		else
			swprintf_s(p, 11, L"%04d-%02d-%02d",yy, mm, dd);
		return p;
	}
#endif

	template <class T>
	inline myDate& operator=(const T* p)
	{
		tmp = 0;
		yy = _ttol_(p);
		mm = _ttol_(p+5);
		dd = _ttol_(p+8);
		return *this;
	}
	
};


struct myTime
{

private:
	int m_dec;
	
public:
	union
	{
		struct
		{
			unsigned __int64 ms		:24;
			unsigned __int64 ss		:6;
			unsigned __int64 nn		:6;
			unsigned __int64 hh		:10;
			unsigned __int64 unused	:1;
			unsigned __int64 sign	:1;
			unsigned __int64 tmp	:16;

		};
		__int64 i64;
	};
	
public:
	inline myTime(int size):m_dec((size - 3)*2){};

	inline void setValue(__int64 v, bool btrvValue = false)
	{

		if (btrvValue)
		{
			btrTime btrt;
			btrt.i = (int)v;
			hh = btrt.hh;
			nn = btrt.nn;
			ss = btrt.ss;
			ms = btrt.uu * 10000;
			tmp=0;
			sign = 1;
		    unused = 0;
			return;
		}
		char* p=(char*)&i64;
		char* src=(char*)&v;
		if (m_dec)
		{
			p[0] = src[5];
			p[1] = src[4];
			p[2] = src[3];
			ms = ms >> (3-m_dec/2)*8;
		}
		p[3] = src[2];
		p[4] = src[1];
		p[5] = src[0];
	}
	
	inline __int64 getValue( bool btrvValue = false)
	{
		__int64 v = 0;
		char* src=(char*)&i64;
		char* p=(char*)&v;
		p[2] = src[3];
		p[1] = src[4];
		p[0] = src[5];
		if (m_dec)
		{
			ms = ms << (3-m_dec/2)*8;
			p[3] = src[2];
			p[4] = src[1];
			p[5] = src[0];
		}
		if (btrvValue)
		{
			btrTime btrt;
		 	btrt.hh = hh;
			btrt.nn = nn;
			btrt.ss = ss;
			btrt.uu = (char)(ms/100000);
			return btrt.i;
		}
		return v;
	}
	
	inline char* toStr(char* p)
	{
		if (m_dec)
			sprintf(p, time_format_ms, (int)hh, (int)nn, (int)ss, (unsigned int)ms);
		else
			sprintf(p, time_format, (int)hh, (int)nn, (int)ss);
		return p;
	}

#ifdef _WIN32
	inline wchar_t* toStr(wchar_t* p)
	{
		if (m_dec)
			swprintf_s(p, 17, wtime_format_ms, (int)hh, (int)nn, (int)ss, (unsigned int)ms);
		else
			swprintf_s(p, 9, wtime_format, (int)hh, (int)nn, (int)ss);
		return p;
	}
#endif

	template <class T>
	inline myTime& operator=(const T* p)
	{
		sign = 1;
		unused = 0;
		ms = 0;
		hh = _ttol_(p);
		nn = _ttol_(p+3);
		ss = _ttol_(p+6);
		if (m_dec && _tcslen_(p)>9)
		{
			T tmp[10]={0x00};
			_tcsncpy_(tmp, p+9, (size_t)m_dec);
			ms = _ttol_(tmp);
		}
		return *this;
	}
};


struct myDateTime
{
private:
	int m_dec;
	
public:
	union
	{
		struct
		{
			unsigned __int64 ms		:24;
			unsigned __int64 ss		:6;
			unsigned __int64 nn		:6;
			unsigned __int64 hh		:5;
			unsigned __int64 dd		:5;
			unsigned __int64 yymm	:17; //yy*13+mm   (yy 0-9999, mm 0-12)
			unsigned __int64 sign	:1;
		};
		__int64 i64;
	};
	
	inline myDateTime(int size):m_dec((size - 5)*2){};
	
	inline void setValue(__int64 v)
	{
		char* p=(char*)&i64;
		char* src=(char*)&v;
		if (m_dec)
		{
			p[0] = src[7];
			p[1] = src[6];
			p[2] = src[5];
			ms = ms >> (3-m_dec/2)*8;
		}
		p[3] = src[4];
		p[4] = src[3];
		p[5] = src[2];
		p[6] = src[1];
		p[7] = src[0];

	}
	
	inline __int64 getValue()
	{
		__int64 v = 0;
		char* src=(char*)&i64;
		char* p=(char*)&v;
		p[4] = src[3];
		p[3] = src[4];
		p[2] = src[5];
		p[1] = src[6];
		p[0] = src[7];
		if (m_dec)
		{
			ms = ms << (3-m_dec/2)*8;
			p[5] = src[2];
			p[6] = src[1];
			p[7] = src[0];
		}
		return v;
	}
	
	inline char* toStr(char* p)
	{
		if (m_dec)
			sprintf(p, datetime_format_ms ,(int)(yymm/13),(int)(yymm%13), (int)dd, (int)hh, (int)nn, (int)ss,(unsigned int) ms);
		else
			sprintf(p, datetime_format, (int)(yymm/13),(int)(yymm%13), (int)dd, (int)hh, (int)nn, (int)ss);
		return p;
	}
	
#ifdef _WIN32
	inline wchar_t* toStr(wchar_t* p)
	{
		if (m_dec)
			swprintf_s(p, 26, wdatetime_format_ms, (int)(yymm/13), (int)(yymm%13), (int)dd, (int)hh, (int)nn, (int)ss, (unsigned int)ms);
		else
			swprintf_s(p, 20, wdatetime_format, (int)(yymm/13), (int)(yymm%13), (int)dd, (int)hh, (int)nn, (int)ss);
		return p;
	}
#endif

	template <class T>
	myDateTime& operator=(const T* p)
	{
		sign = 1;
		ms = 0 ;
		yymm = _ttol_(p)*13+_ttol_(p+5);
		dd = _ttol_(p+8);
		hh = _ttol_(p+11);
		nn = _ttol_(p+14);
		ss = _ttol_(p+17);
		if (m_dec && _tcslen_(p)>20)
		{
			T tmp[10]={0x00};
			_tcsncpy_(tmp, p+20, (size_t)m_dec);
			ms = _ttol_(tmp);
		}
		return *this;
	}

};

struct myTimeStamp
{
private:
	int m_dec;
	
public:
	union
	{
		struct
		{
			unsigned __int64 ms		 :24;
			unsigned __int64 datetime:32;
			unsigned __int64 tmp     :8;
		};
		__int64 i64;
	};
	
	inline myTimeStamp(int size):m_dec((size - 4)*2){};
	
	inline void setValue(__int64 v)
	{
		char* p=(char*)&i64;
		char* src=(char*)&v;
		if (m_dec)
		{
			p[0] = src[6];
			p[1] = src[5];
			p[2] = src[4];
			ms = ms >> (3-m_dec/2)*8;
		}
		p[3] = src[3];
		p[4] = src[2];
		p[5] = src[1];
		p[6] = src[0];
	}
	
	inline __int64 getValue()
	{
		__int64 v = 0;
		char* src=(char*)&i64;
		char* p=(char*)&v;

		p[3] = src[3];
		p[2] = src[4];
		p[1] = src[5];
		p[0] = src[6];
		if (m_dec)
		{
			ms = ms << (3-m_dec/2)*8;
			p[4] = src[2];
			p[5] = src[1];
			p[6] = src[0];
		}
		return v;
	}
	
#ifdef _WIN32
	inline wchar_t* toStr(wchar_t* p)
	{
		struct tm* st;
		time_t v = (time_t)datetime;
		st = localtime(&v);

		if (m_dec)
			swprintf_s(p, 26, wdatetime_format_ms_i
					,st->tm_year+1900, st->tm_mon+1, st->tm_mday
					,st->tm_hour, st->tm_min, st->tm_sec, (unsigned int)ms);
		else
			swprintf_s(p, 20, wdatetime_format_i
					,st->tm_year+1900, st->tm_mon+1, st->tm_mday
					,st->tm_hour, st->tm_min, st->tm_sec);
		return p;
	}
#endif

	inline char* toStr(char* p)
	{
		struct tm* st;
		time_t v = (time_t)datetime;
		st = localtime(&v);

		if (m_dec)
			sprintf(p, datetime_format_ms_i
					,st->tm_year+1900, st->tm_mon+1, st->tm_mday
					,st->tm_hour, st->tm_min, st->tm_sec, (unsigned int)ms);
		else
			sprintf(p, datetime_format_i
					,st->tm_year+1900, st->tm_mon+1, st->tm_mday
					,st->tm_hour, st->tm_min, st->tm_sec);
		return p;
	}

};
inline int btrdateToMydate(int btrd)
{
	myDate myd;
	myd.setValue(btrd, true);
	return myd.getValue(true);
}

inline __int64 btrtimeToMytime(int btrt)
{
	myTime myt(4);
	myt.setValue(btrt, true);
	return myt.getValue(true);
}

inline int mydateToBtrdate(int mydate)
{
	myDate myd;
	myd.setValue(mydate);
	return myd.getValue(true);
}

inline int mytimeToBtrtime(__int64 mytime, int size)
{
	myTime myt(size);
	myt.setValue(mytime);
	return (int)myt.getValue(true);
}
#pragma warning(default:4996) 


#pragma option -a
pragma_pop

}//namespace tdap
}//namespace protocol
}//namespace db
}//namespace bzs

//#endif //MYDATETIME_H
