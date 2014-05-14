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
#include <bzs/env/tstring.h>
#pragma hdrstop

#include "database.h"

#include "table.h"

#include "dbDef.h"
#include <limits.h>
#include <sys/stat.h>
#include <stdio.h>
#include <vector>
#include "stringConverter.h"

#pragma package(smart_init)

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

struct dbimple
{

	_TCHAR rootDir[MAX_PATH];
	bool isOpened;
	dbdef* dbDef;
	void* optionalData;
	bool isTableReadOnly;
	bool lockReadOnly;
	deleteRecordFn m_deleteRecordFn;
	copyDataFn m_copyDataFn;

	dbimple() : dbDef(NULL), isOpened(false), m_deleteRecordFn(NULL),m_copyDataFn(NULL), optionalData(NULL),
		isTableReadOnly(false), lockReadOnly(false)
	{
		rootDir[0] = 0x00;
	}

	dbimple& operator = (const dbimple & rt)
	{
		if (&rt != this)
		{
			dbDef = rt.dbDef;
			_tcscpy(rootDir, rt.rootDir);
			isOpened = rt.isOpened;
			optionalData = rt.optionalData;
			isTableReadOnly = rt.isTableReadOnly;
			lockReadOnly = rt.lockReadOnly;
			m_deleteRecordFn = rt.m_deleteRecordFn;
			m_copyDataFn = rt.m_copyDataFn;

		}
		return *this;

	}
};

void database::destroy(database* db) {delete db;}

database::database() : nsdatabase() 
{
	m_impl = new dbimple();
}

database::~database()
{
	close();
	delete m_impl;
}

void database::release()
{
	if (m_impl->dbDef)
	{
		m_impl->dbDef->release();
		m_impl->dbDef = NULL;
	}
	nsdatabase::release();
}

dbdef* database::dbDef() const {return m_impl->dbDef;}

const _TCHAR* database::rootDir() const {return m_impl->rootDir;}

void database::setRootDir(const _TCHAR* directory) {setDir(directory);}

void* database::optionalData() const {return m_impl->optionalData;}

void database::setOptionalData(void* v) {m_impl->optionalData = v;}

bool database::tableReadOnly() const {return m_impl->isTableReadOnly;}

const deleteRecordFn database::onDeleteRecord() const {return m_impl->m_deleteRecordFn;}

void database::setOnDeleteRecord(const deleteRecordFn v) {m_impl->m_deleteRecordFn = v;}

const copyDataFn database::onCopyData() const {return m_impl->m_copyDataFn;}

void database::setOnCopyData(const copyDataFn v) {m_impl->m_copyDataFn = v;}

void database::setLockReadOnly(bool v) {m_impl->lockReadOnly = v;}

bool database::isOpened() const {return m_impl->isOpened;}

void database::create(const _TCHAR* fullpath, short type)
{
	if (!m_impl->dbDef)
		m_impl->dbDef = new dbdef(this, type); // Create TabelDef here.
	m_impl->dbDef->create(fullpath);
	m_stat = m_impl->dbDef->stat();
}

void database::drop()
{
	if (m_impl->dbDef == NULL)
		m_stat = STATUS_DB_YET_OPEN;
	_TCHAR FullPath[MAX_PATH];
	std::vector<std::_tstring>fileNames;
	for (int i = 0; i <= m_impl->dbDef->tableCount(); i++)
	{
		if (m_impl->dbDef->tableDefs(i))
		{
			_stprintf_s(FullPath, MAX_PATH, _T("%s") PSEPARATOR _T("%s"), rootDir(),
				m_impl->dbDef->tableDefs(i)->fileName());
			fileNames.push_back(FullPath);
		}
	}
	fileNames.push_back(m_impl->dbDef->uri());
	close();
	if (m_stat)
		return;

	for (size_t i = 0; i < fileNames.size(); i++)
	{
		nsdatabase::dropTable(fileNames[i].c_str());
		if (m_stat && (m_stat == STATUS_TABLE_NOTOPEN))
			return;
	}
}

void database::dropTable(const _TCHAR* TableName)
{
	_TCHAR FullPath[MAX_PATH];
	_tcscpy(FullPath, rootDir());
	_tcscat(FullPath, PSEPARATOR);
	short index = m_impl->dbDef->tableNumByName(TableName);
	if (index == -1)
		m_stat = STATUS_TABLENAME_NOTFOUND;
	else
		_tcscat(FullPath, m_impl->dbDef->tableDefs(index)->fileName());
	nsdatabase::dropTable(FullPath);
}

