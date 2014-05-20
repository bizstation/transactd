#ifndef DATABASE_H
#define DATABASE_H
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



#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <vector>
#include <bzs/env/crosscompile.h>
#include "fieldAccess.h"
#include <bzs/db/IBlobBuffer.h>

class THD;
struct TABLE;

#ifndef MAX_KEYLEN
	#define MAX_KEYLEN 1023
#endif
namespace bzs
{
namespace db
{
namespace engine
{
namespace mysql
{

/*
	Please comment out the following, 
	when you emulate btrv variable length record of btrv with a server. 
	
	#define USE_BTRV_VARIABLE_LEN
*/


#define READ_RECORD_GETNEXT 1
#define READ_RECORD_GETPREV 2
#define READ_RECORD_STEPNEXT 3
#define READ_RECORD_STEPPREV 4

#define RF_FIXED_LEN				0
#define	RF_FIXED_PLUS_VALIABLE_LEN	1
#define	RF_VALIABLE_LEN				2
#define	RF_INCLUDE_NIS				4

#define TRN_RECORD_LOCK_SINGLE 0
#define TRN_RECORD_LOCK_MUILTI 1


/** bookmark size
 *  btreive API is MAX 4 byte
 */
#define REF_SIZE_MAX 4
class table;

extern bool g_safe_share_mode;

/** Control mysql table cahche
 */
class tableCacheCounter
{
	std::vector<int> m_tables;
	std::vector<int> m_counts;

	boost::mutex m_mutex;
	int getHash(const std::string& dbname, const std::string& tbname);
	size_t getCounterIndex(const std::string& dbname, const std::string& tbname);
public:
	tableCacheCounter();
	void addref(const std::string& dbname, const std::string& tbname);
	int count(const std::string& dbname, const std::string& tbname);
	void release(const std::string& dbname, const std::string& tbname);

};

class database : private boost::noncopyable
{
	friend class table;
	friend class smartDbReopen;
	std::string m_dbname;
	
	mutable THD* m_thd;
	short m_cid;
	int m_inTransaction;
	int m_inSnapshot;
	int m_stat;
	short m_trnType;
	
	std::vector<boost::shared_ptr<table> > m_tables;
	TABLE* doOpenTable(const std::string& name, short mode, const char* ownerName);
	
	void unUseTable(table* tb);
public:
	database(const char* name, short cid);
	~database();
	int stat(){return m_stat;};
	THD* thd()const{return m_thd;};
	void use() const;
	short clientID()const{return m_cid;}
	table* openTable(const std::string& name, short mode, const char* ownerName);
	const std::string& name()const{return m_dbname;};
	table* useTable(int index, enum_sql_command cmd);
	bool beginTrn(short type);
	bool commitTrn();
	bool abortTrn();
	bool inTransaction()const{return (m_inTransaction!=0);}
	short transactionType()const{return m_trnType;}
	bool beginSnapshot();
	bool endSnapshot();
	bool inSnapshot()const{return m_inSnapshot!=0;}
	bool existsTable(const std::string& name);
	bool existsDatabase();
	void closeTable(const std::string& name, bool drop);
	void closeTable(table* tb);
	void unUseTables(bool rollback);
	void closeForReopen();
	void reopen();
	void cleanTable();
	const std::vector<boost::shared_ptr<table> >& tables() const {return m_tables;};
	static tableCacheCounter tableRef;
};

typedef std::vector<boost::shared_ptr<database> > databases;

class IReadRecordsHandler;
class bookmarks;

/*
 *	Since it differs from the key number which a client specifies
 *   , and an internal key number, it changes.
 *  As for the key name, it is a premise that it is keyNN. 
 *  NN is  client specifies key number.
 */
class keynumConvert
{
	KEY* m_key;
	int m_keyCount;
	char m_keyNum;
	char m_convNum;
	
public:
	keynumConvert(KEY* key, int count):m_key(key)
			,m_keyCount(count),m_keyNum(-1),m_convNum(-1){}
	void setKey(KEY* key){m_key=key;};
	char keyNumByMakeOrder(char num)
	{
		if (m_keyNum==num)
			return m_convNum;
		
		m_keyNum = num;
		for (int i=0;i<m_keyCount;i++)
			if (strstr(m_key[i].name, "key") && m_key[i].name[3] == num + '0')
				return m_convNum = i;
		return m_convNum = num;//If not found, a value as it is is returned.
	}
};



class table : private boost::noncopyable
{
	friend class database;
	TABLE*	m_table;
	std::string m_name;
	const short m_mode;
	int m_id;
	unsigned short m_nullFields;
	uint m_recordLenCl;
	int m_recordFormatType;
#ifdef USE_BTRV_VARIABLE_LEN
	uint m_lastVarLenBytes;
#endif
	database& m_db;
	table(TABLE* table, database& db, const std::string& name, short mode, int id);
	char m_keyNum;
	mutable boost::scoped_array<unsigned char> m_keybuf;
	mutable boost::scoped_array<unsigned char> m_nonNccKeybuf;
	bool m_nonNcc;
	int m_stat;
	bool m_validCursor;
	bool m_cursor;
	bool m_locked;
	bool m_changed;
	bool m_nounlock;
	bool m_bulkInserting;
	int m_percentResult;
	boost::shared_ptr<bookmarks> m_bms;
	String m_str;
	keynumConvert m_keyconv;
	IblobBuffer*  m_blobBuffer;
	std::vector<Field*> m_nonKeySegNullFields;
	
