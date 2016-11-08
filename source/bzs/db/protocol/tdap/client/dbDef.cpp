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
#include "dbDef.h"
#include "database.h"
#include <bzs/db/protocol/tdap/uri.h>
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


/* BDFFORMAT_VERSION used in the databaseSchema.cpp listSchemaTable() */
static const _TCHAR BDFFORMAT_VERSION[] = _T("2.000.00");
static const _TCHAR ow0[11] = { 46, 46, 83, 67, 46, 46, 46, 46, 93, 4, 0 };
static const _TCHAR ow1[11] = { 46, 46, 83, 67, 46, 46, 46, 46, 66, 5, 0 };
static const _TCHAR ow2[11] = { 46, 46, 83, 67, 46, 46, 46, 46, (_TCHAR)-44, 5, 0 };
using namespace bzs::rtl;

class ownerNameSetter : public nstable
{
    void onReadAfter(){};
    void writeRecordData(){};

public:
    ownerNameSetter(nsdatabase* pbe) : nstable(pbe){};
    using nstable::open;
};

static const int BDFMAXBUFFER_BTRV = 32384;
static const int BDFMAXBUFFER = MAX_SCHEMASIZE + 0xFFFF;
struct dbdimple
{
    tabledef* tableDefs[TABLE_NUM_TMP + 1];
    char keybuf[128];
    tabledef* bdf;
    void* relateData;
    ushort_td maxid;
    short tableCount;
    short openMode;
    short deftype;
    _TCHAR userName[20];
    uint_td bdfLen;
    _TCHAR version[9];
    bool noWriteMode;

    dbdimple()
        : bdf(NULL), relateData(NULL), maxid(0), tableCount(0), openMode(1),
          deftype(TYPE_SCHEMA_BDF), bdfLen(BDFMAXBUFFER), noWriteMode(false)
    {
        memset(tableDefs, 0, (TABLE_NUM_TMP + 1) * sizeof(tabledef*));
        _tcscpy(version, (const _TCHAR*)BDFFORMAT_VERSION);
    }

    ~dbdimple()
    {
        for (int i = 0; i <= tableCount; i++)
        {
            tabledef* td = tableDefs[i];
            if (td && (td != (tabledef*)-1) && (td != bdf))
            {
                if (td->defaultImage)
                    free(td->defaultImage);
                free(td);
                tableDefs[i] = NULL;
            }
        }
        if (bdf)
            free(bdf);
        if (tableDefs[TABLE_NUM_TMP])
            free(tableDefs[TABLE_NUM_TMP]);
        if (relateData)
            free(relateData);
    }
};

dbdef::dbdef(nsdatabase* pbe, short defType, short mode) : nstable(pbe)
{
    m_dimpl = new dbdimple();
    m_dimpl->deftype = defType;
    m_dimpl->noWriteMode = (m_dimpl->deftype == TYPE_SCHEMA_BDF_NOPRELOAD);
    m_keybuflen = 128;
    m_keybuf = &m_dimpl->keybuf[0];
    setShared();
    setMode((char_td)mode);
}

dbdef::~dbdef()
{
    delete m_dimpl;
    m_dimpl = NULL;
}

short dbdef::tableCount() const
{
    return m_dimpl->tableCount;
}

void* dbdef::relateData() const
{
    return m_dimpl->relateData;
}

short dbdef::openMode() const
{
    return m_dimpl->openMode;
}

int dbdef::version() const
{
    return m_dimpl->version[7] - '0';
};

void dbdef::setVersion(int v)
{
    m_dimpl->version[7] = (char)v;
}

void* dbdef::allocRelateData(int size)
{
    if (m_dimpl->relateData)
        free(m_dimpl->relateData);
    m_dimpl->relateData = malloc(size);
    return m_dimpl->relateData;
}
keylen_td dbdef::writeKeyData()
{
    return 2;
}
void dbdef::moveById(short id)
{
    while (1)
    {
        m_pdata = m_dimpl->bdf;
        m_buflen = m_dimpl->bdfLen;
        memcpy(m_keybuf, &id, 2);
        seek();
        if (m_stat == STATUS_BUFFERTOOSMALL)
            return;
        else
            break;
    }
}

void* dbdef::getBufferPtr(uint_td& size)
{
    size = m_dimpl->bdfLen;
    return m_dimpl->bdf;
}

bool dbdef::setDefaultImage(short tableIndex, const uchar_td* p, ushort_td size)
{
    tabledef* td = m_dimpl->tableDefs[tableIndex];
    if (td)
    {
        if (td->defaultImage)
        {
            free(td->defaultImage);
            td->defaultImage = NULL;
        }
        if (!p || size == 0)
            return true;

        td->defaultImage = malloc(size);
        if (td->defaultImage)
        {
            memcpy(td->defaultImage, p, size);
            return true;
        }
    }
    return false;
}

bool dbdef::addSchemaImage(const tabledef* p, ushort_td size, short& tableIndex)
{
    tableIndex = ++m_dimpl->tableCount;
    initReadAfter(tableIndex, p, size);
    return true;
}

void dbdef::allocDatabuffer()
{
    m_stat = 0;
    if (m_dimpl->bdf == NULL)
    {
        if (!nsdb()->isUseTransactd())
            m_dimpl->bdfLen = BDFMAXBUFFER_BTRV;
        m_dimpl->bdf =
        (tabledef*)malloc(m_dimpl->bdfLen);
        if (m_dimpl->bdf == NULL)
            m_stat = STATUS_CANT_ALLOC_MEMORY;
    }
}

bool dbdef::testTablePtr(tabledef* td)
{
    if (td == NULL)
    {
        m_stat = STATUS_INVALID_TABLE_IDX;
        return false;
    }
    return true;
}

