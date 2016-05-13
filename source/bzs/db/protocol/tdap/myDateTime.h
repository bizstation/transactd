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
#include <bzs/env/compiler.h>
#include <stddef.h>

namespace bzs
{
namespace db
{
namespace protocol
{
namespace tdap
{
/** @cond INTERNAL */

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
    inline myDate(int /*dec*/, bool /*bigendian*/) {};
    void setValue(int v, bool btrDate_i = false);
    int getValue(bool btrvValue = false);
    char* toString(char* p, size_t size, bool w3_format = false);
    myDate& operator=(const char* p);
#ifdef _WIN32
    wchar_t* toString(wchar_t* p, size_t size, bool w3_format = false);
    myDate& operator=(const wchar_t* p);
#endif
    inline __int64 internalValue() const { return i; }
    inline void setInternalValue(__int64 v) { i = (int)v; }
};

struct PACKAGE myTime
{

protected:
    int m_dec;
    bool m_bigendian;
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
    inline myTime(int dec, bool bigendian) : m_dec(dec), 
        m_bigendian(bigendian){};
    virtual void setValue(__int64 v, bool btrTime_i = false);
    virtual __int64 getValue(bool btrTime_i = false);
    char* toString(char* p, size_t size);
    myTime& operator=(const char* p);
#ifdef _WIN32
    wchar_t* toString(wchar_t* p, size_t size);
    myTime& operator=(const wchar_t* p);
#endif
    inline __int64 internalValue() const { return i64; }
    inline void setInternalValue(__int64 v) { i64 = v; }
};

struct PACKAGE maTime : public  myTime
{
    virtual void setValue(__int64 v, bool btrTime_i = false);
    virtual __int64 getValue(bool btrTime_i = false);
public:
    inline maTime(int dec, bool bigendian) : myTime(dec, bigendian){}
    maTime& operator=(const char* p);
#ifdef _WIN32
    maTime& operator=(const wchar_t* p) ;
#endif
};

struct PACKAGE myDateTime
{
protected:
    int m_dec;
    bool m_bigendian;
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

    inline myDateTime(int dec, bool bigendian) : m_dec(dec), m_bigendian(bigendian){};
    virtual void setValue(__int64 v);
    virtual __int64 getValue();
    char* toString(char* p, size_t size) const;
    myDateTime& operator=(const char* p);
    char* dateStr(char* p, size_t size) const;
    char* timeStr(char* p, size_t size) const;
    void setTime(const char* p);

#ifdef _WIN32
    wchar_t* toString(wchar_t* p, size_t size) const ;
    myDateTime& operator=(const wchar_t* p) ;
    wchar_t* dateStr(wchar_t* p, size_t size) const;
    wchar_t* timeStr(wchar_t*, size_t size) const;
    void setTime(const wchar_t* p);
#endif
    inline __int64 internalValue() const { return i64; }
    inline void setInternalValue(__int64 v) { i64 = v; }
};

struct PACKAGE maDateTime : public  myDateTime
{
    virtual void setValue(__int64 v);
    virtual __int64 getValue();
public:
    inline maDateTime(int dec, bool bigendian) : myDateTime(dec, bigendian){};
    maDateTime& operator=(const char* p);
#ifdef _WIN32
    maDateTime& operator=(const wchar_t* p) ;
#endif
};

struct PACKAGE myTimeStamp
{
private:
    int m_dec;
    bool m_bigendian;
protected:
    bool m_mariadb;
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

    inline myTimeStamp(int dec, bool bigendian) : m_dec(dec), 
    	m_bigendian(bigendian), m_mariadb(false){};
    void setValue(__int64 v);
    __int64 getValue();
    char* toString(char* p, size_t size);
    myTimeStamp& operator=(const char* p);
    char* dateStr(char* p, size_t size) const;
    char* timeStr(char* p, size_t size) const;

#ifdef _WIN32
    wchar_t* toString(wchar_t* p, size_t size);
    myTimeStamp& operator=(const wchar_t* p);
    wchar_t* dateStr(wchar_t* p, size_t size) const;
    wchar_t* timeStr(wchar_t* p, size_t size) const;
#endif
    inline __int64 internalValue() const { return i64; }
    inline void setInternalValue(__int64 v) { i64 = v; }
};


struct PACKAGE maTimeStamp : public myTimeStamp
{

    inline maTimeStamp (int dec, bool bigendian) : myTimeStamp(dec, bigendian)
    {
        m_mariadb = true;
    }
    inline maTimeStamp& operator=(const char* p){ myTimeStamp::operator=(p); return *this;}
#ifdef _WIN32
    inline maTimeStamp& operator=(const wchar_t* p) { myTimeStamp::operator=(p); return *this; }
#endif
};

inline int btrdateToMydate(int btrd)
{
    myDate myd;
    myd.setValue(btrd, true);
    return myd.i;
}

// Do not work at maridab
inline __int64 btrtimeToMytime(int btrTime_i, bool bigendian)
{
    myTime myt(4, bigendian);
    myt.setValue(btrTime_i, true);
    return myt.i64;
}

inline int mydateToBtrdate(int myDate_i)
{
    myDate myd;
    myd.i = myDate_i;
    return myd.getValue(true);
}

inline int mytimeToBtrtime(__int64 myTime_i, bool bigendian, int dec)
{
    myTime myt(dec, bigendian);
    myt.i64 = myTime_i;
    return (int)myt.getValue(true);
}

template <class T>
__int64 getInternalValue(int dec, bool bigendian, __int64 value)
{
    T t(dec, bigendian);
    t.setValue(value);
    return t.internalValue();
}

template <class T>
__int64 getStoreValue(int dec, bool bigendian, __int64 value)
{
    T t(dec, bigendian);
    t.setInternalValue(value);
    return t.getValue();
}

#pragma warning(disable : 4244)
template <class T, typename CHAR>
const CHAR* date_time_str(int dec, bool bigendian, __int64 value, CHAR* buf, size_t size)
{
    T t(dec, bigendian);
    t.setValue(value);
    return t.toString(buf, size);
}

template <class T, class T2>
inline __int64 str_to_64(int dec, bool bigendian, const T2* data)
{
    T t(dec, bigendian);
    t = data;
    return t.getValue();
}

#pragma warning(default : 4244)



#pragma pack(pop)
pragma_pop;
/** @endcond */

} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#endif //BZS_DB_PROTOCOL_TDAP_MYDATETIME_H