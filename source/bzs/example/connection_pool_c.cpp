#include <bzs/db/protocol/tdap/client/trdboostapi.h>
#include <iostream>
#include <vector>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>



using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;
using namespace boost;

/**
@brief make connection pool example


This program makes connection pool of four databases.
And execute ten worker jobs with each thread.
Each worker shows the connection object pointer.

*/


struct clientID
{

#ifdef __x86_64__
    void* con;
    char_td reserved[4];
#else
    void* con;
    char_td reserved[8];
#endif
    char_td     aid[2];
    ushort_td   id;
};


/** connection pool class
    Hold database instance and delivery.
*/
class connectionPool
{
    std::vector<database_ptr> m_dbs;
    mutable mutex m_mutex;
    mutable mutex m_mutex2;

    mutable condition m_busy;

public:
    connectionPool(){};

    /** delivery database instance*/
    database_ptr get()const
    {
        mutex::scoped_lock lck(m_mutex);
        while (1)
        {
            for (size_t i = 0;i<m_dbs.size();i++)
            {
                if (m_dbs[i].use_count() == 1)
                    return m_dbs[i];

            }
            mutex::scoped_lock lck(m_mutex2);
            m_busy.wait(lck);
      
        }
    }

    /** create database and login the server with each connection*/
    void create(size_t size, const connectParams& param)
    {
        for (size_t i =0;i<size;++i)
        {
            database_ptr db = createDatadaseObject();
            connect(db, param, true/* new connection*/);
            openDatabase(db, param);
            m_dbs.push_back(db);
        }
    }
    
    void releaseOne()
    {
        m_busy.notify_one();
    }

};

connectionPool cpool;

void releaseConnection(connectionPool* pool)
{
    pool->releaseOne();
}

class worker
{
   int m_id;
   int m_worktime;
   static mutex m_mutex;
public:
    worker(int id, int worktime):m_id(id),m_worktime(worktime){};
    void execute()
    {
        try
        {
            shared_ptr<connectionPool> pool(&cpool, releaseConnection);
            {
                database_ptr db = pool->get();
            
                clientID* cid = (clientID*)db->clientID();
        
                {
                    mutex::scoped_lock lck(m_mutex);
                    std::cout << "worker strat id = " << m_id
                                << " connection = 0x" << std::hex << cid->con << std::endl;
                }
        
                Sleep(m_worktime);
                if (m_id == 4) throw "error";    //throw error example

                {
                    mutex::scoped_lock lck(m_mutex);
                    std::cout << "worker finish id = " << m_id
                            << " connection = 0x" << std::hex << cid->con << std::endl;
                }
            } // release database

            delete this;
        } //call releaseConnection

        catch(...)
        {
            mutex::scoped_lock lck(m_mutex);
            std::cout << "worker error id = " << m_id << std::endl;
            delete this;
        }
    }

};
mutex worker::m_mutex;

static const worktime[10] = {5, 1, 3, 5, 4, 1, 2, 5, 4, 1};

#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
    try
    {
        connectParams param(_T("tdap"), _T("localhost"), _T("test"), _T("test"));
        //create four databases
        cpool.create(4, param);

        //Execute 10 workers with each thread.
        thread_group threads;
        for (int i=0;i<10;++i)
        {
            worker* w = new worker(i+1, worktime[i]*100);
            threads.create_thread( bind(&worker::execute, w));
        }
        threads.join_all();
        return 0;
    }

    catch(bzs::rtl::exception& e)
    {
        std::tcout << *bzs::rtl::getMsg(e) << std::endl;
    }
    return 1;
}
