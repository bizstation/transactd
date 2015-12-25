#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_ACTIVETABLE_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_ACTIVETABLE_H
/* =================================================================
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
 ================================================================= */
#include "recordset.h"
#include "memRecord.h"

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

/* For php use */
class preparedQuery
{
    pq_handle m_filter;
    int m_index;
public:
    preparedQuery(pq_handle filter) : m_filter(filter),m_index(0){}
    
    inline bool supplyValue(int index, const _TCHAR* v)
    {
        return client::supplyValue(m_filter, index, v);
    }
    
    inline bool supplyValue(int index, __int64 v)
    {
        return client::supplyValue(m_filter, index, v);
    }

    inline bool supplyValue(int index, double v)
    {
        return client::supplyValue(m_filter, index, v);
    }

    inline bool addValue(const _TCHAR* v)
    {
        return client::supplyValue(m_filter, m_index++, v);
    }
    
    inline bool addValue(__int64 v)
    {
        return client::supplyValue(m_filter, m_index++, v);
    }

    inline bool addValue(double v)
    {
        return client::supplyValue(m_filter, m_index++, v);
    }

    inline void resetAddIndex() { m_index = 0; }
/** @cond INTERNAL */
    inline pq_handle& getFilter() { return m_filter; };
/** @endcond */
};


class DLLLIB activeTable
{
    class activeTableImple* m_imple;

    activeTable(const activeTable& r);
    activeTable& operator=(const activeTable& r);

    template<class T> 
    inline void _supplyValue(pq_handle& q, int index, const T v)
    {
        if (!supplyValue(q, index, v))
            THROW_BZS_ERROR_WITH_MSG(_T("Prepared query : supply value error."));
    }

public:
    explicit activeTable(idatabaseManager* mgr, const _TCHAR* tableName);
    explicit activeTable(dbmanager_ptr& mgr, const _TCHAR* tableName);
    explicit activeTable(database_ptr& db, const _TCHAR* tableName,
                                            short mode = TD_OPEN_NORMAL);
    explicit activeTable(database* db, const _TCHAR* tableName,
                                            short mode = TD_OPEN_NORMAL);
    explicit activeTable(database_ptr& db, short tableIndex,
                                            short mode = TD_OPEN_NORMAL);
    explicit activeTable(database* db, short tableIndex,
                                            short mode = TD_OPEN_NORMAL);

    ~activeTable();

    activeTable& alias(const _TCHAR* src, const _TCHAR* dst);

    activeTable& resetAlias();

    writableRecord& getWritableRecord();

    recordset& join(recordset& rs, queryBase& q, const _TCHAR* name1,
                      const _TCHAR* name2 = NULL, const _TCHAR* name3 = NULL,
                      const _TCHAR* name4 = NULL, const _TCHAR* name5 = NULL,
                      const _TCHAR* name6 = NULL, const _TCHAR* name7 = NULL,
                      const _TCHAR* name8 = NULL);

    recordset&
    outerJoin(recordset& rs, queryBase& q, const _TCHAR* name1,
              const _TCHAR* name2 = NULL, const _TCHAR* name3 = NULL,
              const _TCHAR* name4 = NULL, const _TCHAR* name5 = NULL,
              const _TCHAR* name6 = NULL, const _TCHAR* name7 = NULL,
              const _TCHAR* name8 = NULL);

    recordset& join(recordset& rs, pq_handle& q, const _TCHAR* name1,
                      const _TCHAR* name2 = NULL, const _TCHAR* name3 = NULL,
                      const _TCHAR* name4 = NULL, const _TCHAR* name5 = NULL,
                      const _TCHAR* name6 = NULL, const _TCHAR* name7 = NULL,
                      const _TCHAR* name8 = NULL);

    recordset&
    outerJoin(recordset& rs, pq_handle& q, const _TCHAR* name1,
              const _TCHAR* name2 = NULL, const _TCHAR* name3 = NULL,
              const _TCHAR* name4 = NULL, const _TCHAR* name5 = NULL,
              const _TCHAR* name6 = NULL, const _TCHAR* name7 = NULL,
              const _TCHAR* name8 = NULL);

    activeTable& index(int v);
    table_ptr table() const;
    activeTable& option(int v);
    pq_handle prepare(queryBase& q, bool serverPrepare = false);
    recordset& read(recordset& rs, queryBase& q);
    recordset& read(recordset& rs, queryBase& q, validationFunc func);
    recordset& read(recordset& rs, pq_handle& q);
    recordset& read(recordset& rs, pq_handle& q, validationFunc func);
    recordset& readMore(recordset& rs);
    /** @cond INTERNAL */
    template<class T0>
    recordset& read(recordset& rs, pq_handle& q, const T0 v0)
    {
        _supplyValue(q, 0, v0);
        read(rs, q);
        return rs;
    }

    template<class T0, class T1>
    recordset& read(recordset& rs, pq_handle& q, const T0 v0, const T1 v1)
    {
        _supplyValue(q, 0, v0);
        _supplyValue(q, 1, v1);
        read(rs, q);
        return rs;
    }

    template<class T0, class T1, class T2>
    recordset& read(recordset& rs, pq_handle& q, const T0 v0, const T1 v1,
                        const T2 v2)
    {
        _supplyValue(q, 0, v0);
        _supplyValue(q, 1, v1);
        _supplyValue(q, 2, v2);
        read(rs, q);
        return rs;
    }

    template<class T0, class T1, class T2, class T3>
    recordset& read(recordset& rs, pq_handle& q, const T0 v0, const T1 v1,
                        const T2 v2, const T3 v3)
    {
        _supplyValue(q, 0, v0);
        _supplyValue(q, 1, v1);
        _supplyValue(q, 2, v2);
        _supplyValue(q, 3, v3);
        read(rs, q);
        return rs;
    }

    template<class T0, class T1, class T2, class T3, class T4>
    recordset& read(recordset& rs, pq_handle& q, const T0 v0, const T1 v1,
                        const T2 v2, const T3 v3, const T4 v4)
    {
        _supplyValue(q, 0, v0);
        _supplyValue(q, 1, v1);
        _supplyValue(q, 2, v2);
        _supplyValue(q, 3, v3);
        _supplyValue(q, 4, v4);
        read(rs, q);
        return rs;
    }

    template<class T0, class T1, class T2, class T3, class T4, class T5>
    recordset& read(recordset& rs, pq_handle& q, const T0 v0, const T1 v1,
                        const T2 v2, const T3 v3, const T4 v4, const T5 v5)
    {
        _supplyValue(q, 0, v0);
        _supplyValue(q, 1, v1);
        _supplyValue(q, 2, v2);
        _supplyValue(q, 3, v3);
        _supplyValue(q, 4, v4);
        _supplyValue(q, 5, v5);
        read(rs, q);
        return rs;
    }

    template<class T0, class T1, class T2, class T3, class T4, class T5, class T6>
    recordset& read(recordset& rs, pq_handle& q, const T0 v0, const T1 v1,
                        const T2 v2, const T3 v3, const T4 v4, const T5 v5,
                        const T6 v6)
    {
        _supplyValue(q, 0, v0);
        _supplyValue(q, 1, v1);
        _supplyValue(q, 2, v2);
        _supplyValue(q, 3, v3);
        _supplyValue(q, 4, v4);
        _supplyValue(q, 5, v5);
        _supplyValue(q, 6, v6);
        read(rs, q);
        return rs;
    }

    template<class T0, class T1, class T2, class T3, class T4, class T5, class T6,
                class T7>
    recordset& read(recordset& rs, pq_handle& q, const T0 v0, const T1 v1,
                        const T2 v2, const T3 v3, const T4 v4, const T5 v5,
                        const T6 v6, const T7 v7)
    {
        _supplyValue(q, 0, v0);
        _supplyValue(q, 1, v1);
        _supplyValue(q, 2, v2);
        _supplyValue(q, 3, v3);
        _supplyValue(q, 4, v4);
        _supplyValue(q, 5, v5);
        _supplyValue(q, 6, v6);
        _supplyValue(q, 7, v7);
        read(rs, q);
        return rs;
    }
    /** @endcond */

