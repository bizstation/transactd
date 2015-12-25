#ifndef BZS_DB_PROTOCOL_TDAP_MYDATETIME_H
#define BZS_DB_PROTOCOL_TDAP_MYDATETIME_H
/*=================================================================
   Copyright (C) 2015 BizStation Corp All rights reserved.

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
#include <tchar.h>
#include <bzs/db/protocol/tdap/btrDate.h>

namespace bzs
{
namespace db
{
namespace protocol
{
namespace tdap
{


#pragma pack(push, 1)
pragma_pack1;

struct PACKAGE myDate
{
    union
    {
	    struct
	    {
	        unsigned int dd : 5;
	        unsigned int mm : 4;
	        unsigned int yy : 15;
	        unsigned int tmp : 8;
	    };
	    int i;
	};
    inline myDate() {};
    inline myDate(int /*size*/) {};
    void setValue(int v, bool btrvValue = false);
    int getValue(bool btrvValue = false);
    char* toStr(char* p, bool btrvValue = false);
    myDate& operator=(const char* p);
#ifdef _WIN32
    wchar_t* toStr(wchar_t* p, bool btrvValue = false);
    myDate& operator=(const wchar_t* p);
#endif
    __int64 internalValue() const { return i; }
    void setInternalValue(__int64 v) { i = (int)v; }
};

struct PACKAGE myTime
{

private:
    int m_dec;

public:
    union
    {
        struct
        {
            unsigned __int64 ms : 24;
            unsigned __int64 ss : 6;
            unsigned __int64 nn : 6;
            unsigned __int64 hh : 10;
            unsigned __int64 unused : 1;
            unsigned __int64 sign : 1;
            unsigned __int64 tmp : 16;
        };
        __int64 i64;
    };

public:
    inline myTime(int size) : m_dec((size - 3) * 2){};
    void setValue(__int64 v, bool btrvValue = false);
    __int64 getValue(bool btrvValue = false);
    char* toStr(char* p);
    myTime& operator=(const char* p);
#ifdef _WIN32
    wchar_t* toStr(wchar_t* p);
    myTime& operator=(const wchar_t* p);
#endif
    __int64 internalValue() const { return i64; }
    void setInternalValue(__int64 v) { i64 = v; }
};

struct PACKAGE myDateTime
{
private:
    int m_dec;

public:
    union
    {
        struct
        {
            unsigned __int64 ms : 24;
            unsigned __int64 ss : 6;
            unsigned __int64 nn : 6;
            unsigned __int64 hh : 5;
            unsigned __int64 dd : 5;
            unsigned __int64 yymm : 17; // yy*13+mm   (yy 0-9999, mm 0-12)
            unsigned __int64 sign : 1;  //always 1 0 is reserved
        };
        __int64 i64;
    };

    inline myDateTime(int size) : m_dec((size - 5) * 2){};
    void setValue(__int64 v);
    __int64 getValue();
    inline char* toStr(char* p) const{ return dateTime_str(p, m_dec); }
    myDateTime& operator=(const char* p);
    char* date_str(char* p) const;
    char* time_str(char* p, int decimals = 0) const;
    void setTime(const char* p);

    char* dateTime_str(char* p, int decimals = 0) const;
#ifdef _WIN32
    inline wchar_t* toStr(wchar_t* p) const { return dateTime_str(p, m_dec); }
    myDateTime& operator=(const wchar_t* p) ;
    wchar_t* date_str(wchar_t* p) const;
    wchar_t* time_str(wchar_t*, int decimals = 0) const;
    void setTime(const wchar_t* p);

    wchar_t* dateTime_str(wchar_t*, int decimals = 0) const;
#endif
    __int64 internalValue() const { return i64; }
    void setInternalValue(__int64 v) { i64 = v; }
};

struct PACKAGE myTimeStamp
{
private:
    int m_dec;

public:
    union
    {
        struct
        {
            unsigned __int64 ms : 24;
            unsigned __int64 datetime : 32;
            unsigned __int64 tmp : 8;
        };
        __int64 i64;
    };

    inline myTimeStamp(int size) : m_dec((size - 4) * 2){};
    void setValue(__int64 v);
    __int64 getValue();
    char* toStr(char* p);
    myTimeStamp& operator=(const char* p);

#ifdef _WIN32
    wchar_t* toStr(wchar_t* p);
    myTimeStamp& operator=(const wchar_t* p);
#endif
    __int64 internalValue() const { return i64; }
    void setInternalValue(__int64 v) { i64 = v; }
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

template <class T>
__int64 getLittleEndianValue(int len, __int64 bigendianValue)
{
    T t(len);
    t.setValue(bigendianValue);
    return t.internalValue();
}

template <class T>
__int64 getBigEndianValue(int len, __int64 ltendianValue)
{
    T t(len);
    t.setInternalValue(ltendianValue);
    return t.getValue();
}

#pragma warning(disable : 4244)
template <class T, typename CHAR>
const CHAR* date_time_str(int len, __int64 bigendianValue, CHAR* buf)
{
    T t(len);
    t.setValue(bigendianValue);
    return t.toStr(buf);
}

template <class T, class T2>
inline __int64 str_to_64(int len, const T2* data)
{
    T t(len);
    t = data;
    return t.getValue();
}

#pragma warning(default : 4244)



#pragma pack(pop)
pragma_pop;

} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#endif //BZS_DB_PROTOCOL_TDAP_MYDATETIME_H