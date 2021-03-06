/*=================================================================
   Copyright (C) 2014-2016 BizStation Corp All rights reserved.

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
#pragma hdrstop

#include "recordset.h"
#include "recordsetImple.h"

#pragma package(smart_init)

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

// ---------------------------------------------------------------------------
// class recordset
// ---------------------------------------------------------------------------
recordset::recordset() : m_imple(new recordsetImple)
{
}

/* This  is deep copy.
   But text and blob field data memory are shared.
*/
recordset::recordset(const recordset& r) : m_imple(r.m_imple->clone())
{
}

recordset& recordset::operator=(const recordset& r)
{
    if (this != &r)
    {
        delete m_imple;
        m_imple = r.m_imple->clone();
    }
    return *this;
}

recordset* recordset::clone() const
{
    return new recordset(*this);
}

recordset::~recordset()
{
    delete m_imple;
}

void recordset::release()
{
    delete this;
}

void recordset::clearRecords()
{
    m_imple->clearRecords();
}

void recordset::clear()
{
    m_imple->clear();
}

const fielddefs* recordset::fieldDefs() const
{
    return m_imple->fieldDefs();
}

row& recordset::operator[](size_t index) const
{
    m_imple->checkIndex(index);
    m_imple->clearStringBuffer();
    return (*m_imple)[index];
}

row& recordset::first() const
{
    return m_imple->first();
}

row& recordset::last() const
{
    return m_imple->last();
}

size_t recordset::size() const
{
    return m_imple->size();
}

size_t recordset::count() const
{
    return m_imple->count();
}

recordset& recordset::top(recordset& c, int n) const
{
    m_imple->top(*c.m_imple, n);
    return c;
}

recordset::iterator recordset::begin()
{
    return m_imple->begin();
}

recordset::iterator recordset::end()
{
    return m_imple->end();
}

recordset::iterator recordset::erase(size_t index)
{
    m_imple->checkIndex(index);
    return m_imple->erase(m_imple->begin() + index);
}

recordset::iterator recordset::erase(const iterator& it)
{
    return m_imple->erase(it);
}

void recordset::removeField(int index)
{
    m_imple->removeField(index);
}

recordset& recordset::matchBy(recordsetQuery& rq)
{
    m_imple->matchBy(rq);
    return *this;
}

recordset& recordset::groupBy(groupQuery& gq)
{
    m_imple->groupBy(gq);
    return *this;
}

recordset& recordset::orderBy(const _TCHAR* name1, const _TCHAR* name2,
                              const _TCHAR* name3, const _TCHAR* name4,
                              const _TCHAR* name5, const _TCHAR* name6,
                              const _TCHAR* name7, const _TCHAR* name8)
{
    m_imple->orderBy(name1, name2, name3, name4, name5, name6, name7, name8);
    return *this;
}

recordset& recordset::orderBy(const sortFields& orders)
{
    m_imple->orderBy(orders);
    return *this;
}

recordset& recordset::reverse()
{
    m_imple->reverse();
    return *this;
}

recordset& recordset::join(const recordset& rs, recordsetQuery& rq)
{
    recordset* rst = rs.clone();
    m_imple->nestedLoopJoin(*rst->m_imple, rq, true);
    rst->release();
    return *this;
}

recordset& recordset::outerJoin(const recordset& rs, recordsetQuery& rq)
{
    recordset* rst = rs.clone();
    m_imple->nestedLoopJoin(*rst->m_imple, rq, false);
    rst->release();
    return *this;
}

void recordset::reserve(size_t size)
{
    m_imple->reserve(size);
}

void recordset::appendField(const _TCHAR* name, int type, short len)
{
    m_imple->appendField(name, type, len);
}

void recordset::appendField(const fielddef& fd)
{
	m_imple->appendField(fd);	
}

recordset& recordset::operator+=(const recordset& r)
{
    if (r.size() == 0)
        return *this;
    if ((size() == 0) && r.size())
    {
        *this = r;
        return *this;
    }
    m_imple->operator+=(*r.m_imple);
    return *this;
}

recordset* recordset::create()
{
    return new recordset();
}

#ifdef _DEBUG
void recordset::dump()
{
    m_imple->dump();
}
void recordset::dump(std::tostream& os)
{
    m_imple->dump(os);
}
#endif

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs
