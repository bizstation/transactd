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
#include "workerTransactdImple.h"
#include "workerMysqlImple.h"
#include <iostream>


using namespace bzs::test::worker;
using namespace boost;


typedef std::vector<boost::shared_ptr<workerBase> > workers;

struct transactionSec
{
	transactionSec():value(0){}
	int workes;
	__int64 value;

};

void showResult(const std::vector<transactionSec>& results, int total, int totalTransactionTime)
{
	std::tcout << _T("----------------------------------") << std::endl;
	std::tcout << _T("Clients \tTransactions/sec") << std::endl;
	std::tcout << _T("----------------------------------") << std::endl;

	for (int i=0;i<(int)results.size();++i)
	   std::tcout << results[i].workes << _T("\t") <<results[i].value << std::endl;

	std::tcout << _T("----------------------------------") << std::endl;
	std::tcout << _T("total transaction time (sec)") << totalTransactionTime/1000000.0f << std::endl;
	std::tcout << _T("total testing time (sec)") << total/1000000.0f << std::endl;
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
	dropTable(db, _T("cache"));
	openTable(db, _T("cache"));
	//if (!createCacheTable(def))
	//	return false;
	
	return true;
}


#define BENCH_TIMER_SECONDS 1.5f 

#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
	try
	{
		mysql_library_init(0, NULL, NULL);

		if (!mysql_thread_safe()) 
		{
			std::tcout << _T("MySQL client is not thread safe.") << std::endl;
			return 1;
		}

		if (argc < 4)
		{
			printf("usage: bench_scale database_uri function_number max_worker_num worker_num_step \n "
				"exsample : bench_scale \"tdap://localhost/test?dbfile=test.bdf\" 100 1000\n");
			return 0;
		}
		connectParams param(_T("tdap"), argv[1], _T("querytest"), _T("test"));

		bzs::test::worker::mysql::connectParamMysql paramMysql;
		#ifdef _UNICODE
			char hostname[256];
			WideCharToMultiByte(CP_ACP, 0, argv[1], -1, hostname, 256, NULL, NULL);
		#else
			const char* hostname = argv[1];
		#endif
		paramMysql.hostname = hostname;
		paramMysql.username = "root";
		paramMysql.passwd = "";
		paramMysql.database = "querytest";
		paramMysql.port = 0;

		param.setMode(0);
		int funcNumber = _ttol(argv[2]);
		int maxWorkerNum = _ttol(argv[3]);
		int workerNumStep = maxWorkerNum / 20;
		if (workerNumStep == 0) workerNumStep = 1;
		int loopCount = _ttol(argv[4]);
		int avg_count = _ttol(argv[5]);
		if (chacheTableFixer(param))
		{
			std::vector<transactionSec> results;
			results.reserve(maxWorkerNum);
			cpool.setMaxConnections(maxWorkerNum);
			
			double totalTransactionTime = 0;
			bzs::rtl::benchmarkMt bm;
			bm.start();
			for (int wn=workerNumStep;wn<=maxWorkerNum; wn+=workerNumStep)
			{
				double t = 0;
				for (int k=0;k<avg_count;++k)
				{
					Sleep(30);
					workers workers;
					thread_group threads;
					boost::barrier sync(wn + 1);
					for (int i=0;i<wn;++i)
					{
						int id = i + (wn * 10000);
					
						boost::shared_ptr<workerBase> w;
						if (funcNumber < 10)
							w.reset(new transactd::worker(id, loopCount, funcNumber, param, sync));
						else
							w.reset(new bzs::test::worker::mysql::worker(id, loopCount, funcNumber, paramMysql, sync));	
						workers.push_back(w);
						threads.create_thread( bind(&workerBase::execute, w.get()));
					}
					printf("*");
					sync.wait();// start all workers
					threads.join_all();
					for (int i=0;i<wn;++i)
						t += workers[i]->totalTime();
					
				}
				totalTransactionTime += t;
				transactionSec ts;
				if (t) ts.value = (__int64)((double)1000000.0f*wn*wn*avg_count*loopCount/t);

				ts.workes = wn;
				results.push_back(ts);
				
			}

			printf("\n");
			showResult(results, bm.end(), totalTransactionTime);
			mysql_library_end();
			return 0;
		}
		mysql_library_end();
		return 1;
	}

	catch(bzs::rtl::exception& e)
	{
		std::_tstring s = *bzs::rtl::getMsg(e);
		std::tcout << _T("[ERROR] ") << s << std::endl;
	}
	mysql_library_end();
	return 1;
}






