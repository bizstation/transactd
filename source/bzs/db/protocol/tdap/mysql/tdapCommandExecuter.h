#ifndef BZS_DB_PROTOCOL_TDAP_MYSQL_TDAPCOMMANDEXECUTER_H
#define BZS_DB_PROTOCOL_TDAP_MYSQL_TDAPCOMMANDEXECUTER_H
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

#include <bzs/db/engine/mysql/dbManager.h>
#include <bzs/db/protocol/ICommandExecuter.h>
#include "request.h"
#include <bzs/env/crosscompile.h>

extern int g_tableNmaeLower;

namespace bzs
{	
namespace db
{
	class blobBuffer;
namespace protocol
{
namespace tdap
{
namespace mysql
{

class ReadRecordsHandler;
struct extRequest;
struct extRequestSeeks;

#define  FOR_SQL true

std::string getDatabaseName(const request& req, bool forSql=false);
std::string getTableName(const request& req, bool forSql=false);
short_td errorCode(int ha_error);
bool isMetaDb(const request& req);

class dbExecuter : public engine::mysql::dbManager
{
	ReadRecordsHandler* m_readHandler;
	blobBuffer*  m_blobBuffer;

	void connect(request& req);
	void releaseDatabase(request& req, int op);
	std::string makeSQLcreateTable(const request& req);
	inline void doCreateTable(request& req);
	inline void doOpenTable(request& req);
	inline void doSeekKey(request& req, int op);
	inline void doMoveFirst(request& req);
	inline void doMoveKey(request& req, int op);
	inline int  doReadMultiWithSeek(request& req, int op, char* resultBuffer, size_t& size, netsvc::server::buffers* optionalData);
	inline int  doReadMulti(request& req, int op, char* resultBuffer, size_t& size, netsvc::server::buffers* optionalData);
	inline void doStepRead(request& req, int op);


	inline void doInsert(request& req);
	inline void doUpdate(request& req);
	inline void doUpdateKey(request& req);
	inline void doDelete(request& req);
	inline void doDeleteKey(request& req);
	inline void doInsertBulk(request& req);
	inline void doStat(request& req);
	inline short seekEach(extRequestSeeks* ereq);
public:
	dbExecuter();
	~dbExecuter();
	int commandExec(request& req, netsvc::server::IResultBuffer& result, size_t& size, netsvc::server::buffers* optionalData);
	int errorCode(int ha_error);
	short_td errorCodeSht(int ha_error){return (short_td)errorCode(ha_error);};
	
};

/** Command dispatcher for connectionManager
 */
class connMgrExecuter
{
	request& m_req;
	__int64 m_modHandle;
public:
	connMgrExecuter(request& req, unsigned __int64 parent);
	int read(char* buf, size_t& size);
	int disconnectOne(char* buf, size_t& size);
	int disconnectAll(char* buf, size_t& size);
	int commandExec(netsvc::server::IResultBuffer& buf, size_t& size);
};

class commandExecuter :  public ICommandExecuter, public engine::mysql::igetDatabases
{
	mutable request m_req;
	boost::shared_ptr<dbExecuter> m_dbExec;

	int readStatistics(char* buf, size_t& size);
	int cmdStatistics(char* buf, size_t& size);
	unsigned __int64 m_modHandle;
public:
	commandExecuter(__int64 parent);
	~commandExecuter();
	size_t perseRequestEnd(const char* p, size_t size, bool& comp)const;

	size_t getAcceptMessage(char* message, size_t size)
	{
		strcpy_s(message, size, "200 OK\n");
		return strlen(message);
	}

	bool parse(const char* p, size_t size);

	//int execute(char* resultBuffer, size_t& size, netsvc::server::buffers* optionalData)
	int execute(netsvc::server::IResultBuffer& resultBuffer, size_t& size, netsvc::server::buffers* optionalData)
	{
		if (m_req.op == TD_STASTISTICS)
			return connMgrExecuter(m_req, m_modHandle).commandExec(resultBuffer, size);
		return m_dbExec->commandExec(m_req, resultBuffer, size, optionalData);
	}
	
	bool isShutDown(){return m_dbExec->isShutDown();}
	
	void cleanup(){};
	
	const engine::mysql::databases& dbs() const{return m_dbExec->dbs();};
};

}//namespace mysql
}//namespace tdap
}//namespace protocol
}//namespace db
}//namespace bzs

#endif //BZS_DB_PROTOCOL_TDAP_MYSQL_TDAPCOMMANDEXECUTER_H