void database::setDir(const _TCHAR* directory)
{
   _tcscpy(m_impl->rootDir, directory);
}

database& database:: operator = (const database & rt)
{
	if (&rt != this)
	{
		nsdatabase::operator=(rt);
		m_impl->dbimple:: operator=(*(rt.m_impl));
		rt.m_impl->dbDef->addref();
	}
	return *this;
}

database* database::clone()
{
	if (!m_impl->dbDef)
		m_impl->dbDef = new dbdef(this, TYPE_SCHEMA_BDF);
	database* p = new database();
	*p = *this;
	return p;
}

void database::getBtrVersion(btrVersions* versions)
{
	uchar_td* posblk = NULL;
	if (m_impl->dbDef)
		posblk = const_cast<uchar_td*>(m_impl->dbDef->posblk());
	nsdatabase::getBtrVersion(versions, posblk);

}
void database::onCopyDataInternal(table* tb, int recordCount, int count, bool& cancel)
{
	if (m_impl->m_copyDataFn)
		m_impl->m_copyDataFn(this, recordCount, count, cancel);
}

void database::setTableReadOnly(bool value)
{
	if (!m_impl->lockReadOnly)
		m_impl->isTableReadOnly = value;
}

void database::doOpen(const _TCHAR* uri, short type, short mode, const _TCHAR* ownername)
{
	m_stat = STATUS_SUCCESS;
	m_impl->dbDef->setDefType(type);
	m_impl->dbDef->open(uri, (char_td)mode, ownername);

	if ((m_stat == STATUS_SUCCESS) &&
		 (m_impl->dbDef->m_stat == STATUS_INVALID_OWNERNAME) && (type == 0))
		m_impl->dbDef->m_stat = STATUS_DIFFERENT_DBVERSION;

	m_stat = m_impl->dbDef->m_stat;
	m_impl->isOpened = (m_stat == STATUS_SUCCESS);//important
}

bool database::open(const _TCHAR* _uri, short type, short mode, const _TCHAR* dir,
	const _TCHAR* ownername)
{

	_TCHAR buf[MAX_PATH];
	m_stat = STATUS_SUCCESS;
	if (!m_impl->isOpened)
	{
		if (setUri(_uri) == false)
		{
			m_stat = 11;
			return false;
		}
		if (dir && dir[0])
		{
			setDir(dir);
			if (m_stat)
				return false;
		}
		else
		{
			if (m_impl->rootDir[0] == 0x00)
			{
				nstable::getDirURI(uri(), buf);
				setDir(buf);
				if (m_stat)
					return false;
			}
		}
		if (!m_impl->dbDef)
			m_impl->dbDef = new dbdef(this, type);

		doOpen(_uri, type, mode, ownername);
		if ((m_stat == STATUS_TABLE_NOTOPEN) && isUseTransactd()
								&& _tcsstr(_uri, TRANSACTD_SCHEMANAME))
		{
			// Specified TRANSACTD_SCHEMANAME and no table
			// Auto make schema.
			create(_uri, TYPE_SCHEMA_BDF);
			if (m_stat == STATUS_SUCCESS)
			{
				doOpen(_uri, type, mode, ownername);
				if (m_stat == STATUS_SUCCESS)
				{
					m_impl->dbDef->autoMakeSchema();
					m_impl->dbDef->close();
					doOpen(_uri, type, mode, ownername);
				}
			}
		}

	}
	if (m_impl->isOpened && onOpenAfter())
		return true;

	m_impl->isOpened = false;
	m_impl->dbDef->close();
	m_impl->dbDef->release();
	m_impl->dbDef = NULL;
	return false;
}

char* database::getContinuousList(int option)
{
	char* fileList = (char*)malloc(64000);
	memset(fileList, 0, 64000);
	tabledef* TableDef;
	_TCHAR buf[MAX_PATH];
	char tmp[MAX_PATH];
	char* tmpPtr;
	for (short i = 0; i < m_impl->dbDef->tableCount(); i++)
	{
		TableDef = m_impl->dbDef->tableDefs(i);
		if (TableDef)
		{
			getTableUri(buf, TableDef->id);
			tmpPtr = (char*)toChar(tmp, buf, MAX_PATH);
			char* p = strstr(tmpPtr, PSEPARATOR_A PSEPARATOR_A);
			if (p)
				p = strstr(p + 2, PSEPARATOR_A);
			if (p == NULL)
				p = tmpPtr;
			strcat(fileList, p);
			strcat(fileList, ",");
		}
	}
	tmpPtr = (char*)toChar(tmp, uri(), MAX_PATH);
	strcat(fileList, tmpPtr); // add schema table too.
	return fileList;
}

