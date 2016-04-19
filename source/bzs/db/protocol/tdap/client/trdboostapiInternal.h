#ifndef trdboostapiInternalH
#define trdboostapiInternalH
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
#include "table.h"
#include "database.h"
#include "dbDef.h"
#include "fields.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace bzs
{
namespace db
{
namespace protocol
{
namespace tdap
{
namespace client
{

template <int N> struct placeholder
{
    static boost::arg<N>& get()
    {
        static boost::arg<N> result;
        return result;
    }
};

static boost::arg<1>& fields_type = placeholder<1>::get();
inline static void readStatusCheck(table& tb, const _TCHAR* name)
{
    if ((tb.stat() != 0) && (tb.stat() != STATUS_EOF) &&
        (tb.stat() != STATUS_NOT_FOUND_TI))
        nstable::throwError(name, tb.stat());
}

template <class T>
void validateStatus(const T& obj, const _TCHAR* caption)
{
    if (obj->stat())
        nstable::throwError(caption, obj->stat());
}

class indexNavi
{

public:
    inline static void increment(table& tb, ushort_td lockBias)
    {
        tb.seekNext(lockBias);
        readStatusCheck(tb, _T("Seek next"));
    }

    inline static void decrement(table& tb, ushort_td lockBias)
    {
        tb.seekPrev(lockBias);
        readStatusCheck(tb, _T("Seek prev"));
    }
};


class indexRvNavi
{

public:
    inline static void increment(table& tb, ushort_td lockBias)
    {
        tb.seekPrev(lockBias);
        readStatusCheck(tb, _T("Seek next"));
    }

    inline static void decrement(table& tb, ushort_td lockBias)
    {
        tb.seekNext(lockBias);
        readStatusCheck(tb, _T("Seek prev"));
    }
};

class indexFindNavi
{

public:
    inline static void increment(table& tb, ushort_td /*lockBias*/)
    {
        tb.findNext();
        readStatusCheck(tb, _T("Find next"));
    }

    inline static void decrement(table& tb, ushort_td /*lockBias*/)
    {
        tb.findPrev();
        readStatusCheck(tb, _T("Find prev"));
    }
};

class indexRvFindNavi
{

public:
    inline static void increment(table& tb, ushort_td /*lockBias*/)
    {
        tb.findPrev();
        readStatusCheck(tb, _T("Find next"));
    }

    inline static void decrement(table& tb, ushort_td /*lockBias*/)
    {
        tb.findNext();
        readStatusCheck(tb, _T("Find prev"));
    }
};

class stepNavi
{

public:
    inline static void increment(table& tb, ushort_td lockBias)
    {
        tb.stepNext(lockBias);
        readStatusCheck(tb, _T("Step next"));
    }
    inline static void decrement(table& tb, ushort_td lockBias)
    {
        tb.stepPrev(lockBias);
        readStatusCheck(tb, _T("Step prev"));
    }
};

class stepRvNavi
{
   
public:
    inline static void increment(table& tb, ushort_td lockBias)
    {
        tb.stepPrev(lockBias);
        readStatusCheck(tb, _T("Step next"));
    }
    inline static void decrement(table& tb, ushort_td lockBias)
    {
        tb.stepNext(lockBias);
        readStatusCheck(tb, _T("Step prev"));
    }
};

template <class T0 = _TCHAR*, class T1 = _TCHAR*, class T2 = _TCHAR*,
          class T3 = _TCHAR*, class T4 = _TCHAR*, class T5 = _TCHAR*,
          class T6 = _TCHAR*, class T7 = _TCHAR*>
class keyValueSetter
{

public:
    template <class table_ptr>
    static void
    set(table_ptr tb, const char_td keynum, const T0 kv0, const T1 kv1 = NULL,
        const T2 kv2 = NULL, const T3 kv3 = NULL, const T4 kv4 = NULL,
        const T5 kv5 = NULL, const T6 kv6 = NULL, const T7 kv7 = NULL)
    {
        const tabledef& td = *tb->tableDef();
        if (keynum < td.keyCount)
        {
            tb->clearBuffer();
            const keydef kd = td.keyDefs[(int)keynum];
            if (kd.segmentCount > 0)
                tb->setFV(kd.segments[0].fieldNum, kv0);
            if (kd.segmentCount > 1)
                tb->setFV(kd.segments[1].fieldNum, kv1);
            if (kd.segmentCount > 2)
                tb->setFV(kd.segments[2].fieldNum, kv2);
            if (kd.segmentCount > 3)
                tb->setFV(kd.segments[3].fieldNum, kv3);
            if (kd.segmentCount > 4)
                tb->setFV(kd.segments[4].fieldNum, kv4);
            if (kd.segmentCount > 5)
                tb->setFV(kd.segments[5].fieldNum, kv5);
            if (kd.segmentCount > 6)
                tb->setFV(kd.segments[6].fieldNum, kv6);
            if (kd.segmentCount > 7)
                tb->setFV(kd.segments[7].fieldNum, kv7);
        }
        tb->setKeyNum(keynum);
    }
};

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#endif // trdboostapiInternalH
