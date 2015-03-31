#ifndef BZS_TEST_BENCH_WORKERBASE_H
#define BZS_TEST_BENCH_WORKERBASE_H
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
#include <boost/thread/barrier.hpp>
#include <boost/thread/mutex.hpp>
#include <bzs/rtl/benchmark.h>
#include <iostream>
#include <bzs/env/tstring.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#endif
namespace bzs
{
namespace test
{
namespace worker
{

extern int g_bench_signal;

#define BENCH_SIGNAL_BREAK 0
#define BENCH_SIGNAL_GO 1
#define BENCH_SIGNAL_GREEN 2
#define BENCH_SIGNAL_BLUE 3

class workerBase
{
protected:
    int m_id;
    int m_loopCount;
    int m_functionNumber;
    boost::barrier& m_sync;
    static boost::mutex m_mutex;
    int m_bresult;
    virtual void doExecute() = 0;
    virtual void endExecute(){};
    virtual void initExecute(){};
    std::_tstring dummyWork()
    {
        std::_tstring s;
        __int64 v = 0; 
        for (int i=1;i<100;++i)
        {
            v+=rand();
            v = v/i;
            _TCHAR tmp[30];
            _stprintf_s(tmp, 30, _T("random number %d ."), v);
            s += tmp;
        }
        return s;
    }
  

public:
    workerBase(int id, int loopCount, int functionNumber, boost::barrier& sync)
        : m_id(id), m_loopCount(loopCount), m_functionNumber(functionNumber),
          m_sync(sync){};
    virtual ~workerBase() {}

    void execute()
    {
        initExecute();
        bzs::rtl::benchmarkMt bm;
        m_sync.wait();

        bm.start();
        m_bresult = 0;
        while (g_bench_signal)
        {
            doExecute();
            if (g_bench_signal == BENCH_SIGNAL_GREEN)
                ++m_bresult;
            else if (g_bench_signal == BENCH_SIGNAL_BLUE)
            {
                m_bresult = bm.end();
                //dummyWork(); 
                break;
            }
        }

        endExecute();
    }
#ifdef _WIN32
    void execute2(HANDLE hHandle)
    {
        initExecute();
        bzs::rtl::benchmarkMt bm;
        WaitForSingleObject(hHandle, INFINITE);

        bm.start();
        m_bresult = 0;
        while (g_bench_signal)
        {
            doExecute();
            if (g_bench_signal == BENCH_SIGNAL_GREEN)
                ++m_bresult;
            else if (g_bench_signal == BENCH_SIGNAL_BLUE)
            {
                m_bresult = bm.end();
                //dummyWork(); 
                break;
            }
        }

        endExecute();
    }
#endif
    int total() const { return m_bresult; }
};

} // namespace worker
} // namespace test
} // namespace bzs

#endif // BZS_TEST_BENCH_WORKERBASE_H
