#ifndef BZS_TEST_BENCH_WORKERTRANSACTDIMPLE_H
#define BZS_TEST_BENCH_WORKERTRANSACTDIMPLE_H
/* =================================================================
 Copyright (C) 20014 BizStation Corp All rights reserved.

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
#include <bzs/test/transactdBench/workerBase.h>
#include <bzs/db/protocol/tdap/client/connectionPool.h>
#include <bzs/db/protocol/tdap/client/pooledDatabaseManager.h>
#include <bzs/example/queryData.h>
#include <bzs/db/protocol/tdap/client/activeTable.h>
#include <tchar.h>
using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;

namespace bzs
{
namespace test
{
namespace worker
{
namespace transactd
{

#define USE_CONNECTION_POOL
#define TRD_READ_ONE 0
#define TRD_INSERT_ONE 1
#define TRD_QUERY 2
#define TRD_RECORDSET_COUNT 50

class worker : public workerBase
{
    const connectParams& m_parmas;
#ifdef USE_CONNECTION_POOL
    pooledDbManager m_db_real;
    pooledDbManager* m_db;
#else
    database_ptr m_db;

#endif
    table_ptr m_tb;
    boost::shared_ptr<activeTable> m_atu;
    boost::shared_ptr<activeTable> m_atg;
    boost::shared_ptr<activeTable> m_ate;
    recordset m_rs;
    void readOne(table_ptr& tb)
    {
        int v = (rand() % 15000) + 1;
        tb->setFV((short)0, v);
        tb->seek();
        if (tb->stat() != 0)
            printf("readOne error! stat = %d  id = %ld\n", tb->stat(), m_id);
    }

    void insertOne(table_ptr& tb)
    {
        tb->clearBuffer();
        tb->setFV(1, m_id);
        insertRecord(tb);
    }

    void queryOne()
    {
        m_rs.clear();
        query q;
        int v = (rand() % 15000) + 1;
        q.select(_T("id"), _T("name"), _T("group"))
            .where(_T("id"), _T("<"), v + TRD_RECORDSET_COUNT);
        m_atu->index(0).keyValue(v).read(m_rs, q);
        q.reset();

        // m_ate->index(0).join(m_rs,
        // q.select(_T("comment")).optimize(queryBase::joinHasOneOrHasMany),
        // _T("id"));

        // Join group::name
        /*recordset rs2;

        q.select(_T("group_name"));
        for (int i=0;i<m_rs.size();++i)
                q.addSeekKeyValuePtr(m_rs[i][(short)0].ptr());
        m_atg->table()->setQuery(&q);
        m_atg->index(0).keyValue(0).read(rs2, q);
        */
        m_atg->index(0).join(m_rs, q.select(_T("group_name")), _T("group"));
        if (m_rs.size() != TRD_RECORDSET_COUNT)
            printf("query read error! id = %d size = %d\n", m_id, m_rs.size());
    }

public:
    worker(int id, int loopCount, int functionNumber,
           const connectParams& param, boost::barrier& sync)
        : workerBase(id, loopCount, functionNumber, sync), m_parmas(param)
    {
        m_db = &m_db_real;
    }

    void initExecute()
    {
#ifdef USE_CONNECTION_POOL
        m_db->use(&m_parmas);
        const _TCHAR* tbName =
            (m_functionNumber == TRD_READ_ONE) ? _T("user") : _T("cache");
        m_tb = m_db->table(tbName);
#else
        try
        {
            m_db = createDatabaseObject();
            connectOpen(m_db, m_parmas, true);
            const _TCHAR* tbName =
                (m_functionNumber == TRD_READ_ONE) ? _T("user") : _T("cache");
            m_tb = openTable(m_db, tbName);
            m_tb->setKeyNum(0);
        }
        catch (bzs::rtl::exception& e)
        {
            std::_tstring msg;
            msg = *bzs::rtl::getMsg(e);
            boost::mutex::scoped_lock lck(m_mutex);
            std::tcout << _T("[ERROR] ") << msg.c_str() << std::endl;
        }
#endif // USE_CONNECTION_POOL
        if (m_functionNumber == TRD_QUERY)
        {
            m_atu.reset(new activeTable(m_db, _T("user")));
            m_atg.reset(new activeTable(m_db, _T("groups")));
            m_ate.reset(new activeTable(m_db, _T("extention")));

            _TCHAR tmp[30];
            m_atu->alias(name_field_str(tmp), _T("name"));
            m_atg->alias(_T("name"), _T("group_name"));
        }
    }

    void endExecute()
    {
        m_tb.reset();
#ifdef USE_CONNECTION_POOL
        m_db->unUse();
#else
        m_db.reset();
#endif
    }

    void doExecute()
    {
        try
        {
            if (m_functionNumber == TRD_READ_ONE)
            {
                for (int i = 0; i < m_loopCount; ++i)
                    readOne(m_tb);
            }
            else if (m_functionNumber == TRD_INSERT_ONE)
            {
                for (int i = 0; i < m_loopCount; ++i)
                    insertOne(m_tb);
            }
            else
            {
                for (int i = 0; i < m_loopCount; ++i)
                    queryOne();
            }
        }

        catch (bzs::rtl::exception& e)
        {
            std::_tstring msg;
            msg = *bzs::rtl::getMsg(e);
            boost::mutex::scoped_lock lck(m_mutex);
            std::tcout << _T("[ERROR] ") << msg.c_str() << std::endl;
        }

        catch (...)
        {
            boost::mutex::scoped_lock lck(m_mutex);
            std::cout << "worker error id = " << m_id << std::endl;
        }
    }
};

} // namespace transactd
} // namespace worker
} // namespace test
} // namespace bzs

#endif // BZS_TEST_BENCH_WORKERTRANSACTDIMPLE_H