    /** @cond INTERNAL */

    template <class T0> activeTable& keyValue(const T0 kv0)
    {
        keyValueSetter<T0>::set(table(), table()->keyNum(), kv0);
        return *this;
    }

    template <class T0, class T1>
    activeTable& keyValue(const T0 kv0, const T1 kv1)
    {
        keyValueSetter<T0, T1>::set(table(), table()->keyNum(), kv0, kv1);
        return *this;
    }

    template <class T0, class T1, class T2>
    activeTable& keyValue(const T0 kv0, const T1 kv1, const T2 kv2)
    {
        keyValueSetter<T0, T1, T2>::set(table(), table()->keyNum(), kv0, kv1,
                                        kv2);
        return *this;
    }

    template <class T0, class T1, class T2, class T3>
    activeTable& keyValue(const T0 kv0, const T1 kv1, const T2 kv2,
                          const T3 kv3)
    {
        keyValueSetter<T0, T1, T2, T3>::set(table(), table()->keyNum(), kv0,
                                            kv1, kv2, kv3);
        return *this;
    }

    template <class T0, class T1, class T2, class T3, class T4>
    activeTable& keyValue(const T0 kv0, const T1 kv1, const T2 kv2,
                          const T3 kv3, const T4 kv4)
    {
        keyValueSetter<T0, T1, T2, T3, T4>::set(table(), table()->keyNum(), kv0,
                                                kv1, kv2, kv3, kv4);
        return *this;
    }

    template <class T0, class T1, class T2, class T3, class T4, class T5>
    activeTable& keyValue(const T0 kv0, const T1 kv1, const T2 kv2,
                          const T3 kv3, const T4 kv4, const T5 kv5)
    {
        keyValueSetter<T0, T1, T2, T3, T4, T5>::set(
            table(), table()->keyNum(), kv0, kv1, kv2, kv3, kv4, kv5);
        return *this;
    }

    template <class T0, class T1, class T2, class T3, class T4, class T5,
              class T6>
    activeTable& keyValue(const T0 kv0, const T1 kv1, const T2 kv2,
                          const T3 kv3, const T4 kv4, const T5 kv5,
                          const T6 kv6)
    {
        keyValueSetter<T0, T1, T2, T3, T4, T5, T6>::set(
            table(), table()->keyNum(), kv0, kv1, kv2, kv3, kv4, kv5, kv6);
        return *this;
    }

    /** @endcond */

    template <class T0, class T1, class T2, class T3, class T4, class T5,
              class T6, class T7>
    activeTable& keyValue(const T0 kv0, const T1 kv1, const T2 kv2,
                          const T3 kv3, const T4 kv4, const T5 kv5,
                          const T6 kv6, const T7 kv7)
    {
        keyValueSetter<T0, T1, T2, T3, T4, T5, T6, T7>::set(
            table(), table()->keyNum(), kv0, kv1, kv2, kv3, kv4, kv5, kv6, kv7);
        return *this;
    }

    static activeTable* create(idatabaseManager* mgr, const _TCHAR* tableName);
    static activeTable* create(dbmanager_ptr& mgr, const _TCHAR* tableName);
    static activeTable* create(database_ptr& db, const _TCHAR* tableName,
                                                short mode = TD_OPEN_NORMAL);
    static activeTable* create(database* db, const _TCHAR* tableName,
                                                short mode = TD_OPEN_NORMAL);
    static activeTable* create(database_ptr& db, short tableIndex,
                                                short mode = TD_OPEN_NORMAL);
    static activeTable* create(database* db, short tableIndex,
                                                short mode = TD_OPEN_NORMAL);

    void release();
    void releaseTable();
};

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#endif // BZS_DB_PROTOCOL_TDAP_CLIENT_ACTIVETABLE_H
