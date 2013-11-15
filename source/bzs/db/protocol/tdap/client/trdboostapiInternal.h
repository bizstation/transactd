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
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/type_traits.hpp>

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



template<int N>
struct placeholder
{
    static boost::arg<N>& get()
    {
        static boost::arg<N>result;
        return result;
    }
};

static boost::arg<1>&fields_type = placeholder<1>::get();
inline static void readStatusCheck(table& tb, const _TCHAR* name)
{
    if ((tb.stat() != 0) && (tb.stat() != STATUS_EOF)
                    && (tb.stat() != STATUS_NOT_FOUND_TI))
        nstable::throwError(name, tb.stat());
}

class indexNavi
{

public:
    inline static void increment(table& tb)
    {
        tb.seekNext();
        readStatusCheck(tb, _T("Seek next"));

    }
    inline static void decrement(table& tb)
    {
        tb.seekPrev();
        readStatusCheck(tb, _T("Seek prev"));
    }

};

class indexRvNavi
{

public:
    inline static void increment(table& tb)
    {
        tb.seekPrev();
        readStatusCheck(tb, _T("Seek next"));

    }
    inline static void decrement(table& tb)
    {

        tb.seekNext();
        readStatusCheck(tb, _T("Seek prev"));
    }

};

class indexFindNavi
{

public:
    inline static void increment(table& tb)
    {
        tb.findNext();
        readStatusCheck(tb, _T("Find next"));

    }
    inline static void decrement(table& tb)
    {
        tb.findPrev();
        readStatusCheck(tb, _T("Find prev"));

    }

};

class indexRvFindNavi
{

public:
    inline static void increment(table& tb)
    {
        tb.findPrev();
        readStatusCheck(tb, _T("Find next"));

    }
    inline static void decrement(table& tb)
    {
        tb.findNext();
        readStatusCheck(tb, _T("Find prev"));

    }

};

class stepNavi
{

public:
    inline static void increment(table& tb)
    {
        tb.stepNext();
        readStatusCheck(tb, _T("Step next"));

    }
    inline static void decrement(table& tb)
    {
        tb.stepPrev();
        readStatusCheck(tb, _T("Step prev"));
    }

};

class stepRvNavi
{

public:
    inline static void increment(table& tb)
    {
        tb.stepPrev();
        readStatusCheck(tb, _T("Step next"));

    }
    inline static void decrement(table& tb)
    {
        tb.stepNext();
        readStatusCheck(tb, _T("Step prev"));
    }

};





}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs
#endif//trdboostapiInternalH
