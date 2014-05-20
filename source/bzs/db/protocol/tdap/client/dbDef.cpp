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

#include "dbDef.h"
#include "database.h"
#include <bzs/rtl/strtrim.h>
#include <limits.h>
#include <stdio.h>

#include "fileDDF.h"
#include "fieldDDF.h"
#include "indexDDF.h"

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

#define FORMAT_VERSON_BTRV_DEF     		0
#define FORMAT_VERSON_NAME_WIDE_START 	1
#define FORMAT_VERSON_CURRENT   		1

static const _TUCHAR BDFFORMAT_VERSION[] = _T("2.000.00");
static const _TUCHAR ow0[11] = {46, 46, 83, 67, 46, 46, 46, 46, 93, 4, 0};
static const _TUCHAR ow1[11] = {46, 46, 83, 67, 46, 46, 46, 46, 66, 5, 0};
static const _TUCHAR ow2[11] = {46, 46, 83, 67, 46, 46, 46, 46, 212, 5, 0};
using namespace bzs::rtl;

class ownerNameSetter : public nstable
{
   void onReadAfter(){};
   void writeRecordData(){};
public:
   ownerNameSetter(nsdatabase *pbe):nstable(pbe){};
   using nstable::open;
};

static const int BDFMAXBUFFER = 32384;

struct dbdimple
{
	char keybuf[128];
	tabledef* bdf;
	int bdfLen;
	bool noWriteMode;
	short deftype;
	_TCHAR userName[20];
	tabledef* tableDefs[TABLE_NUM_TMP+1];
	_TCHAR version[9];
	ushort_td maxid;
	short tableCount;
	void* relateData;
	short openMode;

	dbdimple() : bdf(NULL), bdfLen(BDFMAXBUFFER), noWriteMode(false), tableCount(0),
		relateData(NULL), openMode(1), maxid(0)
	{
		memset(tableDefs, 0, (TABLE_NUM_TMP+1)*sizeof(tabledef*));
		_tcscpy(version, (const _TCHAR*)BDFFORMAT_VERSION);
	}
};

dbdef::dbdef(nsdatabase *pbe, short DefType) : nstable(pbe)
{
	m_impl = new dbdimple();
	m_impl->deftype = DefType;
	m_keybuflen = 128;
	m_keybuf = &m_impl->keybuf[0];
	setShared();
 
}

dbdef::~dbdef() 
{
	if (m_impl->bdf)
		free(m_impl->bdf);

	for (int i = 0; i <= m_impl->tableCount; i++)
	{
		if (m_impl->tableDefs[i] && (m_impl->tableDefs[i] != (tabledef*)-1) &&
			(m_impl->tableDefs[i] != m_impl->bdf))
			free(m_impl->tableDefs[i]);
	}
	if (m_impl->tableDefs[TABLE_NUM_TMP])
		free(m_impl->tableDefs[TABLE_NUM_TMP]);
	if (m_impl->relateData)
		free(m_impl->relateData);
	delete m_impl;
}

void dbdef::setDefType(short defType) {m_impl->deftype = defType;}

short dbdef::tableCount() const {return m_impl->tableCount;}

void* dbdef::relateData() const {return m_impl->relateData;}

short dbdef::openMode() const {return m_impl->openMode;}

int dbdef::version() const {return m_impl->version[7] - '0';};

void dbdef::setVersion(int v) {m_impl->version[7] = (char)v;}

void* dbdef::allocRelateData(int size)
{
	if (m_impl->relateData)
		free(m_impl->relateData);
	m_impl->relateData = malloc(size);
	return m_impl->relateData;
}
keylen_td dbdef::writeKeyData()
{
	return 2;
}
void dbdef::moveById(short id)
{
	while (1)
	{
		m_pdata = m_impl->bdf;
		m_buflen = m_impl->bdfLen;
		memcpy(m_keybuf, &id, 2);
		seek();
		if (m_stat == STATUS_BUFFERTOOSMALL)
		{
			if (!resizeReadBuf())
				return;
		}
		else
			break;
	}

}

bool dbdef::resizeReadBuf(void)
{
	m_impl->bdf = (tabledef*) realloc(m_impl->bdf, m_impl->bdfLen + BDFMAXBUFFER);
	if (m_impl->bdf == NULL)
	{
		m_stat = STATUS_CANT_ALLOC_MEMORY;
		return false;
	}
	m_impl->bdfLen += BDFMAXBUFFER;
	return true;
}

keydef* dbdef::getKeyDef(tabledef* p)
{
	// keydefPos = size of tabledef { size of fielddef x number
	return (keydef*)((char*) p +sizeof(tabledef) + (p->fieldCount*sizeof(fielddef)));
}

fielddef* dbdef::getFieldDef(tabledef* p)
{

	return (fielddef*)((char*) p +sizeof(tabledef));
}

void dbdef::setRecordLen(short TableIndex)
{
	tabledef* td = tableDefs(TableIndex);
	td->maxRecordLen = getRecordLen(TableIndex);

	// If valible length then specifing fixed length.
	if ((td->fixedRecordLen == 0) ||
		(td->flags.bit0 == false))
		td->fixedRecordLen = td->maxRecordLen;

}

void dbdef::setCodePage(tabledef* td)
{
	if (td->charsetIndex == 0)
		td->charsetIndex = mysql::charsetIndex(GetACP());

	for (short i = 0; i < td->fieldCount; i++)
	{
		fielddef& fd = td->fieldDefs[i];
		if (fd.charsetIndex() == 0)
			fd.setCharsetIndex(td->charsetIndex);
		fd.setSchemaCodePage(td->schemaCodePage);
	}
}

