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
	return true;
}

#define BENCH_TIMER_SECONDS 1.5f


#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
	try
	{
		if (argc < 6)
		{
			printf("\nusage: bench_scale [host] [function_number] [max_worker_num]"
				" [trnsactions] [avg_count]\n"
				"\n"
				"|----- [function_number] description      -----|\n"
				"| 0: Read a record by unique id.               |\n"
				"| 1: Insert a record by autoincrement.         |\n"
				"| 2: Read 50 user records and join group name. |\n"
				"| +10 Test MySQL (10 11 12)                    |\n"
				"|----------------------------------------------|\n"
				"\n"
				"|----- [trnsactions] description          -----|\n"
				"|  1 - n : N times roop a function             |\n"
				"| -1     : Execute Infinite loop and the number|\n"
				"|          of processings within regulation    |\n"
				"|          time is counted.                    |\n"
				"|----------------------------------------------|\n"
				"\n"
				"|----- [avg_count] description            -----|\n"
				"|  1 - n : Count N times and results return    |\n"
				"|          those avarage.                      |\n"
				"|----------------------------------------------|\n"

				"exsample : bench_scale localhost 0 100 1 5     \n"	);
			return 0;
		}
		bzs::test::worker::mysql::mysqlInit mysql;
		
		if (!mysql_thread_safe()) 
		{
			std::tcout << _T("MySQL client is not thread safe.") << std::endl;
			return 1;
		}

		connectParams param(_T("tdap"), argv[1], _T("querytest"), _T("test"));

		bzs::test::worker::mysql::connectParam paramMysql;
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
		int stressMode = (loopCount == -1) ? true:false;
		if (stressMode)	loopCount = 1;
		if (chacheTableFixer(param))
		{
			std::vector<transactionSec> results;
			results.reserve(maxWorkerNum);
			pooledDbManager::setMaxConnections(maxWorkerNum);

			double totalTransactionTime = 0;
			bzs::rtl::benchmarkMt bm;
			bm.start();
			for (int wn=workerNumStep;wn<=maxWorkerNum; wn+=workerNumStep)
			{
				double t = 0;
				for (int k=0;k<avg_count;++k)
				{
					g_bench_signal = stressMode ? BENCH_SIGNAL_GO: BENCH_SIGNAL_BLUE;
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
					Sleep(200 * MCRTOMM);
					sync.wait();// start all workers
					if (stressMode)
					{
						Sleep(200 * MCRTOMM);
						g_bench_signal = BENCH_SIGNAL_GREEN;
						Sleep(BENCH_TIMER_SECONDS * 1000 * MCRTOMM);
						g_bench_signal = BENCH_SIGNAL_BREAK;
					}
					threads.join_all();
					for (int i=0;i<wn;++i)
						t += workers[i]->total();
					
				}
				totalTransactionTime += t;
				transactionSec ts;
				if (stressMode)
				{
					if (t) ts.value = t/BENCH_TIMER_SECONDS;
				}
				else
				{
					if (t) ts.value = (__int64)((double)1000000.0f*wn*wn*avg_count*loopCount/t);
				}
				ts.workes = wn;
				results.push_back(ts);
			}

			printf("\n");
			showResult(results, bm.end(), totalTransactionTime);
			return 0;
		}
		return 1;
	}

	catch(bzs::rtl::exception& e)
	{
		std::_tstring s = *bzs::rtl::getMsg(e);
		std::tcout << _T("[ERROR] ") << s << std::endl;
	}
	return 1;
}






