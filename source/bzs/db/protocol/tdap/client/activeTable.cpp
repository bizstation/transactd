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

#include "activeTable.h"
#include "activeTableImple.h"

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

query* query::create()
{
    return new query();
}

activeTable::activeTable(idatabaseManager* mgr, const _TCHAR* tableName)
    : m_imple(new activeTableImple(mgr, tableName))
{
}

activeTable::activeTable(dbmanager_ptr& mgr, const _TCHAR* tableName)
    : m_imple(new activeTableImple(mgr.get(), tableName))
{
}

activeTable::activeTable(database_ptr& db, const _TCHAR* tableName)
    : m_imple(new activeTableImple(db, tableName))
{
}

activeTable::activeTable(database* db, const _TCHAR* tableName)
    : m_imple(new activeTableImple(db, tableName))
{
}

activeTable::~activeTable()
{
    delete m_imple;
}

table_ptr activeTable::table() const
{
    return m_imple->table();
}

activeTable& activeTable::alias(const _TCHAR* src, const _TCHAR* dst)
{
    m_imple->alias(src, dst);
    return *this;
}

activeTable& activeTable::resetAlias()
{
    m_imple->resetAlias();
    return *this;
}

writableRecord& activeTable::getWritableRecord()
{
    return m_imple->getWritableRecord();
}

activeTable& activeTable::join(recordset& rs, queryBase& q, const _TCHAR* name1,
                               const _TCHAR* name2, const _TCHAR* name3,
                               const _TCHAR* name4, const _TCHAR* name5,
                               const _TCHAR* name6, const _TCHAR* name7,
                               const _TCHAR* name8, const _TCHAR* name9,
                               const _TCHAR* name10, const _TCHAR* name11)
{
    m_imple->join(*rs.m_imple, q, name1, name2, name3, name4, name5, name6,
                  name7, name8);
    return *this;
}

activeTable& activeTable::outerJoin(recordset& rs, queryBase& q,
                                    const _TCHAR* name1, const _TCHAR* name2,
                                    const _TCHAR* name3, const _TCHAR* name4,
                                    const _TCHAR* name5, const _TCHAR* name6,
                                    const _TCHAR* name7, const _TCHAR* name8,
                                    const _TCHAR* name9, const _TCHAR* name10,
                                    const _TCHAR* name11)
{
    m_imple->outerJoin(*rs.m_imple, q, name1, name2, name3, name4, name5, name6,
                       name7, name8);
    return *this;
}

activeTable& activeTable::index(int v)
{
    m_imple->index(v);
    return *this;
}

activeTable& activeTable::option(int v)
{
    m_imple->option(v);
    return *this;
}

filter_ptr activeTable::prepare(queryBase& q, bool serverPrepare)
{
    return m_imple->prepare(q, serverPrepare);
}

activeTable& activeTable::read(recordset& rs, queryBase& q)
{
    m_imple->read(*rs.m_imple, q);
    return *this;
}

activeTable& activeTable::read(recordset& rs, queryBase& q, validationFunc func)
{
    m_imple->read(*rs.m_imple, q, func);
    return *this;
}

activeTable& activeTable::read(recordset& rs, filter_ptr& q)
{
    m_imple->read(*rs.m_imple, q);
    return *this;
}

activeTable& activeTable::read(recordset& rs, filter_ptr& q, validationFunc func)
{
    m_imple->read(*rs.m_imple, q, func);
    return *this;
}

activeTable& activeTable::read(recordset& rs, filter_ptr& q, 
                        const std::vector< std::_tstring>& values)
{
    q->supplyValues(values);
    m_imple->read(*rs.m_imple, q);
    return *this;
}

activeTable& activeTable::read(recordset& rs, filter_ptr& q, validationFunc func,
                        const std::vector< std::_tstring>& values)
{
    q->supplyValues(values);
    m_imple->read(*rs.m_imple, q);
    return *this;
}

template<class T0>
activeTable& activeTable::read(recordset& rs, filter_ptr& q, const T0 v0)
{
    q->supplyValue(0, v0);
    m_imple->read(*rs.m_imple, q);
    return *this;
}

template<class T0, class T1>
activeTable& activeTable::read(recordset& rs, filter_ptr& q, const T0 v0, const T1 v1)
{
    q->supplyValue(0, v0);
    q->supplyValue(1, v1);
    m_imple->read(*rs.m_imple, q);
    return *this;
}

template<class T0, class T1, class T2>
activeTable& activeTable::read(recordset& rs, filter_ptr& q, const T0 v0, const T1 v1,
                    const T2 v2)
{
    q->supplyValue(0, v0);
    q->supplyValue(1, v1);
    q->supplyValue(2, v2);
    m_imple->read(*rs.m_imple, q);
    return *this;
}

template<class T0, class T1, class T2, class T3>
activeTable& activeTable::read(recordset& rs, filter_ptr& q, const T0 v0, const T1 v1,
                    const T2 v2, const T3 v3)
{
    q->supplyValue(0, v0);
    q->supplyValue(1, v1);
    q->supplyValue(2, v2);
    q->supplyValue(3, v3);
    m_imple->read(*rs.m_imple, q);
    return *this;
}

