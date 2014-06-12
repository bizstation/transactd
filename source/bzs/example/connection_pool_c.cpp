#include <bzs/db/protocol/tdap/client/connectionPool.h>
#include <iostream>

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
	void* con;
	char_td reserved[12 - sizeof(void*)];//32bit = 8 64bit = 4
	char_td     aid[2];
	ushort_td   id;
};




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
			begin_use_pool_database();

				dbmanager_ptr db = get_pool_database();

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
			end_use_pool_database();

			delete this;
		} //call releaseConnection

		catch(bzs::rtl::exception& e)
		{
			std::tcout << _T("[ERROR] ") << *bzs::rtl::getMsg(e) << std::endl;
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

static const int worktime[10] = {5, 1, 3, 5, 4, 1, 2, 5, 4, 1};


#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
	try
	{
		connectParams param(_T("tdap"), _T("localhost"), _T("test"), _T("test"));
		//create four databases
		cpool.reserve(4, param);

		//Execute 10 workers with each thread.
		thread_group threads;
		for (int i=0;i<10;++i)
		{
			worker* w = new worker(i+1, worktime[i]*100);
			threads.create_thread( bind(&worker::execute, w));
		}
		threads.join_all();
		std::cout << "Connection pool test success." << std::endl;

		return 0;
	}

	catch(bzs::rtl::exception& e)
	{
		std::tcout << _T("[ERROR] ") << *bzs::rtl::getMsg(e) << std::endl;
	}
	return 1;
}
