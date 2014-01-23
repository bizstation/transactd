#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_DBDEF_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_DBDEF_H
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
#include "nsTable.h"

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

#define TABLE_NUM_TMP 512

/* field size for validation.
 */
enum eFieldQuery {
    eMinlen, eMaxlen, eDefaultlen, eDecimals};


class AGRPACK dbdef : private nstable
{
    friend class database;
    struct dbdimple* m_impl;
    dbdef(const dbdef&);
    dbdef& operator = (const dbdef&);

    bool isUsedField(short tableIndex, short deleteIndex);
    void renumberFieldNum(short tableIndex, short Index, short op);
    bool resizeAt(short tableIndex, bool key);
    bool resizeReadBuf(void);
    void moveById(short id);
    bool validLen(uchar_td FieldType, uint_td FieldLen);
    bool isPassKey(uchar_td FieldType);
    void openDdf(const _TCHAR* dir, short Mode, const _TCHAR* OwnerName);
    void createDDF(const _TCHAR* fullpath);
    void saveDDF(short tableIndex, short opration, bool forPsqlDdf=true);
    ushort_td getDDFNewTableIndex();
    ushort_td getDDFNewFieldIndex();
    int totalDefLength(short tableIndex);
    void setRecordLen(short tableIndex);
    void setCodePage(tabledef* rd);

    void doOpen(const _TCHAR* uri, char_td mode = 0, const _TCHAR* ownername = NULL);
    void doClose();
    keylen_td writeKeyData();
    void writeRecordData(){};
    void onReadAfter(){};
    void drop();
    inline nstable* table() {return this;}
    inline fielddef_t_my& convert(fielddef_t_my& fd_my, const fielddef_t_pv& fd_pv);
    void tableDefCopy(tabledef* dest, tabledef* src, size_t size);

    ~dbdef();
    dbdef(nsdatabase *pbe, short defType);
    void create(const _TCHAR* uri);
    void autoMakeSchema();
    void setDefType(short defType);
    static keydef* getKeyDef(tabledef* p);
    static fielddef* getFieldDef(tabledef* p);

public:
	using nstable::addref;
	using nstable::release;
	using nstable::refCount;
    short tableCount() const ;
    void* relateData() const ;
    short openMode() const ;
    tabledef* tableDefs(int index);
	tabledef** tableDefPtr(int index);
    void setVersion(int v);
    int version() const ;

    inline short_td stat() const {return m_stat;}

    void updateTableDef(short tableIndex, bool forPsqlDdf=true);
    fielddef* insertField(short tableIndex, short insertIndex);
    void deleteField(short tableIndex, short deleteIndex);
    keydef* insertKey(short tableIndex, short insertIndex);
    void deleteKey(short tableIndex, short deleteIndex);
    void insertTable(tabledef* tableDef);
    void deleteTable(short tableIndex);
    void renumberTable(short oldIndex, short newIndex);
    short tableNumByName(const _TCHAR* tableName);
    ushort_td getRecordLen(short tableIndex);

    void getFileSpec(fileSpec* fs, short tableIndex);
    short findKeynumByFieldNum(short tableIndex, short index);
    short fieldNumByViewNum(short tableIndex, short index);
    short fieldNumByName(short tableIndex, const _TCHAR* name);
    void* allocRelateData(int size);

    uint_td fieldValidLength(eFieldQuery query, uchar_td fieldType);
    void pushBackup(short tableIndex);
    bool compAsBackup(short tableIndex);
    void popBackup(short tableIndex);

    inline short_td tdapErr(HWND hWnd, _TCHAR* retbuf = NULL) {
        return nstable::tdapErr(hWnd, retbuf);}

    void reopen(char_td mode=TD_OPEN_READONLY);
    using nstable::setStat;
    static ushort_td getFieldPosition(tabledef *tableDef, short fieldNum);
    static void cacheFieldPos(tabledef *tableDef);


};

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs
#endif//BZS_DB_PROTOCOL_TDAP_CLIENT_DBDEF_H