	void moveKey(boost::function<int()> func);
	void readRecords(IReadRecordsHandler* handler, bool includeCurrent, int type, bool noBookmark);
	
	bool keyCheckForPercent();
	inline bool keynumCheck(char num);
	void preBuildPercent(uchar* first, uchar* last);
	inline key_part_map keymap(){return (1U << m_table->key_info[m_keyNum].user_defined_key_parts) -1;} 
	void seekPos(const uchar* pos);
	int setKeyNullFlags();
	void setFiledNullFlags();
	
	bookmarks* bms();
	int percentage(uchar* first, uchar* last, uchar* cur);
	
	bool setNonKey(bool scan=false);
	void fillNull(uchar* ptr, int size);
	inline void* keybuf()const{return &m_keybuf[0];}
	inline uint  keylen()const{return m_table->key_info[m_keyNum].key_length;};
	void setKeyValues(const uchar* ptr, int size);
	void setBlobFieldPointer(const bzs::db::blobHeader* hd);
	inline void unlockRow();
	inline void initHandler()
	{
		if ((m_db.m_inSnapshot==0)&& (m_table->reginfo.lock_type != TL_WRITE))
			m_table->file->init_table_handle_for_HANDLER() ;
	}

#ifdef USE_HANDLERSOCKET
	std::vector<int> m_useFields;
	void checkFiledIndex(int index);
	int fieldIndexByName(const char* name)const;
	void addUseField(int index){m_useFields.push_back(index);};
public:
	std::vector<int>& useFields(){return m_useFields;};
	void setUseFieldList(const std::string& csv);
	void setKeyValues(const std::vector<std::string>& values
							, int keypart, const std::string* inValue=NULL);
	void setValue(int index, const std::string& v, int type);
	void setUseValues(const std::vector<std::string>& values, int type);
	
	#define UPDATE_REPLACE	0
	#define UPDATE_INC		1
	#define UPDATE_DEC		2

#endif
public:
	static bool noKeybufResult;

	~table();

	inline void setBlobBuffer(IblobBuffer*  blobBuffer){m_blobBuffer = blobBuffer;};
	inline IblobBuffer* blobBuffer(IblobBuffer*  blobBuffer){return m_blobBuffer;};

	inline short mode()const {return m_mode;};
	inline bool islocked(){return m_locked;}
	inline void setLocked(bool value){m_locked = value;}
	inline bool isChanged(){return m_changed;}
	int id(){return m_id;};
	inline void unUse(){m_db.unUseTable(this);}
	void resetTransctionInfo(THD* thd);
	void resetInternalTable(TABLE* table);
	inline const std::string& name()const{return m_name;};
	inline int recordFormatType()const {return m_recordFormatType;};
#ifdef USE_BTRV_VARIABLE_LEN
	inline uint lastVarFiledNum()const{return m_table->s->fields-1-nisFields();}
	inline const Field* lastVarFiled ()const{return m_table->s->field[lastVarFiledNum()];}
	unsigned short lastVarLenBytes()const{return m_lastVarLenBytes;};
	unsigned short lastVarFieldPos()const;
	inline unsigned short lastVarFieldDataLen()const{return fieldDataLen(lastVarFiledNum());}

#endif	
	inline int blobFields()const {return m_table->s->blob_fields;};

	/** if call close then table is deleten.
	 */
	inline void close()
	{
		m_table->file->ha_index_or_rnd_end();
		m_db.closeTable(this);
	}
	inline char keyNum()const {return m_keyNum;}
	