void dbdef::updateTableDef(short TableIndex, bool forPsqlDdf)
{
	m_stat = STATUS_SUCCESS;
	tabledef* td = tableDefs(TableIndex);
	short i, j, ret, Fnum;
	uchar_td type;

	td->optionFlags.bitA = false; // reset valiable type

	for (i=0; i <td->fieldCount; ++i)
	{
		ret = fieldNumByName(TableIndex, td->fieldDefs[i].name());
		if ((ret != -1) && (ret != i))
		{
			m_stat = STATUS_DUPLICATE_FIELDNAME;
			return;
		}
		// Check field length.
		type = td->fieldDefs[i].type;

		//reset update indicator
		td->fieldDefs[i].enableFlags.bitE = false;

		ret = validLen(type, td->fieldDefs[i].len);
		if (!ret)
		{
			m_stat = STATUS_INVALID_FIELDLENGTH;
			return;
		}
		// Note or Lvar type must be the last of fields.
		if ((type == ft_note) || (type == ft_lvar))
		{
			if (i != td->fieldCount - 1) {
				m_stat = STATUS_LVAR_NOTE_NOT_LAST;
				return;
			}
		}

		bool flag = (td->flags.bit0 == true)
							&& (i == (short)(td->fieldCount - 1));
		if (flag &&
				(type != ft_myvarbinary) &&
				(type != ft_mywvarbinary) &&
				(type != ft_note) &&
				(type != ft_lvar))
		{
			m_stat = STATUS_INVALID_VARIABLETABLE;
			return;
		}
		if ((type == ft_myvarchar) || (type == ft_mywvarchar)
			|| (type == ft_myvarbinary) || (type == ft_mywvarbinary)
				|| (type == ft_myfixedbinary))
			td->optionFlags.bitA = true;
		if ((type == ft_myblob) || (type == ft_mytext))
			td->optionFlags.bitB = true;

	}
  

	// Check invalid key type
	for (i = 0; i < td->keyCount; i++)
	{
		for (j = 0; j < td->keyDefs[i].segmentCount; j++)
		{
			Fnum = td->keyDefs[i].segments[j].fieldNum;
			ret = isPassKey(td->fieldDefs[Fnum].type);
			if (!ret)
			{
				m_stat = STATUS_INVALID_KEYTYPE;
				return;
			}
		}
	}


	// Chack duplicate table name.
	for (i = 1; i < m_impl->tableCount; i++)
	{
		if ((tableDefs(i)) && (i != TableIndex))
		{
			if (strcmp(tableDefs(i)->tableNameA(), td->tableNameA()) == 0)
			{
				m_stat = STATUS_DUPPLICATE_KEYVALUE;
				return;
			}
		}
	}
	setCodePage(td);

	setRecordLen(TableIndex);
	td = tableDefs(TableIndex);
	if (m_impl->noWriteMode)
	{
		m_stat = STATUS_ACCESS_DENIED;
		return;
	}
	if (m_impl->deftype == TYPE_SCHEMA_DDF)
		saveDDF(TableIndex, 3, forPsqlDdf);
	else
	{
		moveById(td->id);
		if (m_stat == STATUS_SUCCESS)
		{
			m_pdata = td;
			m_buflen = totalDefLength(TableIndex);
			cacheFieldPos(td);
			td->formatVersion = FORMAT_VERSON_CURRENT;
			update();
			m_pdata = m_impl->bdf;
			m_buflen = m_impl->bdfLen;
		}
	}

}

void dbdef::deleteTable(short TableIndex)
{
	m_stat = STATUS_SUCCESS;
	if (m_impl->noWriteMode)
	{
		free(tableDefs(TableIndex));
		m_impl->tableDefs[TableIndex] = NULL;
		return;
	}

	if (m_impl->deftype == TYPE_SCHEMA_DDF)
		saveDDF(TableIndex, 4);
	else
	{
		moveById(tableDefs(TableIndex)->id);
		if (m_stat == STATUS_SUCCESS)
		{
			m_pdata = tableDefs(TableIndex);
			m_buflen = totalDefLength(TableIndex);
			del();
			m_pdata = m_impl->bdf;
			m_buflen = m_impl->bdfLen;

		}
	}
	if (m_stat == STATUS_SUCCESS)
	{
		free(tableDefs(TableIndex));
		m_impl->tableDefs[TableIndex] = NULL;
	}

}

void dbdef::renumberFieldNum(short TableIndex, short Index, short op)
{
	int i, j;
	keydef* KeyDef;

	for (i = 0; i < tableDefs(TableIndex)->keyCount; i++)
	{
		KeyDef = &(tableDefs(TableIndex)->keyDefs[i]);

		for (j = 0; j < KeyDef->segmentCount; j++)
		{
			switch (op)
			{
			case 2:
				if (KeyDef->segments[j].fieldNum >= Index)
					KeyDef->segments[j].fieldNum++;
				break;
			case 4:
				if (KeyDef->segments[j].fieldNum > Index)
					KeyDef->segments[j].fieldNum--;
				break;
			}
		}
	}
}

bool dbdef::isUsedField(short TableIndex, short DeleteIndex)
{
	int i, j;
	keydef* KeyDef;

	for (i = 0; i < tableDefs(TableIndex)->keyCount; i++)
	{
		KeyDef = &(tableDefs(TableIndex)->keyDefs[i]);
		for (j = 0; j < KeyDef->segmentCount; j++)
		{
			if (KeyDef->segments[j].fieldNum == DeleteIndex)
				return true;
		}
	}
	return false;
}

void dbdef::deleteField(short TableIndex, short DeleteIndex)
{
	m_stat = STATUS_SUCCESS;
	if (isUsedField(TableIndex, DeleteIndex) == true) {
		m_stat = STATUS_USE_KEYFIELD;
		return;
	}
	renumberFieldNum(TableIndex, DeleteIndex, 4);
	tabledef* td = tableDefs(TableIndex);
	if ((DeleteIndex == td->fieldCount - 1) && (td->keyCount == 0))
	{
	}
	else
	{
		memmove(&td->fieldDefs[DeleteIndex], &td->fieldDefs[DeleteIndex + 1],
			totalDefLength(TableIndex) + (char*)td - (char*)&(td->fieldDefs[DeleteIndex + 1]));
	}
	td->fieldCount--;
	td->keyDefs = getKeyDef(tableDefs(TableIndex));
	updateTableDef(TableIndex);

}

void dbdef::deleteKey(short TableIndex, short DeleteIndex)
{
	m_stat = STATUS_SUCCESS;
	tabledef* td = tableDefs(TableIndex);
	if (DeleteIndex != td->keyCount - 1)
	{
		memmove(&td->keyDefs[DeleteIndex], &td->keyDefs[DeleteIndex + 1],
			totalDefLength(TableIndex) + (char*)td - (char*)&(td->keyDefs[DeleteIndex + 1]));
	}
	td->keyCount--;
	if ((td->primaryKeyNum == DeleteIndex) || (td->primaryKeyNum > td->keyCount - 1))
		td->primaryKeyNum = -1;
	if ((td->parentKeyNum == DeleteIndex) || (td->parentKeyNum > td->keyCount - 1))
		td->parentKeyNum = -1;
	if ((td->replicaKeyNum == DeleteIndex) || (td->replicaKeyNum > td->keyCount - 1))
		td->replicaKeyNum = -1;

	updateTableDef(TableIndex);
}

