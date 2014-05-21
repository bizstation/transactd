/*=================================================================
   Copyright (C) 2012 2013 BizStation Corp All rights reserved.

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

#include "dbManager.h"
#include <bzs/netsvc/server/IAppModule.h> //for result value macro.
#include <bzs/rtl/exception.h>
#include <time.h>


namespace bzs
{
namespace db
{
namespace engine
{
namespace mysql
{

std::string smartDbsReopen::removeName="";





dbManager::dbManager():m_autoHandle(0)
{
}


dbManager::~dbManager()
{
	
}

bool dbManager::isShutDown() const
{
	boost::mutex::scoped_lock lck(m_mutex);
#if defined(MARIADB_BASE_VERSION)
	killed_state st = NOT_KILLED;
#else
	THD::killed_state st = THD::NOT_KILLED;
#endif
	for (size_t i=0;i<m_dbs.size();i++)
		if ((m_dbs[i]!=NULL) && (m_dbs[i]->thd()->killed != st))
			return true;
	return false;
}

void dbManager::checkNewHandle(int newHandle)const
{
	for (size_t i=0;i<m_handles.size();i++)
		if(m_handles[i].id == newHandle)
			THROW_BZS_ERROR_WITH_CODEMSG(1, "Allready exits handle.");
}

void dbManager::releaseDatabase(short cid)
{
	boost::mutex::scoped_lock lck(m_mutex);
	int index = -1;
	for (size_t i=0;i<m_dbs.size();i++)
	{
		if ((m_dbs[i]!=NULL) && (cid==m_dbs[i]->clientID()))
		{
			index = (int)i;
			break;
		}
	}
	if (index==-1)
		return ;
	//close tables release thd
	m_dbs[index].reset();

	//erase handles
	for (int i=(int)m_handles.size()-1;i>=0;i--)
		if(m_handles[i].db == index)
			m_handles.erase(m_handles.begin()+i);

}

database* dbManager::useDataBase(int id) const
{
	if (id >= (int)m_dbs.size())
		THROW_BZS_ERROR_WITH_CODEMSG(1, "Invalid database id.");
	if (m_dbs[id]==NULL)
		THROW_BZS_ERROR_WITH_CODEMSG(1, "Invalid database id.");
	m_dbs[id]->use(); 
	return m_dbs[id].get();
}

database* dbManager::createDatabase(const char* dbname, short cid)const
{
	return new database(dbname,  cid);
}

handle* dbManager::getHandle(int handle)const
{
	for (size_t i=0;i<m_handles.size();i++)
	{
		if(m_handles[i].id == handle)
			return &m_handles[i];
	}
	THROW_BZS_ERROR_WITH_CODEMSG(1, "Invalid handle.");
}

int dbManager::getDatabaseID(short cid) const 
{
	for (size_t i=0;i<m_dbs.size();i++)
	{
		if (m_dbs[i]!=NULL && (m_dbs[i]->clientID()==cid))
			return (int)i;
	}
	return -1;
}

database* dbManager::getDatabaseCid(short cid) const 
{
	int id = getDatabaseID(cid); 
	if (id == -1)
		THROW_BZS_ERROR_WITH_CODEMSG(1, "Can not create database object.");
	
	return useDataBase(id);
}

database* dbManager::getDatabase(const char* dbname, short cid) const 
{
	int id = getDatabaseID(cid); 
	if (id == -1)
	{
		 boost::shared_ptr<database> db(createDatabase(dbname, cid));
		if (db ==NULL)
			THROW_BZS_ERROR_WITH_CODEMSG(1, "Can not create database object.");
		m_dbs.push_back(db);
		id = (int)m_dbs.size()-1;
	}
	return useDataBase(id);
}

table* dbManager::getTable(int hdl, enum_sql_command cmd)const
{
	handle* h = getHandle(hdl);	
	if (h && (h->db < (int)m_dbs.size()))
		return useDataBase(h->db)->useTable(h->tb, cmd);


	THROW_BZS_ERROR_WITH_CODEMSG(1, "Invalid handle.");
}

int dbManager::addHandle(int dbid, int tableid,int assignid)
{
	++m_autoHandle;
	if (assignid == -1)
		assignid = m_autoHandle;
	m_handles.push_back(handle(assignid, (short)dbid, (short)tableid));
	return assignid;
}

int dbManager::ddl_execSql(THD* thd, const std::string& sql_stmt)
{
	smartDbsReopen reopen(m_dbs);
	
	thd->clear_error();
	int result = dispatch_command(COM_QUERY, thd,(char*)sql_stmt.c_str() , (uint)sql_stmt.size());
	if (!thd->cp_isOk())
		result = 1;
	if (thd->is_error())
		result = errorCode(thd->cp_get_sql_error());
	return result;
}

int dbManager::ddl_createDataBase(THD* thd,  const std::string& dbname )
{
	std::string cmd = "create database `" + dbname + "`";
	return ddl_execSql(thd, cmd);			

}

int dbManager::ddl_dropDataBase(THD* thd,  const std::string& dbname,  const std::string& dbSqlname)
{
	std::string cmd = "drop database `" + dbSqlname + "`";
	smartDbsReopen::removeName = dbname;
	int ret = ddl_execSql(thd, cmd);
	smartDbsReopen::removeName = "";
	for (int i=(int)m_dbs.size()-1;i>=0;i--)
	{
		if (m_dbs[i] && (m_dbs[i]->name() == dbname))
			m_dbs.erase(m_dbs.begin()+i);
	}
	return ret;
}

int dbManager::ddl_useDataBase(THD* thd, const std::string& dbSqlname)
{
	std::string cmd = "use `" + dbSqlname + "`";
	return ddl_execSql(thd, cmd);
}

int dbManager::closeCacheTable(database* db, const std::string& tbname)
{
	if (database::tableRef.count(db->name(), tbname))
		return DBM_ERROR_TABLE_USED;

	TABLE_LIST tables;
	tables.init_one_table(db->name().c_str(), db->name().size(), tbname.c_str(), tbname.size(), tbname.c_str(), TL_READ);
	if(close_cached_tables(db->thd(), &tables, true, 50000000L))  
		return HA_ERR_LOCK_WAIT_TIMEOUT;
	return 0;

}

int dbManager::ddl_dropTable(database* db, const std::string& tbname,  const std::string& dbSqlname 
				,  const std::string& tbSqlname)
{
	db->closeTable(tbname.c_str(), true);
	int ret = closeCacheTable(db, tbname);
	if (ret) return ret;
	db->thd()->variables.lock_wait_timeout = 0;
	std::string cmd = "drop table `" + dbSqlname + "`.`" + tbSqlname + "`";
	return ddl_execSql(db->thd(), cmd);
	
}

int dbManager::ddl_renameTable(database* db, const std::string& oldName, const std::string& dbSqlName 
				, const std::string& oldSqlName, const std::string& newSqlName)
{
	db->closeTable(oldName.c_str(), true);
	int ret = closeCacheTable(db, oldName);
	if (ret) return ret;

	std::string cmd = "rename table `" + dbSqlName + "`.`" + oldSqlName + "` to `" + dbSqlName + "`.`" + newSqlName + "`";
	return ddl_execSql(db->thd(), cmd);
}

int dbManager::ddl_replaceTable(database* db , const std::string& name1, const std::string& name2
			, const std::string& dbSqlName, const std::string& nameSql1, const std::string& nameSql2)
{  // rename name1 to name2. 
	db->closeTable(name1.c_str(), true);
	db->closeTable(name2.c_str(), true);
	char nameSql3[255];

	time_t timer_ = time(NULL);
	struct tm*  t = localtime(&timer_);
	sprintf(nameSql3, "%s_trsctd_%4d%02d%02d_%02d%02d%02d", nameSql1.c_str()
			, t->tm_year + 1900 , t->tm_mon + 1	, t->tm_mday
			, t->tm_hour, t->tm_min, t->tm_sec);

	int ret = closeCacheTable(db, name2);
	if (ret) return ret;

	std::string cmd = "rename table `" + dbSqlName + "`.`" + nameSql2 + "` to `" + dbSqlName + "`.`" + nameSql3
		+ "`,`" + dbSqlName + "`.`" + nameSql1 + "` to `" + dbSqlName + "`.`" + nameSql2 + "`";		
	ret = ddl_execSql(db->thd(), cmd);	
	if (ret == 0)
	{
		std::string cmd = "drop table `" + dbSqlName + "`.`" + nameSql3 + "`";
		return ddl_execSql(db->thd(), cmd);
	}
	return ret;

}

std::string dbManager::makeSQLChangeTableComment(const std::string& dbSqlName, const std::string& tableSqlName, const char* comment)
{
	std::string s = "alter table `" + dbSqlName + "`.`" + tableSqlName + "` comment \"" + comment + "\"";
	return s;
}

/** Key name of multi byte charctord is not supported. Use only ascii.
*/
std::string dbManager::makeSQLDropIndex(const std::string& dbSqlName, const std::string& tbSqlName, const char* name)
{
	std::string s = "drop index `" + std::string(name) + "` on `" + dbSqlName + "`.`" +tbSqlName + "`";
	return s;
}

void dbManager::clenupNoException()
{
	try
	{
		if (m_tb)m_tb->unUse();
	}
	catch(...){}
}

}//namespace mysql
}//namespace engine
}//namespace db
}//namespace bzs
