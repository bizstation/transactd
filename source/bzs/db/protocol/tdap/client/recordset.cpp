/*=================================================================
   Copyright (C) 2014 BizStation Corp All rights reserved.

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
recordset::recordset():m_imple(new recordsetImple){}

recordset::recordset(recordsetImple* p):m_imple(p)
{

}

recordset::~recordset()
{
	delete m_imple;
}

#ifndef BCB_64

void* recordset::operator new(size_t size)
{
	return malloc(size);
}

void recordset::operator delete(void* p)
{
	free(p);
}

#endif
/* This clone is deep copy.
   But text and blob field data memory are shared.
*/
recordset* recordset::clone() const
{
	return /*recordset::ptr(*/new recordset(m_imple->clone());//);
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

row& recordset::operator[](size_t index)
{
	return (*m_imple)[index];
}

row& recordset::first()
{
	return m_imple->first();
}

row& recordset::last()
{
	return m_imple->last();
}

size_t recordset::size()const
{
	return m_imple->size();
}

size_t recordset::count()const
{
	return m_imple->count();
}

recordset& recordset::top(recordset& c, int n)
{
	m_imple->top(*c.m_imple, n);
	return c;
}

recordset::iterator recordset::begin(){return m_imple->begin();}

recordset::iterator recordset::end(){return m_imple->end();}

recordset::iterator recordset::erase(size_t index){return m_imple->erase(m_imple->begin()+index);}

recordset::iterator recordset::erase(const iterator& it){return m_imple->erase(it);}

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

recordset& recordset::orderBy(const _TCHAR* name1 , const _TCHAR* name2,
				 const _TCHAR* name3,const _TCHAR* name4,
				 const _TCHAR* name5, const _TCHAR* name6,
				 const _TCHAR* name7, const _TCHAR* name8)
{
	m_imple->orderBy(name1, name2, name3, name4, name5, name6, name7, name8);
	return *this;
}

recordset& recordset::orderBy(fieldNames& fns)
{
	m_imple->orderBy(fns);
	return *this;
}

recordset& recordset::reverse()
{
	m_imple->reverse();
	return *this;
}

void recordset::appendCol(const _TCHAR* name, int type, short len)
{
	m_imple->appendCol(name, type, len);
}

recordset& recordset::operator+=(const recordset& r)
{
	m_imple->operator+=(*r.m_imple);
	return *this;
}


#ifdef _DEBUG
void recordset::dump()
{
	m_imple->dump();
}
#endif




}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs

