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

#include <bzs/db/protocol/tdap/client/connectionPool.h>
#include <bzs/rtl/benchmark.h>
#include <bzs/example/queryData.h>

#include <iostream>
#include <boost/thread/barrier.hpp>

using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;
using namespace boost;


class benchResult
{

public:
	benchResult():totalTime(0),transactionCount(0){}
	int totalTime;
	int transactionCount;
	benchResult& operator+=(const benchResult& r)
	{
		if (this != &r)
		{
			totalTime += r.totalTime;
			transactionCount += r.transactionCount;
		}
		return *this;
	}

};


class worker
{
   int m_id;
   int m_loopCount;
   int m_functionNumber;
   benchResult m_result;
   const connectParams& m_parmas;
   boost::barrier& m_sync;
   static mutex m_mutex;

	void readOne(table_ptr& tb)
	{
		tb->setKeyNum(0);
		//Sleep(1);
		tb->setFV((short)0, m_id % 20000 + 1);
		tb->seek();
		if (tb->stat() != 0)
			printf("readOne error! stat = %d  id = %ld\n",tb->stat(),  m_id);


	}

	void insertOne(table_ptr& tb)
	{
		tb->setKeyNum(0);
		tb->clearBuffer();
		tb->setFV(1, m_id);
		insertRecord(tb);

	}


public:
	worker(int id, int loopCount, int functionNumber, const connectParams& param
			,boost::barrier& sync)
			:m_id(id),m_loopCount(loopCount),m_functionNumber(functionNumber)
				,m_parmas(param),m_sync(sync){};

	const benchResult& result() const {return m_result;}

	void execute()
	{

		//m_sync.wait();
		try
		{
			begin_use_pool_database();
			{
				dbmanager_ptr db = get_pool_database(&m_parmas);
				if (db == NULL)
				{
					printf("Can't get database! %ld\n", m_id);
					return;
				}
				const _TCHAR* tbName = (m_functionNumber == 0) ? _T("user"): _T("cache");
				table_ptr tb = db->table(tbName);
				if (tb == NULL)
				{
					printf("Can't get table! %ld\n", m_id);
					return;
				}
                m_sync.wait();
				if (m_functionNumber == 0)
				{
					for (int i=0;i<m_loopCount;++i)
						readOne(tb);
				}else
				{
					for (int i=0;i<m_loopCount;++i)
						insertOne(tb);

				}
				end_use_pool_database();
			}
		}

		catch(bzs::rtl::exception& e)
		{
			std::_tstring msg;
			msg = *bzs::rtl::getMsg(e);
			std::tcout << _T("[ERROR] ") << msg.c_str() << std::endl;
		}

		catch(...)
		{
			mutex::scoped_lock lck(m_mutex);
			std::cout << "worker error id = " << m_id << std::endl;
			delete this;
		}
	}

};
mutex worker::m_mutex;
typedef std::vector<boost::shared_ptr<worker> > workers;

int totalResult(const workers& wks)
{
	benchResult tmp;
	for (int i=0;i<(int)wks.size();++i)
		tmp += wks[i]->result();
	if (tmp.totalTime)
		return 1000 * tmp.transactionCount /  tmp.totalTime;
	return -1;
}

struct transactionSec
{
	int workes;
	__int64 value;

};

void showResult(const std::vector<transactionSec>& results)
{
	std::tcout << _T("----------------------------------") << std::endl;
	std::tcout << _T("Clients \tTransactions/sec") << std::endl;
	std::tcout << _T("----------------------------------") << std::endl;

	for (int i=0;i<(int)results.size();++i)
	   std::tcout << results[i].workes << _T("\t") <<results[i].value << std::endl;

	std::tcout << _T("----------------------------------") << std::endl;

}

bool chacheTableFixer( const connectParams& param)
{
	
	database_ptr db = createDatadaseObject();
	openDatabase(db, param);
	
	dbdef* def = db->dbDef();
	short index = def->tableNumByName(_T("cache"));
	if (index == -1 && !createCacheTable(def))
		return false;
	table_ptr tb = openTable(db, _T("cache"));
	if (tb == NULL)
	{
		printf("Can't get cache table! \n");
		return false;
	}
	tb->setKeyNum(0);
	tb->seekFirst();
	while (tb->stat()==0)
	{
		tb->del();
		tb->seekNext();
	}

	return true;
}

#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
	try
	{
		if (argc < 4)
		{
			printf("usage: bench_scale database_uri function_number max_worker_num worker_num_step \n "
				"exsample : bench_scale \"tdap://localhost/test?dbfile=test.bdf\" 100 1000\n");
			return 0;
		}
		connectParams param(argv[1]);
		param.setMode(0);
		int funcNumber = _ttol(argv[2]);
		int maxWorkerNum = _ttol(argv[3]);
		int workerNumStep = maxWorkerNum / 20;
		if (workerNumStep == 0) workerNumStep = 1;
		int loopCount = _ttol(argv[4]);
		if (chacheTableFixer(param))
		{
			std::vector<transactionSec> results;
			results.reserve(maxWorkerNum);
			cpool.setMaxConnections(maxWorkerNum);
			for (int j=workerNumStep;j<=maxWorkerNum; j+=workerNumStep)
			{
				workers workers;

				thread_group threads;
				boost::barrier sync(j + 1);
				for (int i=0;i<j;++i)
				{
					int id = i + (j * 10000);
					boost::shared_ptr<worker> w(
						new worker(id, loopCount, funcNumber, param, sync));
					workers.push_back(w);
					threads.create_thread( bind(&worker::execute, w.get()));
					//printf("%ld create finish\n", id);
				}

				bzs::rtl::benchmarkMt bm;
				//printf("wait complate! \n");
				printf(".");
				sync.wait();
				bm.start();


				threads.join_all();


				transactionSec ts;
				int t = bm.end();
				if (t)
					ts.value = (__int64)1000000 * j *  loopCount / t;//totalResult(workers);
				else
					ts.value = -1;
				ts.workes = j;
				results.push_back(ts);

			}
			showResult(results);
			return 0;
		}
		return 1;
	}

	catch(bzs::rtl::exception& e)
	{
		std::tcout << _T("[ERROR] ") << *bzs::rtl::getMsg(e) << std::endl;
	}
	return 1;
}