void dbdef::insertTable(tabledef* TableDef)
{
	m_stat = STATUS_SUCCESS;
	if (TableDef->id > TABLE_NUM_TMP)
	{
		m_stat = STATUS_TOO_MANY_TABLES;
		return;
	}
	if (tableDefs(TableDef->id) != NULL)
	{
		m_stat = STATUS_DUPPLICATE_KEYVALUE;
		return;
	}

	if (tableNumByName(TableDef->tableName()) != -1)
	{
		m_stat = STATUS_DUPPLICATE_KEYVALUE;
		return;
	}
	if (TableDef->fieldCount > 512)
	{
		m_stat = STATUS_TOO_MANY_FIELDS;
		return;
	}
	m_impl->tableDefs[TableDef->id] = (tabledef*)malloc(USHRT_MAX /* sizeof(tabledef) */);
	if (m_impl->tableDefs[TableDef->id] == NULL)
	{
		m_stat = STATUS_CANT_ALLOC_MEMORY;
		return;
	}
	if ((TableDef->ddfid == 0) && (m_impl->deftype == TYPE_SCHEMA_DDF))
		TableDef->ddfid = getDDFNewTableIndex();
	memcpy(m_impl->tableDefs[TableDef->id], TableDef, sizeof(tabledef));
	if (m_impl->noWriteMode)
	{
		if (m_impl->tableCount < TableDef->id)
			m_impl->tableCount = TableDef->id;
		return;
	}
	TableDef->formatVersion = FORMAT_VERSON_CURRENT;
	if (m_impl->deftype == TYPE_SCHEMA_DDF)
		saveDDF(TableDef->id, 2);
	else
	{
		memcpy(m_impl->bdf, TableDef, sizeof(tabledef));
		m_pdata = m_impl->bdf;

		memcpy(m_keybuf, &TableDef->id, 2);
		m_buflen = sizeof(tabledef);
		insert();
		m_pdata = m_impl->bdf;
		m_buflen = m_impl->bdfLen;
	}
	if (m_stat != 0)
	{
		free(m_impl->tableDefs[TableDef->id]);
		m_impl->tableDefs[TableDef->id] = NULL;
	}
	else
	{
		if (m_impl->tableCount < TableDef->id)
			m_impl->tableCount = TableDef->id;
	}
}

bool dbdef::resizeAt(short TableIndex, bool key)
{
	uint_td addsize;

	if (key == true)
		addsize = sizeof(keydef);
	else
		addsize = sizeof(fielddef);

	uint_td size = totalDefLength(TableIndex) + addsize;
	tabledef* def = m_impl->tableDefs[TableIndex];
	void* p = malloc(size);
	if (p)
	{
		memcpy(p, def, totalDefLength(TableIndex));
		free(def);
		m_impl->tableDefs[TableIndex] = def = (tabledef*)p;

	}
	else
	{
		m_stat = STATUS_CANT_ALLOC_MEMORY;
		return false;
	}
	// init for resize
	def->fieldDefs = getFieldDef(def);
	def->keyDefs = getKeyDef(def);
	return true;
}

keydef* dbdef::insertKey(short TableIndex, short InsertIndex)
{

	if (resizeAt(TableIndex, true) == false)
		return NULL;


	if (InsertIndex < tableDefs(TableIndex)->keyCount)
	{
		memmove(&tableDefs(TableIndex)->keyDefs[InsertIndex + 1],
			&tableDefs(TableIndex)->keyDefs[InsertIndex],
			totalDefLength(TableIndex) + (char*)tableDefs(TableIndex) -
			(char*)&(tableDefs(TableIndex)->keyDefs[InsertIndex]));
	}
	tableDefs(TableIndex)->keyCount++;
	memset(&(tableDefs(TableIndex)->keyDefs[InsertIndex]), 0, sizeof(keydef));

	if ((!m_impl->noWriteMode) && (m_impl->deftype != TYPE_SCHEMA_DDF))
		updateTableDef(TableIndex);
	else
		setRecordLen(TableIndex);
	return &(tableDefs(TableIndex)->keyDefs[InsertIndex]);
}

fielddef* dbdef::insertField(short TableIndex, short InsertIndex)
{
	if (resizeAt(TableIndex, false) == false)
		return NULL;

	tabledef* td = tableDefs(TableIndex);
	if ((InsertIndex < td->fieldCount) || (td->keyCount > 0))
	{

		memmove(&(td->fieldDefs[InsertIndex + 1]), &(td->fieldDefs[InsertIndex]),
			totalDefLength(TableIndex) + (char*)td - (char*)&(td->fieldDefs[InsertIndex]));
	}
	td->fieldCount++;
	td->keyDefs = getKeyDef(td);
	renumberFieldNum(TableIndex, InsertIndex, 2);
	memset(&(td->fieldDefs[InsertIndex]), 0, sizeof(fielddef));
	setRecordLen(TableIndex);
	fielddef* fd = &(td->fieldDefs[InsertIndex]);
	fd->setCharsetIndex(td->charsetIndex);
	fd->setSchemaCodePage(td->schemaCodePage);
	return fd;
}

int dbdef::totalDefLength(short TableIndex)
{
	tabledef* td = tableDefs(TableIndex);
	int len = (int)(sizeof(tabledef) + (sizeof(fielddef) * td->fieldCount) +
		(sizeof(keydef) * td->keyCount));
	if (isUseTransactd())
		td->varSize = len - 4;
	return len;
}

inline fielddef_t_my& dbdef::convert(fielddef_t_my& fd_my, const fielddef_t_pv& fd_pv)
{
	 memset(&fd_my, 0, sizeof(fd_my));
	 strcpy(fd_my.m_name, fd_pv.m_name);
	 void* dest = (char*)&fd_my + sizeof(fd_my.m_name);
	 const void* src = (char*)&fd_pv + sizeof(fd_pv.m_name);
	 size_t size = sizeof(fd_my) - sizeof(fd_my.m_name);
	 memcpy(dest, src, size);
	 return fd_my;
}

inline int fixVariableLenBug(bool isUseTransactd, tabledef* src, size_t size)
{
	if (isUseTransactd)
	{// A Transactd server format changed to nosupport FIXED_PLUS_VARIABLELEN
		if (src->pageSize+4 == size)
		{ // This is a chagned server
			if (src->preAlloc && ((src->preAlloc % 512 == 0) || src->fieldCount > 255
				|| src->keyCount > 127)
				|| src->fieldCount == 0)
			{
				//memmove(((char*)src)+4, ((char*)src)+6, src->pageSize-2);
				memmove(((char*)src)+2, ((char*)src)+4, src->pageSize);
				size -= 2;
			}
		}
	}
	return size;
}

size_t getNewVersionSize(tabledef* src)
{
	return src->fieldCount * sizeof(fielddef) + sizeof(tabledef)
		   + src->keyCount * sizeof(keydef)+1;

}

void dbdef::tableDefCopy(tabledef* dest, tabledef* src, size_t size)
{

	if (src->formatVersion == FORMAT_VERSON_BTRV_DEF)
	{
		size_t len = 0;
		memcpy(dest, src, sizeof(tabledef));
		len += sizeof(tabledef);
		fielddef_t_my* fd = (fielddef_t_my*)dbdef::getFieldDef(dest);
		fielddef_t_pv* src_fd = (fielddef_t_pv*)dbdef::getFieldDef(src);
		for (int i=0;i<dest->fieldCount;++i)
		{
			convert(*fd , *src_fd);
			len += sizeof(fielddef_t_pv);
			++fd;
			++src_fd;
		}
		memcpy(fd, src_fd, size - len);
		dest->formatVersion = FORMAT_VERSON_CURRENT;
	}
	else
		memcpy(dest, src, size);

}

tabledef** dbdef::tableDefPtr(int index)
{
	tableDefs(index);
	return &m_impl->tableDefs[index];
}

#pragma warn -8004
tabledef* dbdef::tableDefs(int index)
{
	if (index > TABLE_NUM_TMP)
		return NULL;
	tabledef* def = m_impl->tableDefs[index];

	if (index == TABLE_NUM_TMP)
		return def;
	if (m_impl->tableCount < index)
		return NULL;
	if (def == NULL)
	{
		if (m_impl->deftype == TYPE_SCHEMA_DDF)
			return NULL;
		while (1)
		{
			m_pdata = m_impl->bdf;
			m_buflen = m_impl->bdfLen;
			m_impl->bdf->id = (short)index;
			memcpy(m_keybuf, &m_impl->bdf->id, 2);
			seek();
			if (m_stat == STATUS_BUFFERTOOSMALL)
			{
				if (!resizeReadBuf())
					return NULL;
			}
			else
				break;
		}
		if (m_stat && (m_stat != STATUS_NOT_FOUND_TI))
		{
			return NULL;
		}
		if (m_stat)
		{
			def = (tabledef*)-1;
			m_stat = 0;
			return NULL;
		}
		m_datalen = fixVariableLenBug(isUseTransactd(), (tabledef*)m_pdata, m_datalen);

		m_impl->tableDefs[index] = def = (tabledef*)malloc(getNewVersionSize((tabledef*)m_pdata));

		if (def == NULL)
		{
			m_stat = STATUS_CANT_ALLOC_MEMORY;
			return NULL;
		}
		tableDefCopy(def, (tabledef*)m_pdata, m_datalen);
		def->fieldDefs = getFieldDef(def);
		def->keyDefs = getKeyDef(def);
		def->autoIncExSpace = ((database*)nsdb())->defaultAutoIncSpace();

		setCodePage(def);


	}
	else if (def == (tabledef*)-1)
		return NULL;

	return def;
}
#pragma warn .8004

void dbdef::doOpen(const _TCHAR* FullPath, char_td mode, const _TCHAR* OnerName)
{
	m_impl->noWriteMode = true;

	if (m_impl->deftype == TYPE_SCHEMA_DDF)
	{

		openDdf(((database*)nsdb())->rootDir(), mode, OnerName); // DDF
		if (mode != TD_OPEN_READONLY)
			m_impl->noWriteMode = false;
		return;
	}

	//version check
	m_impl->version[7] = '0';
	nstable::doOpen(FullPath, mode, m_impl->version);
	if (m_stat == STATUS_INVALID_OWNERNAME)
	{
		while (m_stat == STATUS_INVALID_OWNERNAME)
		{
			m_impl->version[7]++;
			nstable::doOpen(FullPath, mode, m_impl->version);
			if (m_impl->version[7] > '9')
				return;
		}
	}
	if (m_stat)
		return;

	if (m_impl->bdf == NULL)
		m_impl->bdf = (tabledef*)malloc(m_impl->bdfLen);
	m_pdata = m_impl->bdf;
	memcpy(m_keybuf, &m_impl->bdf->id, 2);
	m_buflen = m_impl->bdfLen;
	m_keynum = 0;

	seekLast();
	if (m_stat == STATUS_SUCCESS)
		m_impl->tableCount = m_impl->bdf->id;
	if (m_stat == STATUS_EOF)
		m_stat = STATUS_SUCCESS;
	if (mode != TD_OPEN_READONLY)
		m_impl->noWriteMode = false;
	m_impl->openMode = mode;
}

void dbdef::doClose()
{
	nstable::doClose();
	if (m_impl->bdf)
	{
		free(m_impl->bdf);
		m_impl->bdf = NULL;
	}
	m_impl->openMode = 1;
}

void dbdef::create(const _TCHAR* fullpath)
{

	if (m_impl->deftype == TYPE_SCHEMA_DDF)
	{
		createDDF(fullpath);
		return;
	}
	fileSpec *fs;

	fs = (fileSpec*)malloc(512);
	memset(fs, 512, 0x00);
	fs->recLen = sizeof(tabledef);
	fs->pageSize = 4096;
	fs->indexCount = 1;
	fs->fileFlag.all = 1; // valiable length
	fs->preAlloc = 10;
	fs->keySpecs[0].keyPos = 1;        // id
	fs->keySpecs[0].keyLen = 2;        // short
	fs->keySpecs[0].keyFlag.all = 258; // changeable  and extended key type
	fs->keySpecs[0].keyType = 1;       // Integer
	nsdb()->createTable(fs, 412, fullpath, 0);
	free(fs);
	if (nsdb()->stat() == 0)
	{ // set owner name
		ownerNameSetter* bao = new ownerNameSetter(nsdb());
		bao->open(fullpath);
		bao->setOwnerName((const _TCHAR*) BDFFORMAT_VERSION);
		bao->release();
	}
	m_stat = nsdb()->stat();
}

void dbdef::drop()
{
	nsdb()->dropTable(uri());
	m_stat = nsdb()->stat();
}

ushort_td dbdef::getRecordLen(short TableIndex)
{
	ushort_td ret = 0;
	short i;
	tabledef* td = tableDefs(TableIndex);
	for (i = 0; i < td->fieldCount; i++)
		ret += td->fieldDefs[i].len	+ td->fieldDefs[i].varLenBytes();
	return ret;
}

ushort_td dbdef::getFieldPosition(tabledef *TableDef, short FieldNum)
{
	short i;
	ushort_td pos = 1;
	for (i = 0; i < FieldNum; i++)
		pos += TableDef->fieldDefs[i].len;
	return pos;
}

void dbdef::getFileSpec(fileSpec* fs, short TableIndex)
{
	keySpec* ks;
	keydef* KeyDef;
	short i, j, k = 0;
	short FieldNum;
	tabledef *TableDef = tableDefs(TableIndex);
	fs->recLen = TableDef->fixedRecordLen;
	fs->pageSize = TableDef->pageSize;
	fs->indexCount = TableDef->keyCount;
	fs->recCount = 0;
	fs->fileFlag.all = TableDef->flags.all;
	fs->reserve1[0] = 0;
	fs->reserve1[1] = 0;
	fs->preAlloc = TableDef->preAlloc;

	for (i = 0; i < TableDef->keyCount; i++)
	{
		KeyDef = &(TableDef->keyDefs[i]);
		for (j = 0; j < KeyDef->segmentCount; j++)
		{
			FieldNum = KeyDef->segments[j].fieldNum;
			ks = &(fs->keySpecs[k]);
			ks->keyPos = getFieldPosition(TableDef, FieldNum);
			ks->keyLen = TableDef->fieldDefs[FieldNum].len;
			ks->keyFlag.all = KeyDef->segments[j].flags.all;
			ks->keyCount = 0;
			ks->keyType = TableDef->fieldDefs[FieldNum].type;

			if ((ks->keyType == ft_autoinc) && (KeyDef->segmentCount > 1))
				ks->keyType = 1;
			if (ks->keyFlag.bit3 == true)
				ks->nullValue = TableDef->fieldDefs[FieldNum].nullValue;
			else
				ks->nullValue = 0;
			ks->reserve2[0] = 0;
			ks->reserve2[1] = 0;
			if (fs->fileFlag.bitA == true)
				ks->keyNo = KeyDef->keyNumber;
			else
				ks->keyNo = 0; ;
			ks->acsNo = 0;
			k++;
		}
	}
}

void dbdef::renumberTable(short OldIndex, short NewIndex)
{
	if (NewIndex > TABLE_NUM_TMP)
	{
		m_stat = STATUS_TOO_MANY_TABLES;
		return;
	}
	if (m_impl->noWriteMode)
	{
		tableDefs(OldIndex)->id = NewIndex;
		m_impl->tableDefs[NewIndex] = tableDefs(OldIndex);
		m_impl->tableDefs[OldIndex] = NULL;
		if (NewIndex > m_impl->tableCount)
			m_impl->tableCount = NewIndex;
		return;
	}
	moveById(tableDefs(OldIndex)->id);
	if (m_stat == STATUS_SUCCESS)
	{
		m_pdata = tableDefs(OldIndex);
		m_buflen = totalDefLength(OldIndex);
		tableDefs(OldIndex)->id = NewIndex;
		update();
		m_pdata = m_impl->bdf;
		m_buflen = m_impl->bdfLen;

		if (m_stat == STATUS_SUCCESS)
		{
			m_impl->tableDefs[NewIndex] = tableDefs(OldIndex);
			m_impl->tableDefs[OldIndex] = NULL;
			if (NewIndex > m_impl->tableCount)
				m_impl->tableCount = NewIndex;
		}
		else
			tableDefs(OldIndex)->id = OldIndex;
	}
}

void dbdef::cacheFieldPos(tabledef *TableDef)
{

	short i;
	for (i = 0; i < TableDef->fieldCount; i++)
		TableDef->fieldDefs[i].pos = (ushort_td)(getFieldPosition(TableDef, i) - 1);
}

short dbdef::fieldNumByViewNum(short TableIndex, short index)
{
	short i;
	tabledef *TableDef = tableDefs(TableIndex);
	for (i = 0; i < TableDef->fieldCount; i++)
	{
		if ((TableDef->fieldDefs[i].viewNum == index) && (TableDef->fieldDefs[i].enableFlags.bit0))
			return i;
	}
	return -1;
}

short dbdef::findKeynumByFieldNum(short TableIndex, short index)
{
	short i;
	tabledef *TableDef = tableDefs(TableIndex);

	for (i = 0; i < TableDef->keyCount; i++)
	{
		if (TableDef->keyDefs[i].segments[0].fieldNum == index)
			return i;
	}
	return -1;
}

short dbdef::tableNumByName(const _TCHAR* tableName)
{
	short i;
	char buf[74];

	for (i = 1; i <= m_impl->tableCount; i++)
	{
		if (tableDefs(i))
		{
			const char* p = tableDefs(i)->toChar(buf, tableName, 74);
			if (strcmp(tableDefs(i)->tableNameA(), p) == 0)
				return i;
		}
	}
	return -1;
}

short dbdef::fieldNumByName(short TableIndex, const _TCHAR* name)
{
	short i;
	char buf[74];
	const char* p = tableDefs(TableIndex)->toChar(buf, name, 74);
	for (i = 0; i < tableDefs(TableIndex)->fieldCount; i++)
	{
		if (strcmp(tableDefs(TableIndex)->fieldDefs[i].nameA(), p) == 0)
			return i;
	}
	return -1;
}