template<class T0, class T1, class T2, class T3, class T4>
activeTable& activeTable::read(recordset& rs, filter_ptr& q, const T0 v0, const T1 v1,
                    const T2 v2, const T3 v3, const T4 v4)
{
    q->supplyValue(0, v0);
    q->supplyValue(1, v1);
    q->supplyValue(2, v2);
    q->supplyValue(3, v3);
    q->supplyValue(4, v4);
    m_imple->read(*rs.m_imple, q);
    return *this;
}

template<class T0, class T1, class T2, class T3, class T4, class T5>
activeTable& activeTable::read(recordset& rs, filter_ptr& q, const T0 v0, const T1 v1,
                    const T2 v2, const T3 v3, const T4 v4, const T5 v5)
{
    q->supplyValue(0, v0);
    q->supplyValue(1, v1);
    q->supplyValue(2, v2);
    q->supplyValue(3, v3);
    q->supplyValue(4, v4);
    q->supplyValue(5, v5);
    m_imple->read(*rs.m_imple, q);
    return *this;
}

template<class T0, class T1, class T2, class T3, class T4, class T5, class T6>
activeTable& activeTable::read(recordset& rs, filter_ptr& q, const T0 v0, const T1 v1,
                    const T2 v2, const T3 v3, const T4 v4, const T5 v5,
                    const T6 v6)
{
    q->supplyValue(0, v0);
    q->supplyValue(1, v1);
    q->supplyValue(2, v2);
    q->supplyValue(3, v3);
    q->supplyValue(4, v4);
    q->supplyValue(5, v5);
    q->supplyValue(6, v6);
    m_imple->read(*rs.m_imple, q);
    return *this;
}

template<class T0, class T1, class T2, class T3, class T4, class T5, class T6,
            class T7>
activeTable& activeTable::read(recordset& rs, filter_ptr& q, const T0 v0, const T1 v1,
                    const T2 v2, const T3 v3, const T4 v4, const T5 v5,
                    const T6 v6, const T7 v7)
{
    q->supplyValue(0, v0);
    q->supplyValue(1, v1);
    q->supplyValue(2, v2);
    q->supplyValue(3, v3);
    q->supplyValue(4, v4);
    q->supplyValue(5, v5);
    q->supplyValue(6, v6);
    q->supplyValue(7, v7);
    m_imple->read(*rs.m_imple, q);
    return *this;
}

template<class T0, class T1, class T2, class T3, class T4, class T5, class T6,
            class T7, class T8>
activeTable& activeTable::read(recordset& rs, filter_ptr& q, const T0 v0, const T1 v1,
                    const T2 v2, const T3 v3, const T4 v4, const T5 v5,
                    const T6 v6, const T7 v7, const T8 v8)
{
    q->supplyValue(0, v0);
    q->supplyValue(1, v1);
    q->supplyValue(2, v2);
    q->supplyValue(3, v3);
    q->supplyValue(4, v4);
    q->supplyValue(5, v5);
    q->supplyValue(6, v6);
    q->supplyValue(7, v7);
    q->supplyValue(8, v8);
    m_imple->read(*rs.m_imple, q);
    return *this;
}

template<class T0, class T1, class T2, class T3, class T4, class T5, class T6,
            class T7, class T8, class T9>
activeTable& activeTable::read(recordset& rs, filter_ptr& q, const T0 v0, const T1 v1,
                    const T2 v2, const T3 v3, const T4 v4, const T5 v5,
                    const T6 v6, const T7 v7, const T8 v8, const T9 v9)
{
    q->supplyValue(0, v0);
    q->supplyValue(1, v1);
    q->supplyValue(2, v2);
    q->supplyValue(3, v3);
    q->supplyValue(4, v4);
    q->supplyValue(5, v5);
    q->supplyValue(6, v6);
    q->supplyValue(7, v7);
    q->supplyValue(8, v8);
    q->supplyValue(9, v9);
    m_imple->read(*rs.m_imple, q);
    return *this;
}

template<class T0, class T1, class T2, class T3, class T4, class T5, class T6,
            class T7, class T8, class T9, class T10>
activeTable& activeTable::read(recordset& rs, filter_ptr& q, const T0 v0, const T1 v1,
                    const T2 v2, const T3 v3, const T4 v4, const T5 v5,
                    const T6 v6, const T7 v7, const T8 v8, const T9 v9,
                    const T10 v10)
{
    q->supplyValue(0, v0);
    q->supplyValue(1, v1);
    q->supplyValue(2, v2);
    q->supplyValue(3, v3);
    q->supplyValue(4, v4);
    q->supplyValue(5, v5);
    q->supplyValue(6, v6);
    q->supplyValue(7, v7);
    q->supplyValue(8, v8);
    q->supplyValue(9, v9);
    q->supplyValue(10, v10);
    m_imple->read(*rs.m_imple, q);
    return *this;
}

activeTable* activeTable::create(idatabaseManager* mgr, const _TCHAR* tableName)
{
    return new activeTable(mgr, tableName);
}

activeTable* activeTable::create(dbmanager_ptr& mgr, const _TCHAR* tableName)
{
    return new activeTable(mgr, tableName);
}

activeTable* activeTable::create(database_ptr& db, const _TCHAR* tableName)
{
    return new activeTable(db, tableName);
}

activeTable* activeTable::create(database* db, const _TCHAR* tableName)
{
    return new activeTable(db, tableName);
}

void activeTable::release()
{
    delete this;
}

void activeTable::releaseTable()
{
    m_imple->releaseTable();
}

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs
