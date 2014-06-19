#pragma once
#ifndef	BZS_TEST_BENCH_WORKERTRANSACTDIMPLE_H
#define	BZS_TEST_BENCH_WORKERTRANSACTDIMPLE_H
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

class worker : public workerBase
{
	const connectParams& m_parmas;
#ifdef USE_CONNECTION_POOL
	dbmanager_ptr m_db;
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
			printf("readOne error! stat = %d  id = %ld\n",tb->stat(),  m_id);
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
		q.select(_T("id"), _T("name"),_T("group")).where(_T("id"), _T("<"), v+50);
		m_atu->index(0).keyValue(v).read(m_rs, q);
		
		//q.reset();
		//m_ate->index(0).join(m_rs, q.select(_T("comment")).optimize(queryBase::joinKeyValuesUnique), _T("id"));
		
		//Join group::name
		q.reset();
		m_atg->index(0).join(m_rs, q.select(_T("group_name")), _T("group"));
		if (m_rs.size() != 50)
			printf("query read error! id = %d size = %d\n", m_id, m_rs.size() );
		
	}

public:
	worker(int id, int loopCount, int functionNumber, const connectParams& param
			,boost::barrier& sync) 
			: workerBase(id, loopCount, functionNumber, sync)
			,m_parmas(param){}
	

	void initExecute()
	{
		#ifdef USE_CONNECTION_POOL
			m_db = cpool.get(&m_parmas);

			if (m_db == NULL)
			{
				printf("Can't get database! %ld\n", m_id);
				return;
			}
			const _TCHAR* tbName = (m_functionNumber == 0) ? _T("user"): _T("cache");
			m_tb = m_db->table(tbName);
			if (m_tb == NULL)
			{
				printf("Can't get table! %ld\n", m_id);
				return;
			}

			
		#else
			try
			{
				m_db = createDatadaseObject();
				connectOpen(m_db, m_parmas, true); 
				const _TCHAR* tbName = (m_functionNumber == 0) ? _T("user"): _T("cache");
				m_tb = openTable(m_db, tbName);
				m_tb->setKeyNum(0);
			}
			catch(bzs::rtl::exception& e)
			{
				std::_tstring msg;
				msg = *bzs::rtl::getMsg(e);
				boost::mutex::scoped_lock lck(m_mutex);
				std::tcout << _T("[ERROR] ") << msg.c_str() << std::endl;
			}
		#endif//USE_CONNECTION_POOL 
		if (m_functionNumber == 2)
		{
			m_atu.reset(new activeTable(m_db, _T("user")));
			m_atg.reset(new activeTable(m_db, _T("groups")));
			m_ate.reset(new activeTable(m_db, _T("extention")));


			#ifdef LINUX
				const char* fd_name = "–¼‘O";
			#else
				#ifdef _UNICODE
					const wchar_t* fd_name = L"–¼‘O";
				#else
					char fd_name[30];
					WideCharToMultiByte(CP_UTF8, 0, L"–¼‘O", -1, fd_name, 30, NULL, NULL);
				#endif
			#endif
			m_atu->alias(fd_name, _T("name"));
			m_atg->alias(_T("name"), _T("group_name"));
		}
		
		
	}

	void endExecute()
	{
		m_tb.reset();
		m_db.reset();
		#ifdef USE_CONNECTION_POOL
			releaseConnection(&cpool);
		#endif
	}

	void doExecute()
	{
		try
		{
			if (m_functionNumber == 0)
			{
				for (int i=0;i<m_loopCount;++i)
					readOne(m_tb);
			}
			else if (m_functionNumber == 1)
			{
				for (int i=0;i<m_loopCount;++i)
					insertOne(m_tb);
			}
			else
			{
				for (int i=0;i<m_loopCount;++i)
					queryOne();
			}
		}

		catch(bzs::rtl::exception& e)
		{
			std::_tstring msg;
			msg = *bzs::rtl::getMsg(e);
			boost::mutex::scoped_lock lck(m_mutex);
			std::tcout << _T("[ERROR] ") << msg.c_str() << std::endl;
		}

		catch(...)
		{
			boost::mutex::scoped_lock lck(m_mutex);
			std::cout << "worker error id = " << m_id << std::endl;
		}
	}

};


} //namespace transactd
} //namespace worker
} //namespace test
} //namespace bzs

#endif	//BZS_TEST_BENCH_WORKERTRANSACTDIMPLE_H