uint_td dbdef::fieldValidLength(eFieldQuery query, uchar_td FieldType)
{ // return MaxLen or MinLen or DefaultLen or Dec of field type

	uint_td maxlen;
	uint_td minlen;
	uint_td defaultlen;
	int dec = -1;
	switch (FieldType)
	{
	case ft_string:
	case ft_mychar: minlen = 1;
		maxlen = 255;
		defaultlen = 1;
		break;
	case ft_lstring:
	case ft_zstring: minlen = 2;
		maxlen = 255;
		defaultlen = 2;
		break;
	case ft_myvarchar:
	case ft_myvarbinary: minlen = 1;
		maxlen = 60000;
		defaultlen = 2;
		break;
	case ft_myblob:
	case ft_mytext: minlen = 9;
		maxlen = 12;
		defaultlen = 1;
		break;
	case ft_mywchar:
	case ft_wstring: minlen = 2;
		maxlen = 255;
		defaultlen = 2;
		break;
	case ft_wzstring: minlen = 3;
		maxlen = 255;
		defaultlen = 2;
		break;
	case ft_mywvarchar:
	case ft_mywvarbinary: minlen = 1;
		maxlen = 60000;
		defaultlen = 3;
		break;
	case ft_myfixedbinary:
		minlen = 256;
		maxlen = 60000;
		defaultlen = 1024;
		break;
	case ft_mydate: minlen = 3;
		maxlen = 3;
		defaultlen = 3;
		break;
	case ft_integer: minlen = 1;
		maxlen = 8;
		defaultlen = 2;
		break;
	case ft_bfloat:
	case ft_float: minlen = 4;
		maxlen = 8;
		defaultlen = 4;
		break;
	case ft_date:
	case ft_time: minlen = 4;
		maxlen = 4;
		defaultlen = 4;
		break;
	case ft_mytime: minlen = 3;
		maxlen = 6;
		defaultlen = 4;
		break;
	case ft_money:
	case ft_decimal: minlen = 1;
		maxlen = 10;
		defaultlen = 6;
		if (FieldType == 5)
			dec = 0;
		else
			dec = 2;
		break;
	case ft_logical: minlen = 1;
		maxlen = 2;
		defaultlen = 2;
		break;
	case ft_numericsts:
	case ft_numericsa:
	case ft_numeric: minlen = 1;
		maxlen = 15;
		defaultlen = 6;
		dec = 0;
		if (FieldType == 17)
			minlen = 2;
		break;

	case ft_note: minlen = 2;
		maxlen = 32761;
		defaultlen = 2;
		break;
	case ft_lvar: minlen = 5;
		maxlen = 32761;
		defaultlen = 5;
		break;
	case ft_uinteger: minlen = 1;
		maxlen = 8;
		defaultlen = 2;
		break;
	case ft_autoIncUnsigned:
	case ft_autoinc: minlen = 2;
		maxlen = 8;
		defaultlen = 4;
		break;
	case ft_bit: minlen = 1;
		maxlen = 1;
		defaultlen = 1;
		break;
	case ft_timestamp:
	case ft_currency: minlen = 8;
		maxlen = 8;
		defaultlen = 8;
		break;
	case ft_mytimestamp: minlen = 4;
		maxlen = 7;
		defaultlen = 7;
		break;
	case ft_mydatetime: minlen = 5;
		maxlen = 8;
		defaultlen = 5;
		break;

	case ft_nullindicator: minlen = 0;
		maxlen = 0;
		defaultlen = 0;
	}
	switch (query)
	{
	case eMinlen: return minlen;
	case eMaxlen: return maxlen;
	case eDefaultlen: return defaultlen;
	case eDecimals: return dec;
	}
	return 0;

}

bool dbdef::validLen(uchar_td FieldType, uint_td FieldLen)
{ // return valid length of field by field type.
	if ((FieldLen >= fieldValidLength(eMinlen, FieldType)) && (FieldLen <= fieldValidLength(eMaxlen,
		FieldType)))
	{
		if ((FieldType == ft_integer) || (FieldType == ft_uinteger))
		{
			if ((FieldLen == 1) || (FieldLen == 2) || (FieldLen == 4) || (FieldLen == 8))
				return true;
			else
				return false;

		}
		else if ((FieldType == ft_autoinc) || (FieldType == ft_autoIncUnsigned))
		{
			if ((FieldLen == 2) || (FieldLen == 4)|| (FieldLen == 8))
				return true;
			else
				return false;
		}
		else if ((FieldType == ft_float) || (FieldType == ft_bfloat))
		{
			if ((FieldLen == 4) || (FieldLen == 8))
				return true;
			else
				return false;
		}
		else
			return true;
	}
	return false;
}

bool dbdef::isPassKey(uchar_td FieldType)
{
	if (FieldType == ft_wstring)
		return true;
	if (FieldType == ft_wzstring)
		return true;
	if (FieldType == ft_myvarchar)
		return true;
	if (FieldType == ft_myvarbinary)
		return true;
	if (FieldType == ft_mywvarchar)
		return true;
	if (FieldType == ft_mywvarbinary)
		return true;
	if (FieldType == ft_mychar)
		return true;
	if (FieldType == ft_mywchar)
		return true;

	if (FieldType == ft_mytext)
		return true;
	if (FieldType == ft_myblob)
		return true;

	if (FieldType == ft_mydate)
		return true;
	if (FieldType == ft_mytime)
		return true;
	if (FieldType == ft_mydatetime)
		return true;
	if (FieldType == ft_myfixedbinary)
		return false;

	if (FieldType == ft_bit)
		return false;
	if (FieldType > ft_numericsts)
		return false;
	if (FieldType == ft_note)
		return false;
	if (FieldType == ft_lvar)
		return false;
	return true;
}

void dbdef::autoMakeSchema() {tdap(TD_AUTOMEKE_SCHEMA);}

void dbdef::createDDF(const _TCHAR* fullpath)
{
	_TCHAR dir[MAX_PATH];
	_TCHAR Path[MAX_PATH];
	short chOpen = 0;

	getDirURI(fullpath, dir);
	fileDDF* tb = new fileDDF(nsdb());
	fieldDDF* fd = new fieldDDF(nsdb());
	indexDDF* id = new indexDDF(nsdb());
	if ((tb) && (fd) && (id))
	{
		_tcscpy(Path, dir);
		_tcscat(Path, PSEPARATOR _T("FILE.DDF"));
		tb->createTable(Path);
		chOpen += tb->stat();

		_tcscpy(Path, dir);
		_tcscat(Path, PSEPARATOR _T("FIELD.DDF"));
		fd->createTable(Path);
		chOpen += fd->stat();

		_tcscpy(Path, dir);
		_tcscat(Path, PSEPARATOR _T("INDEX.DDF"));
		id->createTable(Path);
		chOpen += id->stat();
		m_stat = chOpen;
	}
	if (tb)
		tb->release();
	if (fd)
		fd->release();
	if (id)
		id->release();

}

void dbdef::saveDDF(short TableIndex, short opration, bool forPsqlDdf)
{
	ushort_td chOpen = 0;
	short Mode = 0;
	ushort_td tbid = TableIndex;
	ushort_td fdid;
	ushort_td keyid;
	ushort_td segid;
	ushort_td pos;
	const _TUCHAR* own[3];

	m_stat = STATUS_SUCCESS;
	tabledef* TableDef;
	fielddef* FieldDef;
	keydef* KeyDef;

	fileDDF* tb = new fileDDF(nsdb());
	fieldDDF* fd = new fieldDDF(nsdb());
	indexDDF* id = new indexDDF(nsdb());

	if ((tb) && (fd) && (id))
	{
		if (m_impl->userName[0] != 0x00)
		{
			own[0] = (const _TUCHAR*)m_impl->userName;
			own[1] = (const _TUCHAR*)m_impl->userName;
			own[2] = (const _TUCHAR*)m_impl->userName;
		}
		else
		{
			own[0] = ow0;
			own[1] = ow1;
			own[2] = ow2;
		}
		tb->open(((database*)nsdb())->rootDir(), (char_td)Mode, (const _TCHAR*) own[0]);
		chOpen += tb->stat();
		fd->open(((database*)nsdb())->rootDir(), (char_td)Mode, (const _TCHAR*)own[1]);
		chOpen += fd->stat();
		id->open(((database*)nsdb())->rootDir(), (char_td)Mode, (const _TCHAR*)own[2]);
		chOpen += id->stat();
		if (chOpen)
		{
			tb->release();
			fd->release();
			id->release();
			m_stat = STATUS_INVALID_OWNERNAME;
			return;
		}

		//erase all records
		nsdb()->beginTrn(200);
		tb->setKeyNum(0);


		tb->setStat(0);
		fd->setStat(0);
		id->setStat(0);
		tb->setKeyNum(0);
		fd->setKeyNum(1);
		id->setKeyNum(0);
		if (tableDefs(tbid))
		{
			TableDef = tableDefs(tbid);
			tb->id = TableDef->ddfid;
			tb->seek();
			strcpy(tb->tablename, TableDef->tableNameA());
			strcpy(tb->filename, TableDef->fileNameA());
			if (forPsqlDdf)
				tb->flag = 0;//PSQL are reading flags from table files.
			else
				tb->flag = TableDef->flags.all;
			if (tb->stat() == STATUS_SUCCESS)
			{
				if (opration == 4)
					tb->del();
				else
					tb->update();
			}
			else if (tb->stat() == STATUS_NOT_FOUND_TI)
				tb->insert();

			if (tb->stat())
				goto JmpErr;

			while (fd->stat() == STATUS_SUCCESS)
			{
				fd->fileid = tb->id;
				fd->seek();
				if (fd->stat() == STATUS_SUCCESS)
					fd->del();
			}
			fd->setStat(0);
			if (opration != 4)
			{
				pos = 1;
				for (fdid = 0; fdid < TableDef->fieldCount; fdid++)
				{
					FieldDef = &TableDef->fieldDefs[fdid];
					if (FieldDef->ddfid == 0)
					FieldDef->ddfid = getDDFNewFieldIndex();
					fd->fieldid = FieldDef->ddfid;
					fd->fileid = tb->id;
					strcpy(fd->name, FieldDef->nameA());
					fd->type = FieldDef->type;
					if (forPsqlDdf && (fd->type == ft_logical))
						fd->type = ft_uinteger;
					fd->pos = (ushort_td)(pos - 1);
					fd->len = FieldDef->len;
					pos += FieldDef->len;
					fd->dec = FieldDef->decimals;
					fd->flag = 0;
					fd->insert();
					if (fd->stat())
					goto JmpErr;
				}
			}
			// delete keys.
			while (id->stat() == STATUS_SUCCESS)
			{
				id->fileid = tb->id;
				id->seek();
				if (id->stat() == STATUS_SUCCESS)
					id->del();
			}
			id->setStat(0);
			if (opration != 4)
			{
				for (keyid = 0; keyid < TableDef->keyCount; keyid++)
				{
					KeyDef = &TableDef->keyDefs[keyid];
					id->fileid = tb->id;
					id->keyid = keyid;
					for (segid = 0; segid < KeyDef->segmentCount; segid++)
					{
					id->fieldid = TableDef->fieldDefs[KeyDef->segments[segid].fieldNum].ddfid;
					id->segmentnum = segid;
					id->flag = KeyDef->segments[segid].flags.all;
					id->insert();
					if (id->stat())
					goto JmpErr;
					}
				}
			}
		}

	JmpErr:
		if ((tb->stat()) || (fd->stat()) || (id->stat()))
			nsdb()->abortTrn();
		else
			nsdb()->endTrn();
	}
	m_stat = (short)(tb->stat() + fd->stat() + id->stat());

	if (tb)
		tb->release();
	if (fd)
		fd->release();
	if (id)
		id->release();

}

