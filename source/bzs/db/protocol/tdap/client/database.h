#ifndef	BZS_DB_PROTOCOL_TDAP_CLIENT_DATABASE_H
#define	BZS_DB_PROTOCOL_TDAP_CLIENT_DATABASE_H
/* =================================================================
 Copyright (C) 2000-2013 BizStation Corp All rights reserved.

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
#include "nsDatabase.h"

namespace bzs
{
namespace db
{
namespace protocol
{
namespace tdap
{
namespace client
{

class database;
class table;
class repData;
class dbdef;

#if defined(__BORLANDC__)
	typedef bool __stdcall(*deleteRecordFn)(database* db, table* tb, bool inkey);
	typedef short __stdcall(*schemaMgrFn)(database* db);
	typedef void __stdcall(*copyDataFn)(database* db, int recordCount, int count, bool &cancel);
#else
	typedef bool(__STDCALL *deleteRecordFn)(database* db, table* tb, bool inkey);
	typedef short(__STDCALL *schemaMgrFn)(database* db);
	typedef void (__STDCALL *copyDataFn)(database* db, int recordCount, int count, bool &cancel);
#endif


class AGRPACK database : public nsdatabase
{
	struct dbimple* m_impl;
	void setDir(const _TCHAR* directory);
	virtual table* createTableObject();

protected:
	database& operator = (const database&);
	database();
	virtual ~database();
	void setLockReadOnly(bool v);
	virtual void doClose();
	virtual void doOpen(const _TCHAR* uri, short type, short mode, const _TCHAR* username);
	virtual bool onOpenAfter() {return true;};
	virtual bool onTableOpened(table* tb, short fileNum, short mode, bool isCreated) {return true;};
	virtual char* getContinuousList(int option);
	virtual void onCopyDataInternal(table* tb, int recordCount, int count, bool& cancel);
	virtual void doConvertTable(short tableIndex, bool turbo, const _TCHAR* ownerName);

public:
	virtual void release();
	dbdef* dbDef() const ;
	const _TCHAR* rootDir() const ;
	void setRootDir(const _TCHAR* directory);
	void* optionalData() const ;
	void setOptionalData(void* v);
	bool tableReadOnly() const ;
	void setTableReadOnly(bool value);
	const deleteRecordFn onDeleteRecord() const ;
	void setOnDeleteRecord(const deleteRecordFn v);
	const copyDataFn onCopyData() const;
	void setOnCopyData(const copyDataFn v);
	bool open(const _TCHAR* uri, short schemaType = 0, short mode = -2, const _TCHAR* dir = NULL,
		const _TCHAR* ownerName = NULL);
	table* openTable(short fileNum, short mode = TD_OPEN_NORMAL, bool autoCreate = true,
		const _TCHAR* ownerName = NULL, const _TCHAR* uri = NULL);
	table* openTable(const _TCHAR* tableName, short mode = 0, bool autoCreate = true,
		const _TCHAR* ownerName = NULL, const _TCHAR* uri = NULL);
	database* clone();
	bool createTable(short fileNum, const _TCHAR* uri = NULL);
	void create(const _TCHAR* uri, short type = TYPE_SCHEMA_BDF);
	void drop();
	void dropTable(const _TCHAR* tableName);
	void close();
	short continuous(char_td op = TD_BACKUP_START, bool inclideRepfile = false);
	short assignSchemaData(dbdef* src);
	short copyTableData(table* dest, table* src, bool turbo, int offset = 0, short keyNum = -1,
		int maxSkip = -1);
	void convertTable(short tableIndex, bool turbo, const _TCHAR* ownerName=NULL);
	bool existsTableFile(short tableIndex, const _TCHAR* ownerName=NULL);
	_TCHAR* getTableUri(_TCHAR* buf, short fileNum);
	void getBtrVersion(btrVersions* versions);
	bool isOpened() const ;
	virtual int defaultAutoIncSpace() const {return 0;};
	static database* create();			
	/* For C++ direct only. don't use by wrapper class for COM or SWIG 
	   This method is ignore refarence count of nsdatabse. 	
		*/
	static void destroy(database* db);  

};

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs

#endif//BZS_DB_PROTOCOL_TDAP_CLIENT_DATABASE_H