	/*
	 *	Since it differs from the key number which a client specifies
	 *   , and an internal key number, it changes.
	 *  As for the key name, it is a premise that it is keyNN. 
	 *  NN is  client specifies key number.
	 */
	inline char keyNumByMakeOrder(char num)
	{
		return m_keyconv.keyNumByMakeOrder(num);
	}
	
	bool setKeyNum(char num, bool sorted = true);
	inline void setKeyNum(const char* name, bool sorted = true){setKeyNum(keynumByName(name), sorted);};
	bool isNisKey(char num)const;
	void seekKey(enum ha_rkey_function find_flag);
	void getNextSame();
	void getLast();
	void getFirst();
	void getNext();
	void getPrev();
	void getByPercentage(unsigned short per);
	void calcPercentage();
	inline int* percentResult(){return &m_percentResult;};

	void stepLast();
	void stepFirst();
	void stepNext();
	void stepPrev();
	void movePos(const uchar* pos, char keyNum, bool sureRawValue=false);

	inline void getNextExt(IReadRecordsHandler* handler, bool includeCurrent, bool noBookmark)
	{
		readRecords(handler, includeCurrent, READ_RECORD_GETNEXT, noBookmark);
	}

	inline void getPrevExt(IReadRecordsHandler* handler, bool includeCurrent, bool noBookmark)
	{
		readRecords(handler, includeCurrent, READ_RECORD_GETPREV, noBookmark);
	}

	inline void stepNextExt(IReadRecordsHandler* handler, bool includeCurrent, bool noBookmark)
	{
		if (m_table->file->inited != handler::RND)
			setNonKey(true);
		readRecords(handler, includeCurrent, READ_RECORD_STEPNEXT, noBookmark);
	}

	inline void stepPrevExt(IReadRecordsHandler* handler, bool includeCurrent, bool noBookmark)
	{
		if (m_table->file->inited != handler::RND)
			setNonKey(true);
		readRecords(handler, includeCurrent, READ_RECORD_STEPPREV, noBookmark);
	}

	void clearBuffer();
	void clearKeybuf(){memset(&m_keybuf[0], 0x00, MAX_KEYLEN);};
	
	__int64 insert(bool ncc);
	void update(bool ncc);
	void updateDirectkey();
	void beginUpdate(char keyNum);
	void del();
	void beginDel();
	
	int keynumByName(const char* name)const;
	int stat(){return m_stat;};
	void setKeyValuesPacked(const uchar* ptr, int size);
	void* record()const;
	
	uint  keyPackCopy(uchar* ptr);
	
	void setRecord(void* ptr, unsigned short size, int offset=0);
	void setRecordFromPacked(const uchar* packedPtr, uint size, const bzs::db::blobHeader* hd);
	uint recordPackCopy(char* buf, uint maxsize=0);

	ushort fieldPackCopy(unsigned char* dest, short filedNum);

	uint fieldSizeByte(int fieldNum){return var_bytes_if(m_table->field[fieldNum]);}
	unsigned short fieldDataLen(int fieldNum)const;
	
	inline unsigned short fieldLen(int fieldNum)const
	{
		return m_table->field[fieldNum]->pack_length();
	}

	inline  unsigned short filedVarLenBytes(int fieldNum)const {return var_bytes_if(m_table->field[fieldNum]);}
	inline char* fieldPos(int fieldNum)const{return (char*)m_table->field[fieldNum]->ptr;};
	inline enum enum_field_types fieldType(int fieldNum) const {return m_table->field[fieldNum]->type();};
	inline unsigned int  fieldFlags(int fieldNum) const {return m_table->field[fieldNum]->flags;};
	inline unsigned short fields()const{return m_table->s->fields;};
	inline unsigned int nisFields()const {return m_nullFields;}
	inline const char* fieldName(int fieldNum)const {return m_table->s->field[fieldNum]->field_name;};
	inline const CHARSET_INFO& fieldCharset(int fieldNum)const {return *m_table->s->field[fieldNum]->charset();}
	/*number of key.*/
	inline unsigned short keys()const{return m_table->s->keys;};
	inline const KEY&  keyDef(char keyNum)const{return m_table->key_info[keyNum];};
	inline const KEY* primaryKey()
	{
		return (m_table->s->primary_key != MAX_KEY)
				? &m_table->key_info[m_table->s->primary_key] : NULL;
	}
	inline Field*  field(int fieldNum)const{return m_table->field[fieldNum];};
	inline char  primarykey()const{return m_table->s->primary_key;};
	/** is this view. not table*/
	inline bool isView()const{return m_table->s->is_view;}

	/** char set of this table.*/
	inline const CHARSET_INFO& charset()const {return *m_table->s->table_charset;}
	
	ha_rows recordCount(bool estimate);
	inline ulong recordLen()const{return m_table->s->reclength;}

	/** record length for client side. exclude null flag segmnet 
	 *  ,lastVarLenBytes and NIS fields from recordLen()
	 */
	inline uint recordLenCl()const{return m_recordLenCl;}
	inline uint nullBytes()const{return (uint)(m_table->s->null_fields>0);}
	/** bookmark length*/
	uint posPtrLen()const;

	/** bookmark */
	const uchar* position(bool raw=false);
	const char* keyName(char keyNum);
	inline void errorMessage(String *buf){m_table->file->get_error_message(m_stat, buf);}
	
	void startBulkInsert(ha_rows rows);
	void endBulkInsert();

	inline TABLE* internalTable(){return m_table;} ///for debuglog
	const char* valStr(int index, int &size);			///for debuglog
	
	/** add blob field list.
	 *  @param fieldNum field index if it is -1 then all fields of cureent row;
	 *  @return field count of listed.
	 */
	uint makeBlobFieldList(int fieldNum);
	
	inline void addBlobBuffer(int fieldNum)
	{
		Field* fd = m_table->field[fieldNum];
		m_blobBuffer->addBlob(blob_len(fd), fd->field_index, blobBodyPtr(fd));
	}

	inline void setBlobFieldCount(uint num){m_blobBuffer->setFieldCount(num);};
	inline void indexInit()
	{
		m_table->file->ha_index_or_rnd_end();
		if (m_keyNum >= 0)
			m_table->file->ha_index_init(m_keyNum, true);
		else
			m_table->file->ha_rnd_init(true);
	};
};

class fieldBitmap
{
	bool m_keyRead;
	TABLE* m_table;
public:

	inline fieldBitmap(TABLE* table):m_table(table),m_keyRead(false)
	{
		m_table->read_set = &m_table->tmp_set;
		m_table->write_set = &m_table->tmp_set;
		bitmap_clear_all(m_table->read_set);
	}

	inline fieldBitmap():m_table(NULL),m_keyRead(false)
	{
	}
	
	inline void setTable(table* tb)
	{
		if (tb)
		{
			m_table = tb->internalTable();
			m_table->read_set = &m_table->tmp_set;
			m_table->write_set = &m_table->tmp_set;
			bitmap_clear_all(m_table->read_set);
		}else if (m_table)
		{
			if (m_keyRead)
				m_table->file->extra(HA_EXTRA_NO_KEYREAD);
			m_table->read_set = &m_table->s->all_set;
			m_table->write_set = &m_table->s->all_set;
			m_table = NULL;
		}
	}
	
	
	inline ~fieldBitmap()
	{
		if (m_table)
		{
			if (m_keyRead)
				m_table->file->extra(HA_EXTRA_NO_KEYREAD);
			m_table->read_set = &m_table->s->all_set;
			m_table->write_set = &m_table->s->all_set;
		}
	}
	
	inline void setKeyRead(bool v)
	{
		assert(m_table);
		if (v)
			m_table->file->extra(HA_EXTRA_KEYREAD);
		else if (m_keyRead)
			m_table->file->extra(HA_EXTRA_NO_KEYREAD);
		m_keyRead = v;
	}

	inline void setReadBitmap(uint bit)
	{
		assert(m_table);
		bitmap_set_bit(m_table->read_set, bit);
	}
};


//smart wrapper for exception
class smartBulkInsert
{
	table* m_tb;
public:
	smartBulkInsert(table* tb, ha_rows rows):m_tb(tb)
	{
		m_tb->startBulkInsert(rows);
	}

	~smartBulkInsert()
	{
		m_tb->endBulkInsert();
	}
};

class smartTransction
{
	database* m_db;
	short m_type;
public:
	smartTransction(database* db, short type=TRN_RECORD_LOCK_SINGLE):m_db(db)
	{
		m_db->beginTrn(type);	
	}

	void end()
	{
		m_db->commitTrn();
		m_db=NULL;
	}

	~smartTransction()
	{
		if (m_db)
			m_db->abortTrn();
	}

};

#define BUILINSERT_SCOPE

}//namespace mysql
}//namespace engine
}//namespace db
}//namespace bzs

#endif //DATABASE_H