short database::continuous(char_td IsEnd, bool inclideRepfile)
{ // Local databse only.Cnat not use remote database.
	if (!m_impl->isOpened)
		return STATUS_DB_YET_OPEN;
	char tmp[128] = {0x00};
	char* buf = getContinuousList(inclideRepfile);
	uint_td buflen = (uint_td)strlen(buf) + 1;
	m_stat = m_btrcallid(TD_BACKUPMODE, tmp, buf, &buflen, 0, 0, IsEnd, clientID());
	free(buf);
	return m_stat;
}

void database::doClose()
{
	m_stat = STATUS_SUCCESS;

	if (m_impl->dbDef)
		m_impl->dbDef->release();
	m_impl->dbDef = NULL;

	nsdatabase::reset();
	m_impl->isOpened = false;
	m_impl->rootDir[0] = 0x00;
	m_impl->lockReadOnly = false;
}

void database::close() {doClose();}

_TCHAR* database::getTableUri(_TCHAR* buf, short FileNum)
{
	m_stat = STATUS_SUCCESS;
	if ((m_impl->dbDef) && (m_impl->isOpened))
	{
		if (_tcsstr(m_impl->dbDef->tableDefs(FileNum)->fileName(), PSEPARATOR) == NULL)
			_stprintf_s(buf, MAX_PATH, _T("%s") PSEPARATOR _T("%s"), m_impl->rootDir,
			m_impl->dbDef->tableDefs(FileNum)->fileName());
		else
			_tcscpy(buf, m_impl->dbDef->tableDefs(FileNum)->fileName());
		return buf;
	}
	m_stat = STATUS_DB_YET_OPEN;
	return NULL;
}

table* database::openTable(const _TCHAR* TableName, short mode, bool AutoCreate,
	const _TCHAR* OrnerName, const _TCHAR* pPath)
{
	short filenum;
	m_stat = 0;
	if ((m_impl->dbDef) && (m_impl->isOpened))
	{
		filenum = m_impl->dbDef->tableNumByName(TableName);
		if (filenum == -1)
		{
			m_stat = m_impl->dbDef->m_stat;
			if (m_stat == STATUS_SUCCESS)
				m_stat = STATUS_TABLENAME_NOTFOUND;

			return NULL;
		}
		return openTable(filenum, mode, AutoCreate, OrnerName, pPath);
	}
	m_stat = STATUS_DB_YET_OPEN;
	return NULL;
}

table* database::createTableObject()
{
	return new table(this);
}

table* database::openTable(short FileNum, short mode, bool AutoCreate, const _TCHAR* OrnerName,
	const _TCHAR* path)
{
	/* Select directory
	- Fiest, Specify Direct.
	- Second, specified in filename.
	- Thard, Smae as schem table.
	*/

	_TCHAR buf[MAX_PATH];
	bool regularDir = false;
	bool NewFile = false;
	m_stat = 0;

	if ((!m_impl->dbDef) || (!m_impl->isOpened)) {
		m_stat = STATUS_DB_YET_OPEN;
		return NULL;
	}
	if (m_impl->rootDir[0] == 0x00)
	{
		m_stat = STATUS_DB_YET_OPEN;
		return NULL;
	}
	tabledef* td = m_impl->dbDef->tableDefs(FileNum);
	if (!td)
	{
		m_stat = STATUS_INVALID_TABLE_IDX;
		return NULL;
	}
	table* tb = createTableObject();
	dbdef::cacheFieldPos(td);

	if ((path == NULL) || (path[0]==0x00))
	{
		if (_tcsstr(td->fileName(), PSEPARATOR) == NULL)
		{
			getTableUri(buf, FileNum);
			regularDir = true;
		}
		else
			_tcscpy(buf, td->fileName());
	}
	else
		_tcscpy(buf, path);

	if (m_impl->isTableReadOnly)
		mode = TD_OPEN_READONLY;
	tb->open(buf, (char_td)mode, OrnerName);
	if ((tb->m_stat == STATUS_TABLE_NOTOPEN) || (tb->m_stat == ERROR_NOSPECIFY_TABLE))
	{
		if (AutoCreate)
		{
			createTable(FileNum, buf);
			if (m_stat != STATUS_SUCCESS)
			{
				m_stat = tb->m_stat;
				tb->release();
				return NULL;
			}
			else
			{
				tb->open(buf, (char_td)mode);
				if ((OrnerName) && (OrnerName[0]))
					tb->setOwnerName(OrnerName);
				NewFile = true;
			}
		}else
		{
			m_stat = tb->m_stat;
			tb->release();
			return NULL;
		}
	}
	tb->init(td, FileNum, regularDir);

	if ((m_stat != 0) || (tb->m_stat != 0) || !onTableOpened(tb, FileNum, mode, NewFile))
	{
		m_stat = tb->m_stat;
		tb->release();
		return NULL;
	}
	return tb;
}

bool database::createTable(short FileNum, const _TCHAR* FilePath)
{
	if (isTransactdUri(FilePath))
	{
		if (setUseTransactd() == false)
			return false;

		char buf2[MAX_PATH]={0x00};
		_TCHAR posblk[128] = {0x00};

		const char* p = toServerUri(buf2, MAX_PATH, FilePath, isUseTransactd());

		m_stat = m_btrcallid(TD_CREATETABLE, posblk, m_impl->dbDef->tableDefs(FileNum),
			&m_impl->dbDef->m_datalen, (void*)p, (uchar_td)strlen(p), CR_SUBOP_BY_TABLEDEF /* exists check */ ,
			clientID());
	}
	else
	{
		const _TCHAR* buf;
		fileSpec* fs = (fileSpec*) malloc(1024);
		if (fs == NULL)
		{
			m_stat = STATUS_CANT_ALLOC_MEMORY;
			return false;
		}
		m_impl->dbDef->getFileSpec(fs, FileNum);
		if (FilePath)
			buf = FilePath;
		else
			buf = m_impl->dbDef->tableDefs(FileNum)->fileName();
		nsdatabase::createTable(fs, 1024, buf, CR_SUBOP_BY_FILESPEC);
		free(fs);
	}
	return (m_stat == 0);
}



short database::assignSchemaData(dbdef* src)
{
	beginTrn();
	int Count;

	Count = 1;

	dbdef* defDest = dbDef();
	int recordCount = src->tableCount();

	for (int i=0;i<=src->tableCount();i++)
	{
		tabledef* td = src->tableDefs(i);
		if (td)
		{
			tabledef tdtmp = *td;
			tdtmp.fieldCount = 0;
			tdtmp.keyCount = 0;
			defDest->insertTable(&tdtmp);
			if (defDest->stat())
				break;
			for (int j=0;j<td->fieldCount;++j)
			{
				fielddef& fd = td->fieldDefs[j];
				*defDest->insertField(td->id, j) = fd;
			}
			for (int j=0;j<td->keyCount;++j)
			{
				keydef& kd = td->keyDefs[j];
				*defDest->insertKey(td->id, j) = kd;
			}
			defDest->updateTableDef(td->id);
			if (defDest->stat()) break;
		}
		bool Cancel = false;
		onCopyDataInternal(NULL, recordCount, Count, Cancel);
		if (Cancel)
			return -1;
		Count++;
	}
	

	if ((nstable::tdapErr((HWND)NULL, src->stat()) == 0) && (defDest->stat() == 0))
	{
		endTrn();
		return 0;
	}
	abortTrn();
	if (nstable::tdapErr((HWND)NULL, src->stat()))
		return src->stat();

	return defDest->stat();
}

struct filedChnageInfo
{
	filedChnageInfo():fieldnum(-1),changed(0){}
	short fieldnum;
	bool changed;
};

void makeChangeInfo( const tabledef* ddef, const tabledef* sdef, filedChnageInfo* fci )
{
	for (short i = 0; i < sdef->fieldCount; i++)
	{
		fielddef& fds = sdef->fieldDefs[i];
		for (short j = 0; j < ddef->fieldCount; j++)
		{
			fielddef& fdd = ddef->fieldDefs[j];
			if (strcmp(fdd.nameA(), fds.nameA()) == 0)
			{
				fci[i].fieldnum = j;
				if (fds.type != fdd.type)
					fci[i].changed = true; //diffrent type
				else if (fds.len != fdd.len)
					fci[i].changed = true; //different size
				break;
			}
			else
				fci[i].fieldnum = -1;
		}
	}

}

inline void copyEachFiledData(table* dest, table* src, filedChnageInfo* fci)
{
	const tabledef* ddef = dest->tableDef();
	const tabledef* sdef = src->tableDef();

	for (int i=0; i<sdef->fieldCount; i++)
	{
		int dindex = fci[i].fieldnum;
		fielddef& fds = sdef->fieldDefs[i];
		fielddef& fdd = ddef->fieldDefs[dindex];

		if (dindex != -1)
		{
			//src valiable len and last field;
			if (fci[i].changed == false)
			{
				int len = fds.len;
				if (fds.len > fdd.len)
					len = fdd.len;
				memcpy(dest->fieldPtr(dindex), src->fieldPtr(i), len);
			}else
			{
				if (fdd.maxVarDatalen() && fds.maxVarDatalen())
				{
					uint_td size;
					uint_td maxlen = fdd.maxVarDatalen();
					const void* data = src->getFVbin(i, size);
					if (maxlen < size)
						size = maxlen;
					dest->setFV(dindex, data, size);
				}else
				{
					// If diffrent field type then convert to string then copy.
					dest->setFV(dindex, src->getFVstr(i));
				}

			}
		}
	}
}

inline int moveVaileRecord(table* src)
{
	int count = 0;
	bookmark_td bm = 0;
	src->stepLast();
	while (src->stat() == STATUS_SUCCESS)
	{
		bm = src->bookmark();
		++count;
		src->stepPrev();
	}
	if (count)
	{
		src->seekByBookmark(bm);
		return count;
	}
	return 0;
}

inline void moveNextRecord(table* src, short keyNum)
{
	if (keyNum == -1)
		src->stepNext();
	else
		src->seekNext();
}

inline void moveFirstRecord(table* src, short keyNum)
{
	if (keyNum == -1)
		src->stepFirst();
	else
		src->seekFirst();
}

/*  Copy from src to dest table.
 * 	Copy as same field name.
 *	If turbo then copy use memcpy and offset dest of first address.
 *  if a src field is variable size binary, that dest field needs to be variable size binary.
 *  if src and dest fields are different type ,then a text copy is used.
 */
#pragma warn -8004
short database::copyTableData(table* dest, table* src, bool turbo, int offset, short keyNum,
	int maxSkip)
{
	src->setKeyNum((char_td)keyNum);
	const tabledef* ddef = dest->tableDef();
	const tabledef* sdef = src->tableDef();
	ushort_td ins_rows = 0;
	bool repData = (_tcsstr(ddef->fileName(), _T("rep.dat"))) ? true:false;
	int skipCount = 0, count = 1;
	int recordCount = src->recordCount();
	filedChnageInfo fci[256];

	makeChangeInfo(ddef, sdef, fci);
	moveFirstRecord(src, keyNum);

	while (1)
	{
		if (src->stat())
		{
			while (src->stat() != STATUS_EOF)
			{
				if (maxSkip != -1)
					break;
				if (recordCount < skipCount + count)
				{
					if (src->stat() == STATUS_IO_ERROR)
					{
						int n = moveVaileRecord(src);
						if (n)
							skipCount = recordCount - n - count;
						else
							break;
					}
					else
						break;
				}
				moveNextRecord(src, keyNum);

				skipCount++;
				if (src->stat() == STATUS_SUCCESS)
					break;

			}
			if (src->stat())
				break;
		}
		dest->clearBuffer();
		if (turbo)
		{
			if (dest->m_buflen + offset < src->datalen())
				return STATUS_CANT_ALLOC_MEMORY;
			if (offset)
				memset(dest->fieldPtr(0), 0, offset);
			memcpy((char*)dest->fieldPtr(0) + offset, src->fieldPtr(0), src->datalen());
		}
		else
			copyEachFiledData(dest, src, fci);

		if (repData)
		{
			dest->m_datalen = src->m_datalen;
			dest->tdap(TD_REC_INSERT);
		}
		else
			ins_rows += dest->insert();
		if (dest->stat() == STATUS_INVALID_VALLEN)
			skipCount++;
		else if (dest->stat() == STATUS_DUPPLICATE_KEYVALUE)
			skipCount++;
		else if (dest->stat() != STATUS_SUCCESS)
			return dest->stat();
		else
			count++;
		bool cancel = false;
		onCopyDataInternal(dest, recordCount, count, cancel);
		if (cancel) return -1;

		moveNextRecord(src, keyNum);
	}
	if ((skipCount) && (maxSkip == -1))
	{
		bool cancel = false;
		onCopyDataInternal(dest, -1, count, cancel);
		if (cancel) return -1;

	}

	if (src->stat() == 9) return 0;
	return src->stat();
}
#pragma warn .8004