void dbdef::openDdf(const _TCHAR* dir, short Mode, const _TCHAR* OwnerName)
{
	tabledef TableDef;
	fielddef* FieldDef;
	keydef* KeyDef;
	short tbid = 1;
	short FieldIndex;
	const _TCHAR* own[3];
	m_stat = 0;
	fileDDF* tb = new fileDDF(nsdb());
	fieldDDF* fd = new fieldDDF(nsdb());
	indexDDF* id = new indexDDF(nsdb());
	if ((tb) && (fd) && (id))
	{
		// set owner name
		if ((OwnerName != NULL) && (OwnerName[0] != 0x00))
		{
			own[0] = OwnerName;
			own[1] = OwnerName;
			own[2] = OwnerName;
			_tcscpy(m_impl->userName, OwnerName);
		}
		else
		{
			own[0] = (_TCHAR*)ow0;
			own[1] = (_TCHAR*)ow1;
			own[2] = (_TCHAR*)ow2;
			m_impl->userName[0] = 0x00;
		}

		tb->open(dir, (char_td)Mode, own[0]);
		if (!tb->stat())
		{
			fd->open(dir, (char_td)Mode, own[1]);
			if (!fd->stat())
			{
				id->open(dir, (char_td)Mode, own[2]);
				m_stat = id->stat();
			}
			else
				m_stat = fd->stat();
		}
		else
			m_stat = tb->stat();

		if (m_stat)
		{
			tb->release();
			fd->release();
			id->release();
			return;
		}

		tb->setKeyNum(0);
		tb->seekFirst();
		while (tb->stat() == 0)
		{ // Inser table
			memset(&TableDef, 0, sizeof(TableDef));
			TableDef.setTableNameA(strtrimA(tb->tablename));
			TableDef.setFileNameA(strtrimA(tb->filename));

			TableDef.ddfid = tb->id;
			TableDef.id = tbid;
			TableDef.keyCount = 0;
			TableDef.fieldCount = 0;
			TableDef.flags.all = tb->flag;
			TableDef.primaryKeyNum = -1;
			TableDef.parentKeyNum = -1;
			TableDef.replicaKeyNum = -1;
			insertTable(&TableDef);
			if (m_stat != 0)
				break;


			fd->setKeyNum(1);
			fd->fileid = tb->id;
			fd->seekGreater(true);
			while (fd->fileid == tb->id)
			{
				if (fd->stat() != 0)
					break;

				if (fd->type != ft_nullindicator)
				{
					for (FieldIndex = 0; FieldIndex < tableDefs(tbid)->fieldCount; FieldIndex++)
					{
						if (tableDefs(tbid)->fieldDefs[FieldIndex].pos > fd->pos)
							break;
					}
					FieldDef = insertField(tbid, FieldIndex);
					FieldDef->setNameA(strtrimA(fd->name));
					FieldDef->type = fd->type;
					FieldDef->len = fd->len;
					FieldDef->decimals = fd->dec;
					FieldDef->pos = fd->pos;
					FieldDef->ddfid = fd->fieldid;
					id->setKeyNum(1);
					id->fieldid = fd->fieldid;
					id->seek();
					while (id->fieldid == fd->fieldid)
					{
						if (id->stat() != 0)
							break;

						while (id->keyid >= tableDefs(tbid)->keyCount)
							insertKey(tbid, tableDefs(tbid)->keyCount);

						KeyDef = &(tableDefs(tbid)->keyDefs[id->keyid]);
						if (KeyDef->segmentCount < id->segmentnum + 1)
							KeyDef->segmentCount = (uchar_td)(id->segmentnum + 1);
						KeyDef->segments[id->segmentnum].fieldNum = (uchar_td)FieldIndex;
						KeyDef->segments[id->segmentnum].flags.all = id->flag;
						id->seekNext();
					}
				}
				/*else
				{
				}*/
				fd->seekNext();
			}

			tableDefs(tbid)->maxRecordLen = getRecordLen(tbid);

			tableDefs(tbid)->fixedRecordLen = tableDefs(tbid)->maxRecordLen;
			tableDefs(tbid)->parentKeyNum = -1;
			tableDefs(tbid)->replicaKeyNum = -1;
			tableDefs(tbid)->primaryKeyNum = -1;

			for (short i = tableDefs(tbid)->fieldCount - 1; i >= 0; i--)
			{
				if (tableDefs(tbid)->fieldDefs[i].type >= 227)
				{
					FieldDef = insertField(tbid, tableDefs(tbid)->fieldCount);
					*FieldDef = tableDefs(tbid)->fieldDefs[i];
					deleteField(tbid, i);
				}
			}


			for (short i = tableDefs(tbid)->fieldCount - 1; i >= 0; i--)
			{
				short pos = tableDefs(tbid)->fieldDefs[i].pos;
				short startPos = 0;
				if (i > 0)
					startPos = tableDefs(tbid)->fieldDefs[i - 1].pos +
					tableDefs(tbid)->fieldDefs[i - 1].len;
				if (tableDefs(tbid)->fieldDefs[i].len && (startPos != pos))
				{
					FieldDef = insertField(tbid, i);
					sprintf_s((char*)FieldDef->nameA(), FIELD_NAME_SIZE, "NIS%d", i + 1);

					FieldDef->type = ft_nullindicator;
					FieldDef->len = 1;
					FieldDef->decimals = 0;
					FieldDef->pos = pos - 1;
					FieldDef->ddfid = 0;
				}
			}

			tbid++;
			tb->seekNext();
		}
		m_stat = STATUS_SUCCESS;
	}
	else
		m_stat = STATUS_INVALID_OWNERNAME;

	if (tb)
		tb->release();
	if (fd)
		fd->release();
	if (id)
		id->release();
}

ushort_td dbdef::getDDFNewTableIndex()
{
	int i;
	int max_id = 0;
	for (i = 0; i <= m_impl->tableCount; i++)
	{
		if (tableDefs(i) != NULL)
		{
			if (tableDefs(i)->ddfid > max_id)
				max_id = tableDefs(i)->ddfid;
		}
	}
	max_id++;
	return (ushort_td)max_id;
}

ushort_td dbdef::getDDFNewFieldIndex()
{
	int i, j;
	if (m_impl->maxid == 0)
	{
		for (i = 0; i <= m_impl->tableCount; i++)
		{
			if (tableDefs(i) != NULL)
			{
				for (j = 0; j < tableDefs(i)->fieldCount; j++)
				{
					if (tableDefs(i)->fieldDefs[j].ddfid > m_impl->maxid)
					m_impl->maxid = tableDefs(i)->fieldDefs[j].ddfid;
				}
			}
		}
	}
	m_impl->maxid++;
	return (ushort_td)m_impl->maxid;
}

void setFieldsCharsetIndex(tabledef* def)
{
	if (def->charsetIndex == 0)
		def->charsetIndex = mysql::charsetIndex(GetACP());
	for (short i = 0; i < def->fieldCount; i++)
	{
		fielddef& fd = def->fieldDefs[i];
		if (fd.charsetIndex() == 0)
			fd.setCharsetIndex(def->charsetIndex);
	}
}

void dbdef::pushBackup(short TableIndex)
{
	int blen;

	blen = totalDefLength(TableIndex);
	if (!tableDefs(TABLE_NUM_TMP))
		m_impl->tableDefs[TABLE_NUM_TMP] = (tabledef*) malloc(blen);
	else
		m_impl->tableDefs[TABLE_NUM_TMP] = (tabledef*)realloc(tableDefs(TABLE_NUM_TMP), blen);
	if (!tableDefs(TABLE_NUM_TMP))
	{
		m_stat = STATUS_CANT_ALLOC_MEMORY;
		return;
	}

	setFieldsCharsetIndex(tableDefs(TableIndex));
	memcpy(m_impl->tableDefs[TABLE_NUM_TMP], tableDefs(TableIndex), blen);
}

bool dbdef::compAsBackup(short TableIndex)
{
	if (m_impl->tableCount < TableIndex)
		return false;

	int len = totalDefLength(TABLE_NUM_TMP);
	int len2 = totalDefLength(TableIndex);
	if (len != len2)
		return true;
	else if (memcmp(tableDefs(TABLE_NUM_TMP), tableDefs(TableIndex), len) != 0)
		return true;
	return false;
}

void dbdef::popBackup(short TableIndex)
{
	int len = totalDefLength(TABLE_NUM_TMP);
	m_impl->tableDefs[TableIndex] = (tabledef*)realloc(tableDefs(TableIndex), len);
	memcpy(tableDefs(TableIndex), tableDefs(TABLE_NUM_TMP), len);
	tableDefs(TableIndex)->fieldDefs = getFieldDef(tableDefs(TableIndex));
	tableDefs(TableIndex)->keyDefs = getKeyDef(tableDefs(TableIndex));

	updateTableDef(TableIndex);
}
void dbdef::reopen(char_td mode)
{
	close();
	open(uri(), mode, NULL);
}


}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs
