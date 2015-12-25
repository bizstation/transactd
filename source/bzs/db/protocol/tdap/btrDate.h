#ifndef BZS_DB_PROTOCOL_TDAP_BTRDATE_H
#define BZS_DB_PROTOCOL_TDAP_BTRDATE_H
/* =================================================================
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
 ================================================================= */

#include <bzs/env/compiler.h>
#include <bzs/env/crosscompile.h>

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

#ifdef SWIG
/* For swig interface
   export field names.
*/
union btrDate
{
    char dd;
    char mm;
    short yy;
    int i;
};

union btrTime
{
    char uu;
    char ss;
    char nn;
    char hh;
    int i;
};

union btrDateTime
{
    btrTime time;
    btrDate date;
    __int64 i64;
};

#else

union btrDate
{
    struct
    {
        char dd;
        char mm;
        short yy;
    };

    int i;
};

union btrTime
{
    struct
    {
        char uu;
        char ss;
        char nn;
        char hh;
    };

    int i;
};

union btrDateTime
{
    struct
    {
        btrTime time;
        btrDate date;
    };

    __int64 i64;
};

#endif // SWIG

#pragma pack(pop)
pragma_pop;

class PACKAGE btrTimeStamp
{

    __int64 getDateTimeInt(int& time);

public:
    unsigned __int64 i64;

    explicit btrTimeStamp(unsigned __int64 i);
    explicit btrTimeStamp(const char* p);
    btrTimeStamp(btrDate d, btrTime t);
#ifdef _WIN32
    explicit btrTimeStamp(const wchar_t* p);
    wchar_t* toString(wchar_t* retbuf);
    void fromString(const wchar_t* p);
#endif
    char* toString(char* retbuf);
    void fromString(const char* p);
};

// Class bdate is for btrDate

class PACKAGE bdate
{
    btrDate m_date;

public:
    explicit bdate(int btrDate);
    explicit bdate(const _TCHAR* date);
    const _TCHAR* year_str();
    const _TCHAR* month_str();
    const _TCHAR* date_str();

    int year() { return m_date.yy; };

    int date() { return m_date.dd; };

    int month() { return m_date.mm; };

    btrDate btr_date() { return m_date; };
    const _TCHAR* c_str();
};

PACKAGE btrDate atobtrd(const char* date);

PACKAGE const char* btrdtoa(const btrDate& d, char* retbuf,
                            bool type_vb = false);

PACKAGE const char* btrttoa(const btrTime& t, char* retbuf,
                            bool type_vb = false);

PACKAGE btrTime atobtrt(const char* p);

PACKAGE btrDateTime atobtrs(const char* p);

PACKAGE const char* btrstoa(const btrDateTime& d, char* retbuf,
                              bool type_vb = false);

inline const char* btrdtoa(int date, char* retbuf, bool type_vb = false)
{
    btrDate d;
    d.i = date;
    return btrdtoa(d, retbuf, type_vb);
}

inline const char* btrttoa(int time, char* retbuf, bool type_vb = false)
{
    btrTime t;
    t.i = time;
    return btrttoa(t, retbuf, type_vb);
}

inline const char* btrstoa(__int64 datetime, char* retbuf, bool type_vb = false)
{
    btrDateTime t;
    t.i64 = datetime;
    return btrstoa(t, retbuf, type_vb);
}

#ifndef SWIG
#ifdef _WIN32
PACKAGE btrDate atobtrd(const wchar_t* date);

PACKAGE const wchar_t* btrdtoa(const btrDate& d, wchar_t* retbuf,
                               bool type_vb = false);

PACKAGE const wchar_t* btrttoa(const btrTime& t, wchar_t* retbuf,
                               bool type_vb = false);

PACKAGE btrTime atobtrt(const wchar_t* p);

PACKAGE btrDateTime atobtrs(const wchar_t* p);

PACKAGE const wchar_t* btrstoa(const btrDateTime& d, wchar_t* retbuf,
                              bool type_vb = false);

inline const wchar_t* btrdtoa(int date, wchar_t* retbuf, bool type_vb = false)
{
    btrDate d;
    d.i = date;
    return btrdtoa(d, retbuf, type_vb);
}

inline const wchar_t* btrttoa(int time, wchar_t* retbuf, bool type_vb = false)
{
    btrTime t;
    t.i = time;
    return btrttoa(t, retbuf, type_vb);
}

inline const wchar_t* btrstoa(__int64 datetime, wchar_t* retbuf, bool type_vb = false)
{
    btrDateTime t;
    t.i64 = datetime;
    return btrstoa(t, retbuf, type_vb);
}

#endif
#endif




inline const _TCHAR* c_str(const btrDate& d)
{
    return btrdtoa(d, (_TCHAR*)NULL);
}

inline const _TCHAR* c_str(const btrTime& d)
{
    return btrttoa(d, (_TCHAR*)NULL);
}

inline const _TCHAR* c_str(const btrDateTime& d, bool type_vb = false)
{
    return btrstoa(d, (_TCHAR*)NULL, type_vb);
}

PACKAGE int getNowDate();
PACKAGE int getNowTime();

} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs
#endif // BZS_DB_PROTOCOL_TDAP_BTRDATE_H