void database::doConvertTable(short TableIndex, bool Turbo, const _TCHAR* OwnerName)
{
	table* src;
	table* dest;
	_TCHAR szTempPath[MAX_PATH];
	_TCHAR buf[MAX_PATH];

	short ret;
	if (m_impl->dbDef->tableDefs(TABLE_NUM_TMP) == NULL)
	{
		m_stat = STATUS_NODEF_FOR_CONVERT;
		return;
	}

	tabledef* TableDef = m_impl->dbDef->tableDefs(TABLE_NUM_TMP);
	TableDef->fieldDefs = dbdef::getFieldDef(TableDef);
	TableDef->keyDefs = dbdef::getKeyDef(TableDef);

	src = openTable(TABLE_NUM_TMP, TD_OPEN_EXCLUSIVE, false, OwnerName);
	if (!src)
		return;

	TableDef = m_impl->dbDef->tableDefs(TableIndex);
	short len = TableDef->maxRecordLen;

	TableDef->preAlloc = (ushort_td)(src->recordCount() / TableDef->pageSize / len);
	TableDef->flags.bit2 = true;

	_tcscpy(szTempPath, getTableUri(buf, TableIndex));

	_tcscat(szTempPath, _T("_conv_dest.tmp"));

	createTable(TableIndex, szTempPath);
	dest = openTable(TableIndex, TD_OPEN_EXCLUSIVE, true, NULL, szTempPath);
	if (!dest)
	{
		src->release();
		return;
	}
	beginTrn();
	ret = 0;
	dest->setNoUpdateTimeStamp(true);
	if (src->recordCount(false)) // estimate
		ret = copyTableData(dest, src, Turbo);
	if (ret == 0)
		endTrn();
	else
		abortTrn();
	dest->release();
	src->release();


	if (ret == 0)
	{
		_TCHAR tmp[MAX_PATH];
		_tcscpy(tmp, getTableUri(buf, TableIndex));
		if (isUseTransactd())
			swapTablename(szTempPath, tmp);
		else
		{
			_TCHAR* pireod = _tcsrchr(tmp, '.');
			if (pireod)
				* pireod = 0x00;
			_tcscat(tmp, _T("_conv_src.tmp"));
			rename(getTableUri(buf, TableIndex), tmp);
			if (m_stat)
				return;
			rename(szTempPath, getTableUri(buf, TableIndex));
			if (m_stat)
			{
				rename(tmp, getTableUri(buf, TableIndex));
				return;
			}
			nsdatabase::dropTable(tmp);
		}
	}
	else
		m_stat = ret;

}

void database::convertTable(short tableIndex, bool turbo, const _TCHAR* ownername)
{
	doConvertTable(tableIndex, turbo, ownername);
}

bool database::existsTableFile(short TableIndex, const _TCHAR* OwnerName)
{

	if (TableIndex == TABLE_NUM_TMP)
	{
		m_impl->dbDef->tableDefs(TABLE_NUM_TMP)->fieldDefs =
			dbdef::getFieldDef(m_impl->dbDef->tableDefs(TABLE_NUM_TMP));
		m_impl->dbDef->tableDefs(TABLE_NUM_TMP)->keyDefs = dbdef::getKeyDef(m_impl->dbDef->tableDefs(512));
	}
	table* bao = openTable(TableIndex, TD_OPEN_READONLY, false, OwnerName);
	bool ret = false;
	if (m_stat == STATUS_TABLE_NOTOPEN)
		ret = false;
	else if (m_stat == STATUS_INVALID_OWNERNAME)
		ret =  true;
	else if (m_stat == STATUS_SUCCESS)
		ret = true;
	if (bao)
		bao->release();
	m_stat = 0;
	return ret;
}


}// namespace client
}// namespace btrv
}// namespace protocol
}// namespace db
}// namespace bzs
