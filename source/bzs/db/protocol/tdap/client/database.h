#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_DATABASE_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_DATABASE_H
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
class dbdef;

#if (defined(__BORLANDC__) && !defined(__APPLE__) && !defined(__clang__))
typedef bool __stdcall (*deleteRecordFn)(database* db, table* tb, bool inkey);
typedef short __stdcall (*schemaMgrFn)(database* db);
typedef void __stdcall (*copyDataFn)(database* db, table* tb, int recordCount, int count,
                                     bool& cancel);
#else
/** @cond INTERNAL */
/** Callback function on a record was deleted. */
typedef bool(__STDCALL* deleteRecordFn)(database* db, table* tb, bool inkey);
/** @endcond */

/** Callback function on a database is opening by database::open operation.
 This is use for change a table schema and table data at before database
 */
typedef short(__STDCALL* schemaMgrFn)(database* db);

/** Callback function on a record was copied by convert table operation. */
typedef void(__STDCALL* copyDataFn)(database* db, table* tb, int recordCount, int count,
                                    bool& cancel);
#endif

class DLLLIB database : public nsdatabase
{
    friend struct openTablePrams;
    struct dbimple* m_impl;

    void setDir(const _TCHAR* directory);
    virtual table* createTableObject();
    bool defaultImageCopy(const void* data, short& tableIndex);
    short checkOpened();
    table* doOpenTable(struct openTablePrams* pm, const _TCHAR* ownerName);
    void* getExtendBufferForOpen(uint_td& size); // orverride
    _TCHAR* getTableUri(_TCHAR* buf, short fileNum);
    _TCHAR* getTableUri(_TCHAR* buf, const _TCHAR* filename);
    inline void copyEachFieldData(table* dest, table* src, struct fieldChnageInfo* fci);


protected:
    database& operator=(const database&);
    database();
    virtual ~database();
    void setLockReadOnly(bool v);
    virtual void doClose();
    virtual void doOpen(const _TCHAR* uri, short type, short mode,
                        const _TCHAR* username);

    virtual bool onOpenAfter() { return true; };

    virtual bool onTableOpened(table* tb, short fileNum, short mode,
                        bool isCreated) {return true;}
    virtual char* getContinuousList(int option);
    virtual void onCopyDataInternal(table* tb, int recordCount, int count,
                                    bool& cancel);
    virtual void doConvertTable(short tableIndex, bool turbo,
                                const _TCHAR* ownerName);
    virtual bool doReopenDatabaseSchema();

public:
    virtual void release();
    dbdef* dbDef() const;
    const _TCHAR* rootDir() const;
    void setRootDir(const _TCHAR* directory);
    void* optionalData() const;
    void setOptionalData(void* v);
    bool tableReadOnly() const;
    void setTableReadOnly(bool value);
    const deleteRecordFn onDeleteRecord() const;
    void setOnDeleteRecord(const deleteRecordFn v);
    const copyDataFn onCopyData() const;
    void setOnCopyData(const copyDataFn v);
    bool open(const _TCHAR* uri, short schemaType = 0, short mode = -2,
              const _TCHAR* dir = NULL, const _TCHAR* ownerName = NULL);
    table* openTable(short fileNum, short mode = TD_OPEN_NORMAL,
                     bool autoCreate = true, const _TCHAR* ownerName = NULL,
                     const _TCHAR* uri = NULL);
    table* openTable(const _TCHAR* tableName, short mode = 0,
                     bool autoCreate = true, const _TCHAR* ownerName = NULL,
                     const _TCHAR* uri = NULL);
    database* clone();
    bool createTable(const char* utf8Sql);
    bool createTable(short fileNum, const _TCHAR* uri = NULL);
    char* getSqlStringForCreateTable(const _TCHAR* tableName, char* retbuf, uint_td* size);
    void create(const _TCHAR* uri, short type = TYPE_SCHEMA_BDF);
    void drop(const _TCHAR* uri=NULL);
    void dropTable(const _TCHAR* tableName);
    void close(bool withDropDefaultSchema = false);
    short aclReload();
    short continuous(char_td op = TD_BACKUP_START, bool inclideRepfile = false);
    short assignSchemaData(const dbdef* src);
    short copyTableData(table* dest, table* src, bool turbo,
                        short keyNum = -1, int maxSkip = -1);
    void convertTable(short tableIndex, bool turbo,
                      const _TCHAR* ownerName = NULL);
    bool existsTableFile(short tableIndex, const _TCHAR* ownerName = NULL);
    void getBtrVersion(btrVersions* versions);
    bool isOpened() const; // orverride
    char_td mode() const;
    bool autoSchemaUseNullkey() const;
    void setAutoSchemaUseNullkey(bool v);
    database* createAssociate();
    virtual int defaultAutoIncSpace() const { return 0; };
    inline bool execSql(const char* utf8Sql){return createTable(utf8Sql);}

    static database* create();
    /* For C++ direct only. don't use by wrapper class for COM or SWIG
     This method is ignore refarence count of nsdatabse.
     */
    static void destroy(database* db);
    static void setCompatibleMode(int mode);
    static int compatibleMode();
    static const int CMP_MODE_MYSQL_NULL = 1; //default
    static const int CMP_MODE_OLD_NULL =  0;
};

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#endif // BZS_DB_PROTOCOL_TDAP_CLIENT_DATABASE_H