short dbdef::validateTableDef(short tableIndex)
{
    m_stat = STATUS_SUCCESS;
    tabledef* td = tableDefs(tableIndex);
    if (!testTablePtr(td)) return m_stat;

    td->optionFlags.bitA = false; // reset valiable type
    td->optionFlags.bitB = false;
    for (short i = 0; i < td->fieldCount; ++i)
    {
        const fielddef& fd = td->fieldDefs[i];
        short ret = fieldNumByName(tableIndex, fd.name());
        if ((ret != -1) && (ret != i))
        {
            m_stat = STATUS_DUPLICATE_FIELDNAME;
            return m_stat;
        }
        // Check field length.
        uchar_td type = fd.type;
        ushort_td len = (type == ft_mychar) ? fd.charNum() : fd.len;
        // reset update indicator
        const_cast<fielddef&>(fd).enableFlags.bitE = false;
        ret = validLen(type, len);
        if (!ret || !fd.isValidCharNum())
        {
            m_stat = STATUS_INVALID_FIELDLENGTH;
            return m_stat;
        }

        // Note or Lvar type must be the last of fields.
        if ((type == ft_note) || (type == ft_lvar))
        {
            if (i != td->fieldCount - 1)
            {
                m_stat = STATUS_LVAR_NOTE_NOT_LAST;
                return m_stat;
            }
        }

        bool flag =
            (td->flags.bit0 == true) && (i == (short)(td->fieldCount - 1));
        if (flag && (type != ft_myvarbinary) && (type != ft_mywvarbinary) &&
            (type != ft_note) && (type != ft_lvar))
        {
            m_stat = STATUS_INVALID_VARIABLETABLE;
            return m_stat;
        }
        if ((type == ft_myvarchar) || (type == ft_mywvarchar) ||
            (type == ft_myvarbinary) || (type == ft_mywvarbinary) ||
            (type == ft_myfixedbinary))
            td->optionFlags.bitA = true;
        if ((type == ft_myblob) || (type == ft_mytext) || (type == ft_mygeometry)
            || (type == ft_myjson))
            td->optionFlags.bitB = true;
        if (type == ft_myfixedbinary)
            td->optionFlags.bitC = true;
        //force use pad char
        if ((type == ft_mychar) || (type == ft_mywchar))
            td->fieldDefs[i].m_padCharOptions |= USE_PAD_CHAR;

        if (type == ft_mytimestamp && (fd.decimals != 0))
        {
            int dec = (fd.len - 4) * 2;
            if (fd.decimals > dec || fd.decimals < dec -1)
            {
                m_stat = STATUS_INVALID_FIELDLENGTH;
                return m_stat;
            }
        }
        if (type == ft_mytime && (fd.decimals != 0))
        {
            int dec = (fd.len - 3) * 2;
            if (fd.decimals > dec || fd.decimals < dec -1)
            {
                m_stat = STATUS_INVALID_FIELDLENGTH;
                return m_stat;
            }
        }
        if (type == ft_mydatetime && (fd.decimals != 0))
        {
            int dec = (fd.len - 5) * 2;
            if (fd.decimals > dec || fd.decimals < dec -1)
            {
                m_stat = STATUS_INVALID_FIELDLENGTH;
                return m_stat;
            }
        }
    }

    // Check invalid key type
    for (short i = 0; i < td->keyCount; i++)
    {
        for (short j = 0; j < td->keyDefs[i].segmentCount; j++)
        {
            short fnum = td->keyDefs[i].segments[j].fieldNum;
            short ret = isPassKey(td->fieldDefs[fnum].type);
            if (!ret)
            {
                m_stat = STATUS_INVALID_KEYTYPE;
                return m_stat;
            }
        }
    }

    // Chack duplicate table name.
    for (short i = 1; i < m_dimpl->tableCount; i++)
    {
        if ((tableDefs(i)) && (i != tableIndex))
        {
            m_stat = 0;
            if (strcmp(tableDefs(i)->tableNameA(), td->tableNameA()) == 0)
            {
                m_stat = STATUS_DUPPLICATE_KEYVALUE;
                return m_stat;
            }
        }
    }
    if (td->inUse() == 0)
        td->calcReclordlen();
    return m_stat;
}

void dbdef::updateTableDef(short tableIndex, bool forPsqlDdf)
{
    if ((m_stat = validateTableDef(tableIndex)) != 0)
        return;

    tabledef* td = tableDefs(tableIndex);
    if (!testTablePtr(td)) return;

    if (m_dimpl->noWriteMode)
    {
        if (m_dimpl->deftype != TYPE_SCHEMA_BDF_NOPRELOAD)
            m_stat = STATUS_ACCESS_DENIED;
        return;
    }
    if (m_dimpl->deftype == TYPE_SCHEMA_DDF)
        saveDDF(tableIndex, 3, forPsqlDdf);
    else
    {
        moveById(td->id);
        if (m_stat == STATUS_SUCCESS)
        {
            m_pdata = td;
            m_buflen = td->size();
            td->formatVersion = FORMAT_VERSON_CURRENT;
            if(isUseTransactd())
                td->varSize = td->size() - 4;

            update();
            m_pdata = m_dimpl->bdf;
            m_buflen = m_dimpl->bdfLen;
            if (m_stat == STATUS_SUCCESS)
                setDefaultImage(tableIndex, NULL, 0);
        }
    }
}

