#ifndef BZS_DB_ENGINE_MYSQL_DATABASE_H
#define BZS_DB_ENGINE_MYSQL_DATABASE_H
/* =================================================================
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
 ================================================================= */

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

#define RF_FIXED_LEN 0
#define RF_FIXED_PLUS_VALIABLE_LEN 1
#define RF_VALIABLE_LEN 2
#define RF_INCLUDE_NIS 4

#define TRN_RECORD_LOCK_SINGLE 0
#define TRN_RECORD_LOCK_MUILTI 1

#define MODE_READ_ONLY -2
#define MODE_EXCLUSIVE -4
#define MODE_READ_EXCLUSIVE -6


/** bookmark size
 *  btreive API is MAX 4 byte
 */
#define REF_SIZE_MAX 112
class table;


/** Control mysql table cahche
 */
class tableCacheCounter
{
    std::vector<int> m_tables;
    std::vector<int> m_counts;

    boost::mutex m_mutex;
    int getHash(const std::string& dbname, const std::string& tbname);
    size_t getCounterIndex(const std::string& dbname,
                           const std::string& tbname);

public:
    tableCacheCounter();
    void addref(const std::string& dbname, const std::string& tbname);
    int count(const std::string& dbname, const std::string& tbname);
    void release(const std::string& dbname, const std::string& tbname);
    boost::mutex& mutex() { return m_mutex; }
};

struct rowLockMode
{
    bool lock : 1;
    bool read : 1;
};

class database : private boost::noncopyable
{
    friend class table;
    friend class smartDbReopen;
public:
    typedef std::vector<boost::shared_ptr<table> > tableList;
private:
    std::string m_dbname;
    mutable THD* m_thd;
    int m_inTransaction;
    int m_inSnapshot;
    int m_stat;
    int m_usingExclusive;
    table* m_inAutoTransaction;
    short m_trnType;
    short m_cid;
    enum_tx_isolation m_iso;
    tableList m_tables;
    ulong m_privilege;
   
    TABLE* doOpenTable(const std::string& name, short mode,
                                const char* ownerName);
    void unUseTable(table* tb);
    size_t getNomalOpenTables(tableList& tables);
    void prebuildIsoratinMode();
    void prebuildExclusieLockMode(table* tb);
    void prebuildLocktype(table* tb, enum_sql_command& cmd, rowLockMode* lck) ;
    void changeIntentionLock(table* tb, thr_lock_type lock_type);
    void checkACL(enum_sql_command cmd);
    void releaseTable(size_t index);
public:
    

    database(const char* name, short cid);
    ~database();
    bool setGrant(const char* host, const char* user);

    unsigned char* getUserSha1Passwd(const char* host, const char* user,
        unsigned char* buf);

    int stat() { return m_stat; }

    THD* thd() const { return m_thd; }
    void use() const;

    short clientID() const { return m_cid; }

    const std::string& name() const { return m_dbname; }

    bool inTransaction() const { return (m_inTransaction != 0); }

    short transactionType() const { return m_trnType; }

    enum_tx_isolation transactionIsolation() const { return m_iso; }

    bool inSnapshot() const { return m_inSnapshot != 0; }

    const std::vector<boost::shared_ptr<table> >& tables() const
    {
        return m_tables;
    }

    bool beginSnapshot(enum_tx_isolation iso);
    bool endSnapshot();
    table* openTable(const std::string& name, short mode,
                     const char* ownerName);
    table* useTable(int index, enum_sql_command cmd, rowLockMode* lck);
    bool beginTrn(short type, enum_tx_isolation iso);
    bool commitTrn();
    bool abortTrn();
    bool existsDatabase();
    void closeTable(const std::string& name, bool drop);
    void closeTable(table* tb);
    void unUseTables(bool rollback);
    void closeForReopen();
    void reopen();
    void cleanTable();

    inline bool canUnlockRow() const
    {
        /* inSnapshot or inTransaction multi record lock do not unlock */
        if (m_inSnapshot || (m_inTransaction && (m_trnType == TRN_RECORD_LOCK_MUILTI)))
            return false;
        return true;
    }

    inline bool noUserTransaction() const
    {
        return ((m_inSnapshot + m_inTransaction) == 0);
    }

    short aclReload();
    inline void setCurTime(){m_thd->set_current_time();}

    static tableCacheCounter tableRef;

};

typedef std::vector<boost::shared_ptr<database> > databases;

class IReadRecordsHandler;
class IPrepare;
class bookmarks;

//unsigned char* getUserSha1Passwd(const char* host, const char* user, unsigned char* buf);
//short aclReload();
/*
 *  Since it differs from the key number which a client specifies
 *   , and an internal key number, it changes.
 *  As for the key name, it is a premise that it is keyNN.
 *  NN is  client specifies key number.
 */
class keynumConvert
{
    const KEY* m_key;
    int m_keyCount;
    char m_keyNum;
    char m_convNum;

public:
    keynumConvert(const KEY* key, int count)
        : m_key(key), m_keyCount(count), m_keyNum(-1), m_convNum(-1)
    {
    }

    void setKey(KEY* key) { m_key = key; }

    char keyNumByMakeOrder(char num)
    {
        if (m_keyNum == num)
            return m_convNum;

        m_keyNum = num;
        for (int i = 0; i < m_keyCount; i++)
            if (strstr(m_key[i].name, "key") && m_key[i].name[3] == num + '0')
                return m_convNum = i;
        return m_convNum = num; // If not found, a value as it is returned.
    }

    char clientKeynum(char num)
    {
        if (num < m_keyCount)
        {
            if (strstr(m_key[num].name, "key"))
                return m_key[num].name[3] - '0';
        }
        return -1;
    }
};

class table : private boost::noncopyable
{
    friend class database;
    friend struct smartForceConsistantRead;
    TABLE* m_table;

    std::string m_name;

    short m_mode;
    unsigned short m_nisNullFields;
    int m_id;
    uint m_recordLenCl;
    int m_recordFormatType;
#ifdef USE_BTRV_VARIABLE_LEN
    uint m_lastVarLenBytes;
#endif
    database& m_db;
    mutable boost::scoped_array<unsigned char> m_keybuf;

    int m_stat;
    int m_percentResult;
    boost::shared_ptr<bookmarks> m_bms;
    String m_str;
    keynumConvert m_keyconv;
    IblobBuffer* m_blobBuffer;
    std::vector<Field*> m_noNullModeNullFieldList;
    std::vector<Field*> m_timeStampFields;
    unsigned int m_readCount;
    unsigned int m_updCount;
    unsigned int m_delCount;
    unsigned int m_insCount;
    char m_keyNum;
    struct
    {
        bool m_nonNcc : 1;
        bool m_validCursor : 1;
        bool m_cursor : 1;
        bool m_locked : 1;
        bool m_changed : 1;
        bool m_nounlock : 1;
        bool m_bulkInserting : 1;
        bool m_delayAutoCommit : 1;
    };
    struct
    {
        bool m_forceConsistentRead : 1;
        bool m_mysqlNull;
    };

    table(TABLE* table, database& db, const std::string& name, short mode,
          int id, bool mysqlnull);
    void moveKey(boost::function<int()> func);
    void readRecords(IReadRecordsHandler* handler, bool includeCurrent,
                     int type, bool noBookmark);

    bool keyCheckForPercent();
    void preBuildPercent(uchar* first, uchar* last);
    void seekPos(const uchar* pos);
    int setKeyNullFlags();
    void setFieldNullFlags();
	void setTimeStamp(bool insert);

    bookmarks* bms();
    int percentage(uchar* first, uchar* last, uchar* cur);

    bool setNonKey(bool scan = false);
    void fillNull(uchar* ptr, int size);

    inline void* keybuf() const { return &m_keybuf[0]; }

    inline uint keylen() const
    {
        return m_table->key_info[(int)m_keyNum].key_length;
    }
    void setKeyValues(const uchar* ptr, int size);
    void setBlobFieldPointer(const bzs::db::blobHeader* hd);
    
    inline bool setCursorStaus()
    {
        if (m_stat == 0)
        {
            ++m_readCount;
            m_validCursor = true;
            m_cursor = true;
        }else
        {
            m_validCursor = false;
            m_cursor = ((m_stat == HA_ERR_LOCK_WAIT_TIMEOUT) ||
                        (m_stat == HA_ERR_LOCK_DEADLOCK)) ? m_cursor : false;
        }
        return m_validCursor;
    }
    
    inline void unlockRow(bool noConsistent);
    inline void tryConsistentRead(bool noConsistent);

#ifdef USE_HANDLERSOCKET
    std::vector<int> m_useFields;
    void checkFiledIndex(int index);
    int fieldIndexByName(const char* name) const;

    void addUseField(int index) { m_useFields.push_back(index); };

public:
    std::vector<int>& useFields() { return m_useFields; };
    void setUseFieldList(const std::string& csv);
    void setValue(int index, const std::string& v, int type);
    void setUseValues(const std::vector<std::string>& values, int type);

#define UPDATE_REPLACE 0
#define UPDATE_INC 1
#define UPDATE_DEC 2

#endif

public:
    static bool noKeybufResult;
    std::vector<IPrepare*> preparedStatements;

    ~table();

    inline void setBlobBuffer(IblobBuffer* blobBuffer)
    {
        m_blobBuffer = blobBuffer;
    }

    inline IblobBuffer* blobBuffer() const
    {
        return m_blobBuffer;
    }

    inline short mode() const { return m_mode; }

    inline bool cursor() const {return m_cursor;}

    inline bool isReadOnly() const 
    {
        return (m_mode == MODE_READ_ONLY) 
             ||(m_mode == MODE_READ_EXCLUSIVE); 
    }

    inline bool isExclusveMode() const { return m_mode <= MODE_EXCLUSIVE;}

    inline bool isNomalMode() const { return m_mode > MODE_EXCLUSIVE;}

    inline bool islocked() { return m_locked; }

    inline void setLocked(bool value) { m_locked = value; }

    inline bool isChanged() { return m_changed; }

    int id() const { return m_id; };

    /* The singleRowLock is no effects with Transaction or Snapshot. */
    inline void unUse() 
    { 
        if (m_delayAutoCommit)
        {
            m_delayAutoCommit = false;
            m_db.m_inAutoTransaction = this;
        }
        else
            m_db.unUseTable(this);
    }

    inline const std::string& name() const { return m_name; }

    inline int recordFormatType() const { return m_recordFormatType; }
    void resetTransctionInfo(THD* thd);
    void resetInternalTable(TABLE* table);

#ifdef USE_BTRV_VARIABLE_LEN

    inline uint lastVarFieldNum() const
    {
        return m_table->s->fields - 1 - nisFields();
    }

    inline const Field* lastVarFiled() const
    {
        return m_table->s->field[lastVarFieldNum()];
    }

    unsigned short lastVarLenBytes() const { return m_lastVarLenBytes; }

    inline unsigned short lastVarFieldDataLen() const
    {
        return fieldDataLen(lastVarFieldNum());
    }

    unsigned short lastVarFieldPos() const;
#endif

    inline int blobFields() const { return m_table->s->blob_fields; }

    /** if call close then table is deleten.
     */
    inline void close()
    {
        m_table->file->ha_index_or_rnd_end();
        m_db.closeTable(this);
    }

    inline char keyNum() const { return m_keyNum; }

    inline bool isUniqueKey()
    {
        return (m_table->key_info[(int)m_keyNum].flags & HA_NOSAME);
    }

    /*
     *  Since it differs from the key number which a client specifies
     *   , and an internal key number, it changes.
     *  As for the key name, it is a premise that it is keyNN.
     *  NN is  client specifies key number.
     */
    inline char keyNumByMakeOrder(char num)
    {
        return m_keyconv.keyNumByMakeOrder(num);
    }

    inline bool keynumCheck(char num)
    {
        return ((num >= 0) && (num < (short)m_table->s->keys));
    }

    bool setKeyNum(char num, bool sorted = true);

    inline void setKeyNum(const char* name, bool sorted = true)
    {
        setKeyNum(keynumByName(name), sorted);
    }
    bool isNisKey(char num) const;

    inline key_part_map keymap()
    {
        return (1U << m_table->key_info[(int)m_keyNum].user_defined_key_parts) - 1;
    }
    unsigned long long tableFlags() const { return m_table->file->ha_table_flags();}
    void seekKey(enum ha_rkey_function find_flag, key_part_map keyMap);
    void getNextSame(key_part_map keyMap);
    void getLast();
    void getFirst();
    void getNext();
    void getPrev();
    void getByPercentage(unsigned short per);
    void calcPercentage();

    inline int* percentResult() { return &m_percentResult; }

    void stepLast();
    void stepFirst();
    void stepNext();
    void stepPrev();
    void movePos(const uchar* pos, char keyNum, bool sureRawValue = false);

    inline void getNextExt(IReadRecordsHandler* handler, bool includeCurrent,
                           bool noBookmark)
    {
        readRecords(handler, includeCurrent, READ_RECORD_GETNEXT, noBookmark);
    }

    inline void getPrevExt(IReadRecordsHandler* handler, bool includeCurrent,
                           bool noBookmark)
    {
        readRecords(handler, includeCurrent, READ_RECORD_GETPREV, noBookmark);
    }

    inline void stepNextExt(IReadRecordsHandler* handler, bool includeCurrent,
                            bool noBookmark)
    {
        if (m_table->file->inited != handler::RND)
            setNonKey(true);
        readRecords(handler, includeCurrent, READ_RECORD_STEPNEXT, noBookmark);
    }

    inline void stepPrevExt(IReadRecordsHandler* handler, bool includeCurrent,
                            bool noBookmark)
    {
        if (m_table->file->inited != handler::RND)
            setNonKey(true);
        readRecords(handler, includeCurrent, READ_RECORD_STEPPREV, noBookmark);
    }

    void clearBuffer();

    inline void clearKeybuf() { memset(&m_keybuf[0], 0x00, MAX_KEYLEN); }

    __int64 insert(bool ncc);
    void update(bool ncc);
    void updateDirectkey();
    void beginUpdate(char keyNum);
    void del();
    void beginDel();

    int keynumByName(const char* name) const;

    inline int stat() { return m_stat; }
    short setKeyValuesPacked(const uchar* ptr, int size);
    void* record() const;

    uint keyPackCopy(uchar* ptr);

    void setRecord(void* ptr, unsigned short size, int offset = 0);
    void setRecordFromPacked(const uchar* packedPtr, uint size,
                             const bzs::db::blobHeader* hd);
    uint recordPackCopy(char* buf, uint maxsize = 0);

    ushort fieldPackCopy(unsigned char* nullPtr, int& nullbit, unsigned char* dest, short fieldNum);

    inline uint fieldSizeByte(int fieldNum)
    {
        return var_bytes_if(m_table->field[fieldNum]);
    }
    unsigned short fieldDataLen(int fieldNum) const;

    inline unsigned short fieldLen(int fieldNum) const
    {
        return m_table->field[fieldNum]->pack_length();
    }

    inline unsigned short filedVarLenBytes(int fieldNum) const
    {
        return var_bytes_if(m_table->field[fieldNum]);
    }

    inline char* fieldPos(int fieldNum) const
    {
        return (char*)m_table->field[fieldNum]->ptr;
    }

    inline enum enum_field_types fieldType(int fieldNum) const
    {
        return m_table->field[fieldNum]->type();
    }

    inline enum enum_field_types fieldRealType(int fieldNum) const
    {
        return m_table->field[fieldNum]->real_type();
    }

    inline unsigned int fieldFlags(int fieldNum) const
    {
        return m_table->field[fieldNum]->flags;
    }

    inline unsigned short fields() const { return m_table->s->fields; }

    inline bool isMysqlNull() const { return m_mysqlNull; }

    inline unsigned int nisFields() const { return m_nisNullFields; }

    inline const char* fieldName(int fieldNum) const
    {
        return m_table->s->field[fieldNum]->field_name;
    }

    inline const CHARSET_INFO& fieldCharset(int fieldNum) const
    {
        return *m_table->s->field[fieldNum]->charset();
    }

    /* number of key. */
    inline unsigned short keys() const { return m_table->s->keys; }

    inline const KEY& keyDef(char keyNum) const
    {
        return m_table->key_info[(int)keyNum];
    }

    inline const KEY* primaryKey() const
    {
        return (m_table->s->primary_key <= MAX_KEY)
                   ? &m_table->key_info[m_table->s->primary_key]
                   : NULL;
    }

    inline Field* field(int fieldNum) const { return m_table->field[fieldNum]; }

    inline char primarykeyNum() const { return m_table->s->primary_key; }

    /** is this view. not table */
    inline bool isView() const { return m_table->s->is_view; }

    /** char set of this table. */
    inline const CHARSET_INFO& charset() const
    {
        return *m_table->s->table_charset;
    }

    ha_rows recordCount(bool estimate);

    inline ulong recordLen() const { return m_table->s->reclength; }

    /** record length for client side. exclude null flag segmnet
     *  ,lastVarLenBytes and NIS fields from recordLen()
     */
    inline uint recordLenCl() const { return m_recordLenCl; }

    inline uint nullBytes() const
    {
        return (uint)(m_table->s->null_fields > 0);
    }
    /** bookmark length */
    uint posPtrLen() const;

    inline uint posPtrLenRaw() const
    {
        return m_table->file->ref_length;
    }

    /** bookmark */
    const uchar* position(bool raw = false);
    const char* keyName(char keyNum);

    inline void errorMessage(String* buf)
    {
        m_table->file->get_error_message(m_stat, buf);
    }

    void startBulkInsert(ha_rows rows);
    void endBulkInsert();

    inline TABLE* internalTable() { return m_table; }
    /// for debuglog
    const char* valStr(int index, int& size);
    /// for debuglog

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

    inline void setBlobFieldCount(uint num)
    {
        m_blobBuffer->setFieldCount(num);
    }

    inline uint getBlobFieldCount()
    {
        return m_blobBuffer->fieldCount();
    }

    inline void indexInit()
    {
        int ret = m_table->file->ha_index_or_rnd_end();
        assert(ret == 0);
        if (m_keyNum >= 0)
            ret = m_table->file->ha_index_init(m_keyNum, true);
        else
            ret = m_table->file->ha_rnd_init(true);
        assert(ret == 0);
    }

    inline void setRowLock(rowLockMode* lck)
    {
        if (lck->lock && m_db.noUserTransaction() && isNomalMode())
            m_delayAutoCommit = true;
        else
            m_delayAutoCommit = false;
    }

    inline void setRowLockError()
    {
        m_delayAutoCommit = false;
    }

    inline bool isDelayAutoCommit() const
    {
        return m_delayAutoCommit;
    }

    inline short unlock()
    {
        if (m_db.inSnapshot() || m_db.inTransaction())
        {
            if (m_validCursor)
            {
                m_table->file->unlock_row();
                m_nounlock = true;
            }
            else
                return 1;
        }else if (m_db.m_inAutoTransaction == this)
            unUse();
        return 0;
    }

    inline void startStmt()
    {
        m_validCursor = false;
        m_table->file->start_stmt(m_db.m_thd, m_table->reginfo.lock_type);
    }

    inline void initForHANDLER()
    {
        if (!m_db.m_thd->in_lock_tables)
        {
            m_table->file->init_table_handle_for_HANDLER();
            m_validCursor = false;
        }
    }
    void setKeyValues(const std::vector<std::string>& values, int keypart,
                    const std::string* inValue = NULL);

    unsigned int writeDefaultImage(unsigned char* p, size_t size);

    unsigned int writeSchemaImage(unsigned char* p, size_t size);


    void restoreRecord() { restore_record(m_table, s->default_values);}

    inline  unsigned int readCount() const { return m_readCount; }

    inline  unsigned int updCount() const { return m_updCount; }

    inline  unsigned int delCount() const { return m_delCount; }

    inline  unsigned int insCount() const { return m_insCount; }
};

class fieldBitmap
{
    TABLE* m_table;
    bool m_keyRead;

public:
    inline fieldBitmap(TABLE* table) : m_table(table), m_keyRead(false)
    {
        m_table->read_set = &m_table->tmp_set;
        m_table->write_set = &m_table->tmp_set;
        bitmap_clear_all(m_table->read_set);
    }

    inline fieldBitmap() : m_table(NULL), m_keyRead(false) {}

    inline void setTable(table* tb)
    {
        if (tb)
        {
            m_table = tb->internalTable();
            m_table->read_set = &m_table->tmp_set;
            m_table->write_set = &m_table->tmp_set;
            bitmap_clear_all(m_table->read_set);
        }
        else if (m_table)
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

       
    inline MY_BITMAP* getReadBitmap()
    {
        if (m_table)
            return m_table->read_set;
        return NULL;
    }

    inline bool isUsing() const { return (m_table != NULL); }

};

// smart wrapper for exception
class smartBulkInsert
{
    table* m_tb;

public:
    smartBulkInsert(table* tb, ha_rows rows) : m_tb(tb)
    {
        m_tb->startBulkInsert(rows);
    }

    ~smartBulkInsert() { m_tb->endBulkInsert(); }
};

class smartTransction
{
    database* m_db;
    short m_type;

public:
    smartTransction(database* db, short type = TRN_RECORD_LOCK_SINGLE
            , enum_tx_isolation iso = ISO_READ_COMMITTED)
        : m_db(db)
    {
        m_db->beginTrn(type, iso);
    }

    void end()
    {
        m_db->commitTrn();
        m_db = NULL;
    }

    ~smartTransction()
    {
        if (m_db)
            m_db->abortTrn();
    }
};


struct smartForceConsistantRead
{
    table* tb;
    inline smartForceConsistantRead(table* t):tb(t)
    {
        tb->m_forceConsistentRead = true;
    }

    inline ~smartForceConsistantRead()
    {
        tb->m_forceConsistentRead = false;
    }
};


class igetDatabases
{
public:
    virtual ~igetDatabases(){};
    virtual const databases& dbs() const = 0;
    virtual boost::mutex& mutex() = 0;
};

#define BUILINSERT_SCOPE

} // namespace mysql
} // namespace engine
} // namespace db
} // namespace bzs

#endif // BZS_DB_ENGINE_MYSQL_DATABASE_H
