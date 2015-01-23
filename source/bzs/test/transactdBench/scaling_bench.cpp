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
#include "workerMySQLImple.h"

using namespace bzs::test::worker;
using namespace boost;

typedef std::vector<boost::shared_ptr<workerBase> > workers;

struct transactionSec
{
    transactionSec() : value(0),timeOne(0) {}
    int workes;
    __int64 value;
    double timeOne;
};

struct commandLineParam
{
    int funcNumber;
    int maxWorkerNum;
    int workerNumStep;
    int loopCount;
    int avg_count;
    int stressMode;
    commandLineParam(_TCHAR* argv[])
    {
        funcNumber = _ttol(argv[2]);
        maxWorkerNum = _ttol(argv[3]);
        workerNumStep = maxWorkerNum / 20;
        if (workerNumStep == 0)
            workerNumStep = 1;
        loopCount = _ttol(argv[4]);
        avg_count = _ttol(argv[5]);
        stressMode = (loopCount == -1) ? true : false;
        if (stressMode)
            loopCount = 1;
    }
};

void showResult(const std::vector<transactionSec>& results, int total,
                int totalTransactionTime, int totalTransactions,
                bool stressMode)
{

    std::tcout << _T("----------------------------------") << std::endl;
    std::tcout << _T("Clients \tTransactions/sec") << std::endl;
    std::tcout << _T("----------------------------------") << std::endl;

    for (int i = 0; i < (int)results.size(); ++i)
    {
        if (!stressMode)
            std::tcout << results[i].workes << _T("\t") << results[i].value
                   << _T("\t") << results[i].timeOne
                   << std::endl;
        else
            std::tcout << results[i].workes << _T("\t") << results[i].value
                   << std::endl;
    }
    if (!stressMode)
    {
        std::tcout << _T("----------------------------------") << std::endl;
        std::tcout << _T("total transaction time   (sec) ")
                   << totalTransactionTime / 1000000.0f << std::endl;
        std::tcout << _T("avarage transaction time (sec) ")
                   << totalTransactionTime / 1000000.0f / totalTransactions
                   << std::endl;
        std::tcout << _T("total testing time       (sec) ")
                   << total / 1000000.0f << std::endl;
    }
    std::tcout << _T("----------------------------------") << std::endl;
}

bool tableFixer(const connectParams& param)
{

    database_ptr db = createDatabaseObject();
    openDatabase(db, param);

    dbdef* def = db->dbDef();
    short index = def->tableNumByName(_T("cache"));
    if (index == -1 && !createCacheTable(def))
        return false;
    try{
    dropTable(db, _T("cache"));
    }
    catch(...){}
    openTable(db, _T("cache"));
    return true;
}

void makeMysqlParam(bzs::test::worker::mysql::connectParam& param,
                    const _TCHAR* host)
{
#ifdef _UNICODE
    char hostname[256];
    WideCharToMultiByte(CP_ACP, 0, host, -1, hostname, 256, NULL, NULL);
#else
    const char* hostname = host;
#endif
    param.hostname = hostname;
    param.username = "root";
    param.passwd = "";
    param.database = "querytest";
    param.port = 0;
}

#define BENCH_TIMER_SECONDS 1.5f
double executeWorkers(const commandLineParam& cmd, const connectParams param,
                      const bzs::test::worker::mysql::connectParam& paramMysql,
                      int wn)
{
    double t = 0;
    for (int k = 0; k < cmd.avg_count; ++k)
    {
        g_bench_signal = cmd.stressMode ? BENCH_SIGNAL_GO : BENCH_SIGNAL_BLUE;
        workers workers;
        thread_group threads;
        boost::barrier sync(wn + 1);
        for (int i = 0; i < wn; ++i)
        {
            int id = i + (wn * 10000);

            boost::shared_ptr<workerBase> w;
            if (cmd.funcNumber < 10)
                w.reset(new transactd::worker(id, cmd.loopCount, cmd.funcNumber,
                                              param, sync));
            else
                w.reset(new bzs::test::worker::mysql::worker(
                    id, cmd.loopCount, cmd.funcNumber, paramMysql, sync));
            workers.push_back(w);
            threads.create_thread(bind(&workerBase::execute, w.get()));
        }
        printf("*");
        fflush(stdout);
        Sleep(500);
        sync.wait(); // start all workers
        if (cmd.stressMode)
        {
            Sleep(200);
            g_bench_signal = BENCH_SIGNAL_GREEN;
            //Measurement interval
            Sleep((unsigned int)(BENCH_TIMER_SECONDS * 1000));
            g_bench_signal = BENCH_SIGNAL_BREAK;
        }
        threads.join_all();
        for (int i = 0; i < wn; ++i)
        {
            t += workers[i]->total();
            //std::tcout << _T("Execute Workers = ") << wn << " " << workers[i]->total() << std::endl;
        }
    }
    return t;
}

int execute(const commandLineParam& cmd, const connectParams param,
            const bzs::test::worker::mysql::connectParam& paramMysql,
            std::vector<transactionSec>& results, double& totalTransactionTime)
{
    int totalTransactions = 0;
    for (int wn = cmd.workerNumStep; wn <= cmd.maxWorkerNum;
         wn += cmd.workerNumStep)
    {
        double t = executeWorkers(cmd, param, paramMysql, wn);
        transactionSec ts;
        if (cmd.stressMode)
        {
            if (t)
                ts.value = (__int64)(t / BENCH_TIMER_SECONDS);
        }
        else
        {
            int n = wn * cmd.loopCount * cmd.avg_count;
            totalTransactionTime += t;
            totalTransactions += n;
            if (t)
            {
                ts.timeOne =  t / n / 1000000.0f;
                ts.value = (__int64)((double)1000000.0f * wn * wn *
                                    cmd.avg_count * cmd.loopCount / t);
            } 
        }
        ts.workes = wn;
        results.push_back(ts);
    }
    return totalTransactions;
}

void showUsage()
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

           "exsample : bench_scale localhost 0 100 1 5     \n");
}

#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
    try
    {
        if (argc < 6)
        {
            showUsage();
            return 0;
        }
        bzs::test::worker::mysql::mysqlInit mysql;

        if (!mysql_thread_safe())
        {
            std::tcout << _T("MySQL client is not thread safe.") << std::endl;
            return 1;
        }

        connectParams param(_T("tdap"), argv[1], _T("querytest"), _T("test"));
        param.setMode(0);

        bzs::test::worker::mysql::connectParam paramMysql(
            argv[1], _T("querytest"), _T("root"));

        commandLineParam cmd(argv);

        if (tableFixer(param))
        {
            std::vector<transactionSec> results;
            results.reserve(cmd.maxWorkerNum);
            pooledDbManager::setMaxConnections(cmd.maxWorkerNum);

            double totalTransactionTime = 0;
            bzs::rtl::benchmarkMt bm;
            bm.start();
            int totalTransactions =
                execute(cmd, param, paramMysql, results, totalTransactionTime);
            printf("\n");
            showResult(results, bm.end(), (int)totalTransactionTime,
                       totalTransactions, cmd.stressMode);
            pooledDbManager pdb;
            pdb.reset(0);
            //return 0;
        }
        //return 1;
    }

    catch (bzs::rtl::exception& e)
    {
        std::_tstring s = *bzs::rtl::getMsg(e);
        std::tcout << _T("[ERROR] ") << s << std::endl;
    }
#ifdef _MSC_VER
        _CrtDumpMemoryLeaks();
#endif
    return 1;
}