void dbdef::deleteTable(short tableIndex)
{
    tabledef* td = tableDefs(tableIndex);
    if (!testTablePtr(td)) return;

    m_stat = STATUS_SUCCESS;
    if (m_dimpl->noWriteMode == false)
    {
        if (m_dimpl->deftype == TYPE_SCHEMA_DDF)
            saveDDF(tableIndex, 4);
        else
        {
            moveById(td->id);
            if (m_stat == STATUS_SUCCESS)
            {
                m_pdata = td;
                m_buflen = td->size();
                del();
                m_pdata = m_dimpl->bdf;
                m_buflen = m_dimpl->bdfLen;
            }
        }
    }
    if (m_stat == STATUS_SUCCESS)
    {
        setDefaultImage(tableIndex, NULL, 0);
        free(td);
        m_dimpl->tableDefs[tableIndex] = NULL;
    }
}

void dbdef::renumberFieldNum(short tableIndex, short Index, short op)
{
    int i, j;
    keydef* KeyDef;

    tabledef* td = tableDefs(tableIndex);
    if (!testTablePtr(td)) return;

    for (i = 0; i < td->keyCount; i++)
    {
        KeyDef = &(td->keyDefs[i]);

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

bool dbdef::isUsedField(short tableIndex, short deleteIndex)
{
    int i, j;
    keydef* KeyDef;
    tabledef* td = tableDefs(tableIndex);
    if (!testTablePtr(td)) return false;

    for (i = 0; i < td->keyCount; i++)
    {
        KeyDef = &(td->keyDefs[i]);
        for (j = 0; j < KeyDef->segmentCount; j++)
        {
            if (KeyDef->segments[j].fieldNum == deleteIndex)
                return true;
        }
    }
    return false;
}

void dbdef::deleteField(short tableIndex, short deleteIndex)
{
    m_stat = STATUS_SUCCESS;
    if (isUsedField(tableIndex, deleteIndex) == true)
    {
        m_stat = STATUS_USE_KEYFIELD;
        return;
    }
    renumberFieldNum(tableIndex, deleteIndex, 4);
    tabledef* td = tableDefs(tableIndex);
    if ((deleteIndex == td->fieldCount - 1) && (td->keyCount == 0))
    {
    }
    else
    {
        memmove(&td->fieldDefs[deleteIndex], &td->fieldDefs[deleteIndex + 1],
                td->size() + (char*)td -
                    (char*)&(td->fieldDefs[deleteIndex + 1]));
    }
    td->fieldCount--;
    td->setKeydefsPtr();
    updateTableDef(tableIndex);
}

void dbdef::deleteKey(short tableIndex, short deleteIndex)
{
    m_stat = STATUS_SUCCESS;
    tabledef* td = tableDefs(tableIndex);
    if (!testTablePtr(td)) return ;
    if (deleteIndex != td->keyCount - 1)
    {
        memmove(&td->keyDefs[deleteIndex], &td->keyDefs[deleteIndex + 1],
                td->size() + (char*)td -
                    (char*)&(td->keyDefs[deleteIndex + 1]));
    }
    td->keyCount--;
    if ((td->primaryKeyNum == deleteIndex) ||
        (td->primaryKeyNum > td->keyCount - 1))
        td->primaryKeyNum = -1;
    if ((td->parentKeyNum == deleteIndex) ||
        (td->parentKeyNum > td->keyCount - 1))
        td->parentKeyNum = -1;
    if ((td->replicaKeyNum == deleteIndex) ||
        (td->replicaKeyNum > td->keyCount - 1))
        td->replicaKeyNum = -1;

    updateTableDef(tableIndex);
}

void dbdef::insertTable(tabledef* td)
{
    m_stat = STATUS_SUCCESS;
    if (td->id > TABLE_NUM_TMP)
    {
        m_stat = STATUS_TOO_MANY_TABLES;
        return;
    }
    if (tableDefs(td->id) != NULL)
    {
        m_stat = STATUS_DUPPLICATE_KEYVALUE;
        return;
    }

    if (tableNumByName(td->tableName()) != -1)
    {
        m_stat = STATUS_DUPPLICATE_KEYVALUE;
        return;
    }
    if (td->fieldCount > 512)
    {
        m_stat = STATUS_TOO_MANY_FIELDS;
        return;
    }

    m_dimpl->tableDefs[td->id] =
        (tabledef*)malloc(USHRT_MAX /* sizeof(tabledef) */);
    if (m_dimpl->tableDefs[td->id] == NULL)
    {
        m_stat = STATUS_CANT_ALLOC_MEMORY;
        return;
    }
    if ((td->ddfid == 0) && (m_dimpl->deftype == TYPE_SCHEMA_DDF))
        td->ddfid = getDDFNewTableIndex();
    memcpy(m_dimpl->tableDefs[td->id], td, sizeof(tabledef));
    m_dimpl->tableDefs[td->id]->defaultImage = NULL;
    if (m_dimpl->noWriteMode)
    {
        if (m_dimpl->tableCount < td->id)
            m_dimpl->tableCount = td->id;
        return;
    }
    td->formatVersion = FORMAT_VERSON_CURRENT;
    if (m_dimpl->deftype == TYPE_SCHEMA_DDF)
        saveDDF(td->id, 2);
    else
    {
        memcpy(m_dimpl->bdf, td, sizeof(tabledef));
        m_pdata = m_dimpl->bdf;

        memcpy(m_keybuf, &td->id, 2);
        m_buflen = sizeof(tabledef);
        insert();
        m_pdata = m_dimpl->bdf;
        m_buflen = m_dimpl->bdfLen;
    }
    if (m_stat != 0)
    {
        free(m_dimpl->tableDefs[td->id]);
        m_dimpl->tableDefs[td->id] = NULL;
    }
    else
    {
        if (m_dimpl->tableCount < td->id)
            m_dimpl->tableCount = td->id;
    }
}

bool dbdef::resizeAt(short tableIndex, bool key)
{
    tabledef* td = m_dimpl->tableDefs[tableIndex];
    if (!testTablePtr(td)) return false;

    if (!key && td->m_inUse != 0) return false;

    uint_td addsize;

    if (key == true)
        addsize = sizeof(keydef);
    else
        addsize = sizeof(fielddef);

    uint_td size = td->size() + addsize;

    void* p = malloc(size);
    if (p)
    {
        memcpy(p, td, td->size());
        free(td);
        m_dimpl->tableDefs[tableIndex] = td = (tabledef*)p;
        // init for memcpy
        td->setFielddefsPtr();
        td->setKeydefsPtr();
        return true;
    }
    m_stat = STATUS_CANT_ALLOC_MEMORY;
    return false;
}

keydef* dbdef::insertKey(short tableIndex, short insertIndex)
{
    if (resizeAt(tableIndex, true) == false)
        return NULL;

    tabledef* td = m_dimpl->tableDefs[tableIndex];
    if (insertIndex < tableDefs(tableIndex)->keyCount)
    {
        memmove(&td->keyDefs[insertIndex + 1],
                &td->keyDefs[insertIndex],
                td->size() + (char*)td - (char*)&(td->keyDefs[insertIndex]));
    }
    td->keyCount++;
    memset(&(td->keyDefs[insertIndex]), 0, sizeof(keydef));

    if ((!m_dimpl->noWriteMode) && (m_dimpl->deftype != TYPE_SCHEMA_DDF))
        updateTableDef(tableIndex);
    else
        td->calcReclordlen();
    return &(td->keyDefs[insertIndex]);
}

fielddef* dbdef::insertField(short tableIndex, short insertIndex)
{
    if (resizeAt(tableIndex, false) == false)
        return NULL;

    tabledef* td = tableDefs(tableIndex);
    if ((insertIndex < td->fieldCount) || (td->keyCount > 0))
    {

        memmove(&(td->fieldDefs[insertIndex + 1]),
                &(td->fieldDefs[insertIndex]),
                td->size() + (char*)td -
                    (char*)&(td->fieldDefs[insertIndex]));
    }
    td->fieldCount++;
    td->setKeydefsPtr();
    renumberFieldNum(tableIndex, insertIndex, 2);
    memset(&(td->fieldDefs[insertIndex]), 0, sizeof(fielddef));
    fielddef* fd = &(td->fieldDefs[insertIndex]);
    fd->setCharsetIndex(td->charsetIndex);
    fd->setSchemaCodePage(td->schemaCodePage);
    fd->setPadCharSettings(false, true);
    return fd;
}

inline fielddef_t_my& dbdef::convert(fielddef_t_my& fd_my,
                                     const fielddef_t_pv& fd_pv)
{
    memset(&fd_my, 0, sizeof(fd_my));
    strcpy(fd_my.m_name, fd_pv.m_name);
    void* dest = (char*)&fd_my + sizeof(fd_my.m_name);
    const void* src = (char*)&fd_pv + sizeof(fd_pv.m_name);
    size_t size = sizeof(fd_my) - sizeof(fd_my.m_name);
    memcpy(dest, src, size);
    return fd_my;
}

inline int fixVariableLenBug(bool isUseTransactd, const tabledef* src, size_t size)
{
    if (isUseTransactd)
    { // A Transactd server format changed to nosupport FIXED_PLUS_VARIABLELEN
        if (src->varSize + 4 == (int)size)
        { // This is a chagned server
            if (src->preAlloc &&
                ((src->preAlloc % 512 == 0) || (src->fieldCount > 255) ||
                 (src->keyCount > 127) || (src->fieldCount == 0)))
            {
                // memmove(((char*)src)+4, ((char*)src)+6, src->pageSize-2);
                memmove(((char*)src) + 2, ((char*)src) + 4, src->pageSize);
                size -= 2;
            }
        }
    }
    return (int)size;
}

size_t getNewVersionSize(const tabledef* src)
{
    return src->fieldCount * sizeof(fielddef) + sizeof(tabledef) +
           src->keyCount * sizeof(keydef) + 1;
}

void dbdef::tableDefCopy(tabledef* dest, const tabledef* src, size_t size)
{
    if (src->formatVersion == FORMAT_VERSON_BTRV_DEF)
    {
        size_t len = 0;
        memcpy(dest, src, sizeof(tabledef));
        len += sizeof(tabledef);
        fielddef_t_my* fd = (fielddef_t_my*)dest->setFielddefsPtr();
        fielddef_t_pv* src_fd = (fielddef_t_pv*)const_cast<tabledef*>(src)->setFielddefsPtr();
        for (int i = 0; i < dest->fieldCount; ++i)
        {
            convert(*fd, *src_fd);
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
    return &m_dimpl->tableDefs[index];
}

tabledef* dbdef::initReadAfter(short tableIndex, const tabledef* data, uint_td datalen)
{
    m_datalen = fixVariableLenBug(isUseTransactd(), data, datalen);
    size_t size = getNewVersionSize(data);
    tabledef* td = (tabledef*)malloc(size + 10);
    if (td == NULL)
    {
        m_stat = STATUS_CANT_ALLOC_MEMORY;
        return NULL;
    }
    m_dimpl->tableDefs[tableIndex] = td;
    tableDefCopy(td, data, min<size_t>(m_datalen, size));
    td->setFielddefsPtr();
    td->setKeydefsPtr();
    td->autoIncExSpace = ((database*)nsdb())->defaultAutoIncSpace();
    td->convertToUtf8Schema();
    //Fix:Bug of maxRecordLen is mistake value saved, recalculate maxRecordLen.
    td->calcReclordlen();
    td->optionFlags.bitC = (td->fieldDefs[td->fieldCount -1].type == ft_myfixedbinary);
    td->id = tableIndex;
    td->defaultImage = NULL;

    //Check a page size
    if (!isUseTransactd())
    {
        if (td->pageSize < 512 || td->pageSize > 4096 || td->pageSize % 512)
            td->pageSize = 2048;
    }
    return td;
}

#pragma warn -8004
tabledef* dbdef::tableDefs(int index)
{
    if (index > TABLE_NUM_TMP)
        return NULL;
    tabledef* td = m_dimpl->tableDefs[index];

    if (index == TABLE_NUM_TMP)
        return td;
    if (m_dimpl->tableCount < index)
        return NULL;
    if (td == NULL)
    {
        if (m_dimpl->deftype == TYPE_SCHEMA_DDF)
            return NULL;
        while (1)
        {
            m_pdata = m_dimpl->bdf;
            m_buflen = m_dimpl->bdfLen;
            m_dimpl->bdf->id = (short)index;
            memcpy(m_keybuf, &m_dimpl->bdf->id, 2);
            seek();
            if (m_stat == STATUS_BUFFERTOOSMALL)
                return NULL;
            else
                break;
        }
        if (m_stat && (m_stat != STATUS_NOT_FOUND_TI))
        {
            return NULL;
        }
        if (m_stat)
        {
            td = (tabledef*)-1;
            m_stat = 0;
            return NULL;
        }
        td = initReadAfter(index, (tabledef*)m_pdata, m_datalen);
    }
    else if (td == (tabledef*)-1)
        return NULL;

    return td;
}
#pragma warn .8004

void dbdef::doOpen(const _TCHAR* uri, char_td mode, const _TCHAR* onerName)
{
    assert((m_dimpl->deftype != TYPE_SCHEMA_BDF_NOPRELOAD));
    m_dimpl->noWriteMode = true;

    if (m_dimpl->deftype == TYPE_SCHEMA_DDF)
    {

        openDdf(((database*)nsdb())->rootDir(), mode, onerName); // DDF
        if (mode != TD_OPEN_READONLY)
            m_dimpl->noWriteMode = false;
        return;
    }

    // version check
    m_dimpl->version[7] = '0';
    nstable::doOpen(uri, mode, m_dimpl->version);
    if (m_stat == STATUS_INVALID_OWNERNAME)
    {
        while (m_stat == STATUS_INVALID_OWNERNAME)
        {
            m_dimpl->version[7]++;
            nstable::doOpen(uri, mode, m_dimpl->version);
            if (m_dimpl->version[7] > '9')
                return;
        }
    }
    if (m_stat) return;

    if (m_dimpl->bdf == NULL)
        allocDatabuffer();
    if (m_stat) return;
    
    m_pdata = m_dimpl->bdf;
    m_buflen = m_dimpl->bdfLen;
    m_keynum = 0;
    seekLast();
    if (m_stat == STATUS_SUCCESS)
        m_dimpl->tableCount = m_dimpl->bdf->id;
    if (m_stat == STATUS_EOF)
        m_stat = STATUS_SUCCESS;
    if (mode != TD_OPEN_READONLY)
        m_dimpl->noWriteMode = false;
    m_dimpl->openMode = mode;
}

void dbdef::doClose()
{
    nstable::doClose();
    if (m_dimpl->bdf)
    {
        free(m_dimpl->bdf);
        m_dimpl->bdf = NULL;
    }
    m_dimpl->openMode = 1;
}

void dbdef::create(const _TCHAR* fullpath)
{
    if (m_dimpl->deftype == TYPE_SCHEMA_DDF)
    {
        createDDF(fullpath);
        return;
    }
    fileSpec* fs;

    fs = (fileSpec*)malloc(512);
    memset(fs, 0x00, 512);
    fs->recLen = sizeof(tabledef);
    fs->pageSize = 4096;
    fs->indexCount = 1;
    fs->fileFlag.all = 1; // valiable length
    fs->preAlloc = 10;
    fs->keySpecs[0].keyPos = 1; // id
    fs->keySpecs[0].keyLen = 2; // short
    fs->keySpecs[0].keyFlag.all = 258; // changeable  and extended key type
    fs->keySpecs[0].keyType = 1; // Integer
    nsdb()->createTable(fs, 412, fullpath, 0);
    free(fs);

    //TODO Create operation include set owner name.
    if (nsdb()->stat() == 0)
    { // set owner name
        ownerNameSetter* bao = new ownerNameSetter(nsdb());
        bao->open(fullpath);
        bao->setOwnerName((const _TCHAR*)BDFFORMAT_VERSION);
        bao->release();
    }
    m_stat = nsdb()->stat();
}

void dbdef::drop()
{
    nsdb()->dropTable(uri());
    m_stat = nsdb()->stat();
}

void dbdef::getFileSpec(fileSpec* fs, short tableIndex)
{
    short i, j, k = 0;
    tabledef* td = tableDefs(tableIndex);
    fs->recLen = td->fixedRecordLen;
    fs->pageSize = td->pageSize;
    fs->indexCount = td->keyCount;
    fs->recCount = 0;
    fs->fileFlag.all = td->flags.all;
    fs->reserve1[0] = 0;
    fs->reserve1[1] = 0;
    fs->preAlloc = td->preAlloc;

    for (i = 0; i < td->keyCount; i++)
    {
        keydef* kd = &(td->keyDefs[i]);
        for (j = 0; j < kd->segmentCount; j++)
        {
            short fnum = kd->segments[j].fieldNum;
            keySpec* ks = &(fs->keySpecs[k]);
            ks->keyPos = td->fieldDefs[fnum].pos + 1;
            ks->keyLen = td->fieldDefs[fnum].len;
            ks->keyFlag.all = kd->segments[j].flags.all;
            ks->keyCount = 0;
            ks->keyType = td->fieldDefs[fnum].type;

            if ((ks->keyType == ft_autoinc) && (kd->segmentCount > 1))
                ks->keyType = 1;
            if (ks->keyFlag.bit3 == true)
                ks->nullValue = td->fieldDefs[fnum].nullValue;
            else
                ks->nullValue = 0;
            ks->reserve2[0] = 0;
            ks->reserve2[1] = 0;
            if (fs->fileFlag.bitA == true)
                ks->keyNo = kd->keyNumber;
            else
                ks->keyNo = 0;
            ;
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
    tabledef* td = tableDefs(OldIndex);
    if (m_dimpl->noWriteMode)
    {
        td->id = NewIndex;
        m_dimpl->tableDefs[NewIndex] = td;
        m_dimpl->tableDefs[OldIndex] = NULL;
        if (NewIndex > m_dimpl->tableCount)
            m_dimpl->tableCount = NewIndex;
        return;
    }
    moveById(td->id);
    if (m_stat == STATUS_SUCCESS)
    {
        
        m_pdata = td;
        m_buflen = td->size();
        td->id = NewIndex;
        update();
        m_pdata = m_dimpl->bdf;
        m_buflen = m_dimpl->bdfLen;

        if (m_stat == STATUS_SUCCESS)
        {
            m_dimpl->tableDefs[NewIndex] = td;
            m_dimpl->tableDefs[OldIndex] = NULL;
            if (NewIndex > m_dimpl->tableCount)
                m_dimpl->tableCount = NewIndex;
        }
        else
            td->id = OldIndex;
    }
}

short dbdef::fieldNumByViewNum(short tableIndex, short index)
{
    tabledef* td = tableDefs(tableIndex);
    if (td)
    {
        for (short i = 0; i < td->fieldCount; i++)
        {
            if ((td->fieldDefs[i].viewNum == index) &&
                (td->fieldDefs[i].enableFlags.bit0))
                return i;
        }
    }
    return -1;
}

short dbdef::findKeynumByFieldNum(short tableIndex, short index)
{
    tabledef* td = tableDefs(tableIndex);
    if (td)
        return td->findKeynumByFieldNum(index);
    return -1;
}

short dbdef::tableNumByName(const _TCHAR* tableName)
{
    char buf[74];
    const char* p = NULL;
    for (short i = 1; i <= m_dimpl->tableCount; i++)
    {
        tabledef* td = tableDefs(i);
        if (td)
        {
            if (!p)
            	p = td->toChar(buf, tableName, 74);
            if (strcmp(td->tableNameA(), p) == 0)
                return i;
        }
    }
    return -1;
}

short dbdef::fieldNumByName(short tableIndex, const _TCHAR* name)
{
    tabledef* td = tableDefs(tableIndex);
    if (td)
        return td->fieldNumByName(name);
    return -1;
}

uint_td dbdef::fieldValidLength(eFieldQuery query, uchar_td FieldType)
{ // return MaxLen or MinLen or DefaultLen or Dec of field type

    uint_td maxlen = 0;
    uint_td minlen = 0;
    uint_td defaultlen = 0;
    int dec = -1;
    switch (FieldType)
    {
    case ft_string:
    case ft_mychar:
        minlen = 1;
        maxlen = 255;
        defaultlen = 1;
        break;
    case ft_lstring:
    case ft_zstring:
        minlen = 2;
        maxlen = 255;
        defaultlen = 2;
        break;
    case ft_myvarchar:
    case ft_myvarbinary:
        minlen = 1;
        maxlen = 65535;
        defaultlen = 2;
        break;
    case ft_myblob:
    case ft_mytext:
        minlen = 9;
        maxlen = 12;
        defaultlen = 9;
        break;
    case ft_mygeometry:
    case ft_myjson:
        minlen = 12;
        maxlen = 12;
        defaultlen = 12;
        break;
    case ft_mywchar:
    case ft_wstring:
        minlen = 2;
        maxlen = 255;
        defaultlen = 2;
        break;
    case ft_wzstring:
        minlen = 3;
        maxlen = 255;
        defaultlen = 2;
        break;
    case ft_mywvarchar:
    case ft_mywvarbinary:
        minlen = 1;
        maxlen = 65535;
        defaultlen = 3;
        break;
    case ft_myfixedbinary:
        minlen = 3;
        maxlen = 60000;
        defaultlen = 1024;
        break;
    case ft_myyear:
        minlen = 1;
        maxlen = 1;
        defaultlen = 1;
        break;
    case ft_mydate:
        minlen = 3;
        maxlen = 3;
        defaultlen = 3;
        break;
    case ft_integer:
        minlen = 1;
        maxlen = 8;
        defaultlen = 2;
        break;
    case ft_bfloat:
    case ft_float:
        minlen = 4;
        maxlen = 8;
        defaultlen = 4;
        break;
    case ft_date:
    case ft_time:
        minlen = 4;
        maxlen = 4;
        defaultlen = 4;
        break;
    case ft_mytime:
        minlen = 3;
        maxlen = 6;
        defaultlen = 4;
        break;
    case ft_money:
    case ft_decimal:
        minlen = 1;
        maxlen = 10;
        defaultlen = 6;
        if (FieldType == 5)
            dec = 0;
        else
            dec = 2;
        break;
    case ft_logical:
        minlen = 1;
        maxlen = 2;
        defaultlen = 2;
        break;
    case ft_numericsts:
    case ft_numericsa:
    case ft_numeric:
        minlen = 1;
        maxlen = 15;
        defaultlen = 6;
        dec = 0;
        if (FieldType == 17)
            minlen = 2;
        break;
    case ft_mydecimal:
        minlen = 1;
        maxlen = 32;
        defaultlen = 5;
        dec = 0;
        break;
    case ft_note:
        minlen = 2;
        maxlen = 32761;
        defaultlen = 2;
        break;
    case ft_lvar:
        minlen = 5;
        maxlen = 32761;
        defaultlen = 5;
        break;
    case ft_uinteger:
        minlen = 1;
        maxlen = 8;
        defaultlen = 2;
        break;
    case ft_autoIncUnsigned:
    case ft_autoinc:
        minlen = 2;
        maxlen = 8;
        defaultlen = 4;
        break;
    case ft_bit:
    case ft_set:
        minlen = 1;
        maxlen = 8;
        defaultlen = 1;
        break;
    case ft_enum:
        minlen = 1;
        maxlen = 2;
        defaultlen = 1;
        break;
    case ft_timestamp:
    case ft_currency:
        minlen = 8;
        maxlen = 8;
        defaultlen = 8;
        break;
    case ft_mytimestamp:
        minlen = 4;
        maxlen = 7;
        defaultlen = 7;
        break;
    case ft_mydatetime:
        minlen = 5;
        maxlen = 8;
        defaultlen = 5;
        break;
    case ft_nullindicator:
        minlen = 0;
        maxlen = 0;
        defaultlen = 0;
    }
    switch (query)
    {
    case eMinlen:
        return minlen;
    case eMaxlen:
        return maxlen;
    case eDefaultlen:
        return defaultlen;
    case eDecimals:
        return dec;
    }
    return 0;
}

bool dbdef::validLen(uchar_td FieldType, uint_td FieldLen)
{ // return valid length of field by field type.
    if ((FieldLen >= fieldValidLength(eMinlen, FieldType)) &&
        (FieldLen <= fieldValidLength(eMaxlen, FieldType)))
    {
        if ((FieldType == ft_integer) || (FieldType == ft_uinteger))
        {
            if ((FieldLen == 1) || (FieldLen == 2) || (FieldLen == 3) || (FieldLen == 4) ||
                (FieldLen == 8))
                return true;
            else
                return false;
        }
        else if ((FieldType == ft_autoinc) || (FieldType == ft_autoIncUnsigned))
        {
            if ((FieldLen == 2) || (FieldLen == 3) || (FieldLen == 4) || (FieldLen == 8))
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
    if (FieldType == ft_myfixedbinary)
        return false;
    if (FieldType == ft_bit)
        return false;
    //if (FieldType > ft_numericsts)
    //    return false;
    if (FieldType == ft_note)
        return false;
    if (FieldType == ft_lvar)
        return false;
    return true;
}

void dbdef::autoMakeSchema(bool nouseNullkey)
{
    m_keynum = (int)nouseNullkey;
    if (database::compatibleMode() & database::CMP_MODE_BINFD_DEFAULT_STR)
        m_keynum += 2; // binary field defaut string

    tdap(TD_AUTOMEKE_SCHEMA);
}

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

void dbdef::saveDDF(short tableIndex, short opration, bool forPsqlDdf)
{
    ushort_td chOpen = 0;
    short Mode = 0;
    ushort_td tbid = tableIndex;
    ushort_td fdid;
    ushort_td keyid;
    ushort_td segid;
    ushort_td pos;
    const _TCHAR* own[3];

    m_stat = STATUS_SUCCESS;
    tabledef* TableDef;
    fielddef* FieldDef;
    keydef* KeyDef;

    fileDDF* tb = new fileDDF(nsdb());
    fieldDDF* fd = new fieldDDF(nsdb());
    indexDDF* id = new indexDDF(nsdb());

    if ((tb) && (fd) && (id))
    {
        if (m_dimpl->userName[0] != 0x00)
        {
            own[0] = (const _TCHAR*)m_dimpl->userName;
            own[1] = (const _TCHAR*)m_dimpl->userName;
            own[2] = (const _TCHAR*)m_dimpl->userName;
        }
        else
        {
            own[0] = ow0;
            own[1] = ow1;
            own[2] = ow2;
        }
        tb->open(((database*)nsdb())->rootDir(), (char_td)Mode,
                 (const _TCHAR*)own[0]);
        chOpen += tb->stat();
        fd->open(((database*)nsdb())->rootDir(), (char_td)Mode,
                 (const _TCHAR*)own[1]);
        chOpen += fd->stat();
        id->open(((database*)nsdb())->rootDir(), (char_td)Mode,
                 (const _TCHAR*)own[2]);
        chOpen += id->stat();
        if (chOpen)
        {
            tb->release();
            fd->release();
            id->release();
            m_stat = STATUS_INVALID_OWNERNAME;
            return;
        }

        // erase all records
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
                tb->flag = 0; // PSQL are reading flags from table files.
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
                        id->fieldid =
                            TableDef
                                ->fieldDefs[KeyDef->segments[segid].fieldNum]
                                .ddfid;
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
            _tcscpy(m_dimpl->userName, OwnerName);
        }
        else
        {
            own[0] = (_TCHAR*)ow0;
            own[1] = (_TCHAR*)ow1;
            own[2] = (_TCHAR*)ow2;
            m_dimpl->userName[0] = 0x00;
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
                    for (FieldIndex = 0;
                         FieldIndex < tableDefs(tbid)->fieldCount; FieldIndex++)
                    {
                        if (tableDefs(tbid)->fieldDefs[FieldIndex].pos >
                            fd->pos)
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
                        if (id->segmentnum < 8)
                        {
                            if (KeyDef->segmentCount < id->segmentnum + 1)
                                KeyDef->segmentCount =
                                    (uchar_td)(id->segmentnum + 1);
                            KeyDef->segments[id->segmentnum].fieldNum =
                                (uchar_td)FieldIndex;
                            KeyDef->segments[id->segmentnum].flags.all = id->flag;
                        }
                        id->seekNext();
                    }
                }
                /*else
                {
                }*/
                fd->seekNext();
            }

            tableDefs(tbid)->calcReclordlen();

            tableDefs(tbid)->fixedRecordLen = tableDefs(tbid)->recordlen();
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
                    sprintf_s((char*)FieldDef->nameA(), FIELD_NAME_SIZE,
                              "NIS%d", i + 1);

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
    for (i = 0; i <= m_dimpl->tableCount; i++)
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
    if (m_dimpl->maxid == 0)
    {
        for (i = 0; i <= m_dimpl->tableCount; i++)
        {
            if (tableDefs(i) != NULL)
            {
                for (j = 0; j < tableDefs(i)->fieldCount; j++)
                {
                    if (tableDefs(i)->fieldDefs[j].ddfid > m_dimpl->maxid)
                        m_dimpl->maxid = tableDefs(i)->fieldDefs[j].ddfid;
                }
            }
        }
    }
    m_dimpl->maxid++;
    return (ushort_td)m_dimpl->maxid;
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

void dbdef::pushBackup(short tableIndex)
{
    int blen;
    tabledef* td = tableDefs(tableIndex);
    if (!td)
    {
        m_stat = STATUS_INVALID_TABLE_IDX;
        return;
    }
    blen = td->size();
    if (!tableDefs(TABLE_NUM_TMP))
        m_dimpl->tableDefs[TABLE_NUM_TMP] = (tabledef*)malloc(blen);
    else
        m_dimpl->tableDefs[TABLE_NUM_TMP] =
            (tabledef*)realloc(tableDefs(TABLE_NUM_TMP), blen);

    tabledef* tdt = m_dimpl->tableDefs[TABLE_NUM_TMP];
    if (!tdt)
    {
        m_stat = STATUS_CANT_ALLOC_MEMORY;
        return;
    }
    setFieldsCharsetIndex(td);
    memcpy(tdt, td, blen);
    tdt->m_inUse = 0;
    tdt->setFielddefsPtr();
    tdt->setKeydefsPtr();

}

bool dbdef::compAsBackup(short tableIndex)
{
    if (m_dimpl->tableCount < tableIndex)
        return false;

    tabledef* tds = tableDefs(tableIndex);
    tabledef* tdo = tableDefs(TABLE_NUM_TMP);

    if (tds->size() != tdo->size())
        return true;

    //tabledef
    bool isSame = (*tds == *tdo);
    if (!isSame) return true;

    for (int i = 0; i < tds->fieldCount; i++)
    {
        isSame = (tds->fieldDefs[i] == tdo->fieldDefs[i]);
        if (!isSame) return true;
    }
    for (int i = 0; i < tds->keyCount; i++)
    {
        isSame = (tds->keyDefs[i] == tdo->keyDefs[i]);
        if (!isSame) return true;
    }
    return false;
}

void dbdef::popBackup(short tableIndex)
{
    tabledef* tdt = tableDefs(TABLE_NUM_TMP);
    tabledef* td = m_dimpl->tableDefs[tableIndex];
    if (tdt && td)
    {
        m_dimpl->tableDefs[tableIndex] = td = (tabledef*)realloc(td, tdt->size());
        memcpy(td, tdt, tdt->size());
        td->setFielddefsPtr();
        td->setKeydefsPtr();
        updateTableDef(tableIndex);
    }
}

void dbdef::reopen(char_td mode)
{
    close();
    open(uri(), mode, NULL);
}

void dbdef::synchronizeSeverSchema(short tableIndex)
{
    if (isUseTransactd() == false)
    {
        m_stat = STATUS_NOSUPPORT_OP;
        return ;
    }

    tabledef* tdold = tableDefs(tableIndex);
    if (!tdold)
    {
        m_stat = STATUS_INVALID_TABLE_IDX;
        return;
    }
    void* tmp = m_keybuf;

    _TCHAR dummyUrl[MAX_PATH] = _T("tdap://srv/db?dbfile=");
    _tcscat(dummyUrl, tdold->fileName());

    char tmpName[MAX_PATH] = { 0x00 };
    const char* p = nsdatabase::toServerUri(tmpName, MAX_PATH, dummyUrl, true);
    m_keybuf = (void*)p;
    m_keylen = (keylen_td)strlen(p) + 1;
    m_pdata = m_dimpl->bdf;
    m_buflen = m_datalen = m_dimpl->bdfLen;
    m_dimpl->bdf->id = tableIndex;
    m_keynum = SC_SUBOP_TABLEDEF;
    tdap((ushort_td)TD_GET_SCHEMA);
    m_keybuf = tmp;
    m_keynum = 0;
    if (m_stat == STATUS_SUCCESS)
    {
        if (m_datalen == 0)
        {
            m_stat = STATUS_NOSUPPORT_OP;
            m_keybuf = tmp;
            return;
        }
        tabledef* td = (tabledef*)m_pdata;
        td->m_mysqlNullMode = tdold->isMysqlNullMode();
        td = initReadAfter(tableIndex, td, m_datalen);
        if (td)
        {
            m_stat = td->synchronize(tdold);
            delete tdold;
        }
    }
}

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs
