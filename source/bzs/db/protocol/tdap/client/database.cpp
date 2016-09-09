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
#include "database.h"
#include "table.h"
#include "dbDef.h"
#include <limits.h>
#include <sys/stat.h>
#include <stdio.h>
#include <vector>
#include "stringConverter.h"
#include <bzs/db/protocol/tdap/uri.h>

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
    static int m_compatibleMode;
    dbdef* dbDef;
    void* optionalData;
    _TCHAR rootDir[MAX_PATH];
    deleteRecordFn m_deleteRecordFn;
    copyDataFn m_copyDataFn;
    uint_td openBuflen;
    btrVersions vers;
    struct
    {
        bool isOpened : 1;
        bool isTableReadOnly : 1;
        bool lockReadOnly : 1;
        bool autoSchemaUseNullkey : 1;
        bool noPreloadSchema : 1;
        bool createExistNoCheck : 1;
    };
    dbimple()
        : dbDef(NULL), optionalData(NULL), m_deleteRecordFn(NULL),
          m_copyDataFn(NULL), openBuflen(0), isOpened(false), isTableReadOnly(false),
          lockReadOnly(false), autoSchemaUseNullkey(false), noPreloadSchema(false),
          createExistNoCheck(false)
    {
        rootDir[0] = 0x00;
        memset(&vers, 0 , sizeof(btrVersions));
    }

    dbimple& operator=(const dbimple& rt)
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
            autoSchemaUseNullkey = rt.autoSchemaUseNullkey;
            noPreloadSchema = rt.noPreloadSchema;
            vers = rt.vers;
        }
        return *this;
    }

    const btrVersion& mysqlVer() const { return vers.versions[VER_IDX_DB_SERVER];}
    const btrVersion& pluginVer() const { return vers.versions[VER_IDX_PLUGIN];}
    
};

int dbimple::m_compatibleMode = database::CMP_MODE_MYSQL_NULL;

void database::destroy(database* db)
{
    delete db;
}

database::database() : nsdatabase()
{
    m_impl = new dbimple();
}

database::~database()
{
    doClose();
    delete m_impl;
}

void database::release()
{
    if (refCount() == 1)
        nsdatabase::release();
    else
    {
        nsdatabase::release();
        if ((refCount() == 1) && m_impl->dbDef &&
            (m_impl->dbDef->nsdb() == this))
            nsdatabase::release();
    }
}

database* database::createAssociate()
{
    if (isOpened() && !enableTrn())
    {
        database* db = database::create();
        //Copy id
        memcpy(db->clientID() + 14,  clientID() + 14, 2);
        db->setAssociate();
        db->m_btrcallid = m_btrcallid;
        //Set same connection and connection->addref() in tdclc
        m_stat = m_btrcallid(TD_CONNECT, NULL, NULL, NULL, (void*)clientID(),
                         16, LG_SUBOP_ASSOCIATE, db->clientID());
        if (m_stat == 0)
            return db;
        database::destroy(db);
    }
    return NULL;
}

dbdef* database::dbDef() const
{
    return m_impl->dbDef;
}

const _TCHAR* database::rootDir() const
{
    return m_impl->rootDir;
}

void database::setRootDir(const _TCHAR* directory)
{
    setDir(directory);
}

void* database::optionalData() const
{
    return m_impl->optionalData;
}

void database::setOptionalData(void* v)
{
    m_impl->optionalData = v;
}

bool database::tableReadOnly() const
{
    return m_impl->isTableReadOnly;
}

const deleteRecordFn database::onDeleteRecord() const
{
    return m_impl->m_deleteRecordFn;
}

void database::setOnDeleteRecord(const deleteRecordFn v)
{
    m_impl->m_deleteRecordFn = v;
}

const copyDataFn database::onCopyData() const
{
    return m_impl->m_copyDataFn;
}

void database::setOnCopyData(const copyDataFn v)
{
    m_impl->m_copyDataFn = v;
}

void database::setLockReadOnly(bool v)
{
    m_impl->lockReadOnly = v;
}

bool database::isOpened() const
{
    return m_impl->isOpened;
}

char_td database::mode() const
{
    assert(m_impl->dbDef);
    return m_impl->dbDef->mode();
}

bool database::autoSchemaUseNullkey() const
{
    return m_impl->autoSchemaUseNullkey;
}

void database::setAutoSchemaUseNullkey(bool v)
{
    m_impl->autoSchemaUseNullkey = v;
}

void database::create(const _TCHAR* uri, short type)
{
    bool dbdefCreated = false;
    short stat;
    if (!m_impl->dbDef)
    {
        m_impl->dbDef = new dbdef(this, type); // Create TabelDef here.
        dbdefCreated = true;
    }
    bool isTransactd = isTransactdUri(uri);
    _TCHAR buf[MAX_PATH];
    schemaTable(uri, buf, MAX_PATH);
    if (buf[0] || !isTransactd)
    {
        m_impl->dbDef->create(uri);
        stat = m_impl->dbDef->stat();
    }
    else
    {
        if (isTransactd)
        {
            if (setUseTransactd() == false)
                stat = ERROR_LOAD_CLIBRARY;
            else
            {
                _TCHAR uri_tmp[MAX_PATH];
                stripParam(uri, buf, MAX_PATH);
                _tcscpy_s(uri_tmp, MAX_PATH, buf);
                _tcscat(uri_tmp, _T("?dbfile="));
                _tcscat(uri_tmp, TRANSACTD_SCHEMANAME);
                passwd(uri, buf, MAX_PATH);
                if (buf[0])
                {
                    _tcscat(uri_tmp, _T("&pwd="));
                    _tcscat(uri_tmp, buf);
                }

                _TCHAR posblk[128] = { 0x00 };
                const char* p = toServerUri((char*)buf, MAX_PATH, uri_tmp, true);
                uint_td len = 0;
                stat = tdapEx(TD_CREATETABLE, posblk, NULL, &len,
                     (void*)p, (uchar_td)strlen(p), CR_SUBOP_CREATE_DBONLY);
            }
        }
        else
            stat = STATUS_NOSUPPORT_OP;
    }
    if (dbdefCreated)
        doClose();
    m_stat = stat;
}

void database::drop(const _TCHAR* uri)
{
    _TCHAR path[MAX_PATH];
    _TCHAR* fileNames[255] = {NULL};

    if (((uri == NULL) || (uri[0] == 0x00)) && checkOpened())
    {
        m_stat = STATUS_DB_YET_OPEN;
        return;
    }

    if (uri && uri[0])
    {
        _TCHAR pwd[MAX_PATH];
        passwd(uri, pwd, MAX_PATH);
        stripParam(uri, path, MAX_PATH);
        _tcscat(path, _T("?dbfile="));
        _tcscat(path, TRANSACTD_SCHEMANAME);
        if (pwd[0])
        {
            _tcscat(path, _T("&pwd="));
            _tcscat(path, pwd);
        }
        nsdatabase::dropTable(path);
        if (m_stat)  return;
        nsdatabase::reset();
        return;
    }

    int count = 0;
    for (int i = 1; i <= m_impl->dbDef->tableCount(); i++)
    {
        tabledef* td = m_impl->dbDef->tableDefs(i);
        if (td)
        {
            _stprintf_s(path, MAX_PATH, _T("%s") PSEPARATOR _T("%s"),
                        rootDir(), td->fileName());
            size_t len = _tcslen(path);
            _TCHAR* s(new _TCHAR[len + 1]);
            _tcscpy(s, path);
            fileNames[count++] = s;
        }
    }
    size_t len = _tcslen(m_impl->dbDef->uri());
    if (len)
    {
        _TCHAR* s(new _TCHAR[len + 1]);
        _tcscpy(s, m_impl->dbDef->uri());
   
        fileNames[count++] = s;
        BTRCALLID_PTR ptr = m_btrcallid;
        if (isUseTransactd())
            m_btrcallid = NULL;
        close();
        m_btrcallid = ptr;
        for (int i = 0; i < count; i++)
        {
            nsdatabase::dropTable(fileNames[i]);
            if (m_stat && (m_stat == STATUS_TABLE_NOTOPEN))
                break;
        }

        for (int i = 0; i < count; i++)
            delete [] fileNames[i];
    }else
    {
        _TCHAR s[MAX_PATH];
        _tcscpy(s, rootDir());
        _tcscat(s, TRANSACTD_SCHEMANAME);
        nsdatabase::dropTable(s);
    }
    if (m_stat)
        return;
    nsdatabase::reset();
}

void database::dropTable(const _TCHAR* TableName)
{
    if (checkOpened()) return;
    
    _TCHAR FullPath[MAX_PATH];
    _tcscpy(FullPath, rootDir());
    _tcscat(FullPath, PSEPARATOR);
    short index = m_impl->dbDef->tableNumByName(TableName);
    if (index == -1)
    {
        m_stat = STATUS_TABLENAME_NOTFOUND;
        return;
    }
    tabledef* td = m_impl->dbDef->tableDefs(index);
    _tcscat(FullPath, td->fileName());

    int i = 0;
    while (1)
    {
        nsdatabase::dropTable(FullPath);
        if (m_stat != STATUS_LOCK_ERROR)
            break;
        if (++i == 10) 
            break;
        Sleep(50);
    }
    if (m_stat == 0)
    {
        m_impl->dbDef->setDefaultImage(index, NULL, 0);
        td->m_inUse = 0;
    }
}

void database::setDir(const _TCHAR* directory)
{
    _tcscpy(m_impl->rootDir, directory);
}

database& database::operator=(const database& rt)
{
    if (&rt != this)
    {
        nsdatabase::operator=(rt);
        m_impl->dbimple::operator=(*(rt.m_impl));
        if (rt.m_impl->dbDef)
            rt.m_impl->dbDef->addref();
        addref();
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

void database::onCopyDataInternal(table* tb, int recordCount, int count,
                                  bool& cancel)
{
    if (m_impl->m_copyDataFn)
        m_impl->m_copyDataFn(this, tb, recordCount, count, cancel);
}

void database::setTableReadOnly(bool value)
{
    if (!m_impl->lockReadOnly)
        m_impl->isTableReadOnly = value;
}

void database::doOpen(const _TCHAR* uri, short type, short mode,
                      const _TCHAR* ownername)
{
    m_stat = STATUS_SUCCESS;
    m_impl->dbDef->open(uri, (char_td)mode, ownername);

    if ((m_stat == STATUS_SUCCESS) &&
        (m_impl->dbDef->m_stat == STATUS_INVALID_OWNERNAME) && (type == 0))
        m_impl->dbDef->m_stat = STATUS_DIFFERENT_DBVERSION;

    m_stat = m_impl->dbDef->m_stat;
    m_impl->isOpened = (m_stat == STATUS_SUCCESS); // important
}

bool isConnected(const uchar_td* cid)
{
    const void* p = *((void**)cid);
    return p != 0;
}

bool database::open(const _TCHAR* _uri, short type, short mode,
                    const _TCHAR* dir, const _TCHAR* ownername)
{

    _TCHAR buf[MAX_PATH+50];
    m_stat = STATUS_SUCCESS;
    m_impl->noPreloadSchema = false;
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
            nstable::getDirURI(uri(), buf);
            setDir(buf);
            if (m_stat)
                return false;
        }

        if (type == TYPE_SCHEMA_BDF)
        {// BDF
            if (isTransactdUri(_uri))
            {
                _TCHAR name[128];
                schemaTable(_uri, name, 128);
                m_impl->noPreloadSchema = (name[0] == 0x00);
                if (m_impl->noPreloadSchema)
                    type = TYPE_SCHEMA_BDF_NOPRELOAD;
                dbname(_uri, name, 128);
                if (name[0] == 0x00)
                {
                    m_stat = ERROR_NO_DATABASE;
                    return false;
                }
            }
        }
        if (!m_impl->dbDef)
            m_impl->dbDef = new dbdef(this, type);

        if (m_impl->noPreloadSchema)
        {
            if ((compatibleMode() & CMP_MODE_MYSQL_NULL) == 0)
                m_stat = STATUS_INVALID_NULLMODE;
            else if (isAssociate() || isConnected(clientID()) || connect(_uri))
            {
                m_impl->dbDef->allocDatabuffer();
                m_stat = m_impl->dbDef->stat();
                if (m_stat == 0)
                {
                    m_impl->isOpened = true;
                    m_impl->autoSchemaUseNullkey = true;
                    getBtrVersion(&m_impl->vers);
                    return true;
                }
            }
        }
        else
        {
            doOpen(_uri, type, mode, ownername);
            m_impl->isOpened = (m_stat == STATUS_SUCCESS); // important
            if ((m_stat == STATUS_TABLE_NOTOPEN) && isUseTransactd() &&
                _tcsstr(_uri, TRANSACTD_SCHEMANAME))
            {
                // Specified TRANSACTD_SCHEMANAME and no table
                // Auto make schema.
                create(_uri, TYPE_SCHEMA_BDF);
                if (m_stat == STATUS_SUCCESS)
                {
                    //Open mode force normal
                    doOpen(_uri, type, TD_OPEN_NORMAL, ownername);
                    if (m_stat == STATUS_SUCCESS)
                    {
                        m_impl->dbDef->autoMakeSchema(autoSchemaUseNullkey());
                        m_impl->dbDef->close();
                        doOpen(_uri, type, mode, ownername);
                    }
                }
            }
        }
    }
    if (m_impl->isOpened && onOpenAfter())
    {
        if (isUseTransactd())
            getBtrVersion(&m_impl->vers);
        if (m_stat == 0)
            return true;
    }
    m_impl->isOpened = false;
    m_impl->dbDef->close();
    m_impl->dbDef->release();
    nsdatabase::release();
    m_impl->dbDef = NULL;
    return false;
}

short database::aclReload()
{
    if (!m_impl->isOpened)
        return STATUS_DB_YET_OPEN;
    _TCHAR buf[MAX_PATH];
    const _TCHAR* p = dbname(rootDir(), buf, MAX_PATH);
    if (_tcscmp(p, _T("mysql")) != 0)
        return m_stat = STATUS_DB_YET_OPEN; 
    _TCHAR posblk[128] = { 0x00 };
    uint_td buflen = 0;
    return m_stat = tdapEx(TD_ACL_RELOAD, posblk, NULL, &buflen, 0, 0, 0);
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
    char tmp[128] = { 0x00 };
    char* buf = getContinuousList(inclideRepfile);
    uint_td buflen = (uint_td)strlen(buf) + 1;
    m_stat = tdap(TD_BACKUPMODE, tmp, buf, &buflen, 0, 0, IsEnd);
    free(buf);
    return m_stat;
}

void database::doClose()
{
    m_stat = STATUS_SUCCESS;
    resetSnapshot();
    if (m_impl->dbDef)
    {
        dbdef* def = m_impl->dbDef;
        m_impl->dbDef = NULL;
        def->release();
        nsdatabase::reset();
    }
    m_impl->dbDef = NULL;
    m_impl->isOpened = false;
    m_impl->rootDir[0] = 0x00;
    m_impl->lockReadOnly = false;
}

void database::close(bool withDropDefaultSchema)
{
    bool flag = (m_impl->dbDef != NULL);
    if (m_impl && m_impl->dbDef)
    {
        m_impl->dbDef->setKeyNum(withDropDefaultSchema ? CR_SUBOP_DROP : 0);
        if (!isUseTransactd() && withDropDefaultSchema)
        {
            _TCHAR uri[MAX_PATH];
            _tcscpy(uri, m_impl->dbDef->uri());
            doClose();
            nsdatabase::dropTable(uri);
            nsdatabase::release();
            return;
        }
    }
    doClose();
    if (flag)
        nsdatabase::release();
}

bool database::doReopenDatabaseSchema()
{
    if (m_impl->dbDef && m_impl->dbDef->isOpen())
    {
        doReconnect(m_impl->dbDef);
        return (m_stat == 0);
    }
    return true;
}

_TCHAR* database::getTableUri(_TCHAR* buf, short tableIndex)
{
    m_stat = STATUS_SUCCESS;
    if ((m_impl->dbDef) && (m_impl->isOpened))
    {
        tabledef* td = m_impl->dbDef->tableDefs(tableIndex);
        if (td)
            return getTableUri(buf, td->fileName());
    }
    m_stat = STATUS_DB_YET_OPEN;
    return NULL;
}

_TCHAR* database::getTableUri(_TCHAR* buf, const _TCHAR* filename)
{
    if (_tcsstr(filename, PSEPARATOR) == NULL)
        _stprintf_s(buf, MAX_PATH, _T("%s") PSEPARATOR _T("%s"), m_impl->rootDir, filename);
    else
        _tcscpy(buf, filename);
    return buf;
}

table* database::createTableObject()
{
    return new table(this);
}

void* database::getExtendBufferForOpen(uint_td& size)
{
    void* p =  m_impl->dbDef->getBufferPtr(size);
    if (!p)
        size = 0;
    else if (m_impl->openBuflen)
        size = m_impl->openBuflen;
    return p;
}

bool database::defaultImageCopy(const void* data, short& tableIndex)
{
    struct extraImage
    {
        ushort_td bookmarklen;
        ushort_td recordSize;
    };
    struct schemaImage
    {
        ushort_td schemaSize;
        tabledef td;
    };

    extraImage* p = (extraImage*)data;
    size_t size = p->recordSize;
    bool ret = false;
    if (size)
    {
        if (tableIndex > 0)
            ret = m_impl->dbDef->setDefaultImage(tableIndex, (uchar_td*)(++p), (ushort_td)size);
        else
        {
            schemaImage* si = (schemaImage*)(((uchar_td*)p) + sizeof(ushort_td) * 2 + size);
            if (si->schemaSize == 0) return false;
            ret = m_impl->dbDef->addSchemaImage(&si->td, si->schemaSize, tableIndex);
            if (ret)
                ret = m_impl->dbDef->setDefaultImage(tableIndex, (uchar_td*)++p, (ushort_td)size);
        }
    }
    return ret;
}

short database::checkOpened()
{
    if ((!m_impl->dbDef) || (!m_impl->isOpened))
        return STATUS_DB_YET_OPEN;
    if (m_impl->rootDir[0] == 0x00)
        return  STATUS_DB_YET_OPEN;

    return 0;
}

struct openTablePrams
{
    tabledef* td;
    _TCHAR uri[MAX_PATH];
    short mode;
    bool mysqlnull;
    bool regularDir;
    bool autoCreate;
    bool getSchema;
    bool getDefaultImage;
    bool useInMariadb;
    bool isTransactd;
    
    openTablePrams(bool autoCreate_p) : autoCreate(autoCreate_p),
        getSchema(false), getDefaultImage(false)
    {
        uri[0] = 0x00;
    }

    short setParam(tabledef* def, short v,  bool isTransactd, bool readOnly, bool noPreloadSchema)
    {
        this->isTransactd = isTransactd;
        mode = v;
        td = def;
        mysqlnull = false;
        if (isTransactd)
        {
            if (IS_MODE_MYSQL_NULL(mode))
                mysqlnull = true;
            else if (database::compatibleMode() & database::CMP_MODE_MYSQL_NULL)
            {
                mode += TD_OPEN_MASK_MYSQL_NULL;
                mysqlnull = true;
            }
            if (mysqlnull && (!td || td->defaultImage == NULL))
            {
                mode += TD_OPEN_MASK_GETDEFAULTIMAGE;
                getDefaultImage = true;
            }
            if (!td && noPreloadSchema)
            {
                mode += TD_OPEN_MASK_GETSHCHEMA;
                getSchema = true;
                autoCreate = false;
                if (database::compatibleMode() & database::CMP_MODE_BINFD_DEFAULT_STR)
                    mode += TD_OPEN_MASK_BIN_STR;
            }
        }
        if (readOnly && !IS_MODE_READONLY(mode))
            mode += TD_OPEN_READONLY;

        if (td)
        {
            if (td->inUse())
            {
                if ((td->isMysqlNullMode() == true && mysqlnull == false)
                    || (td->isMysqlNullMode() == false && mysqlnull))
                    return STATUS_INVALID_NULLMODE;
            }else
                td->setMysqlNullMode(mysqlnull);
        }
        return 0;
    }

    void setPath(const _TCHAR* path, database* db)
    {
        regularDir = false;
        if (td)
        {
            if ((path == NULL) || (path[0] == 0x00))
            {
                if (_tcsstr(td->fileName(), PSEPARATOR) == NULL)
                {
                    db->getTableUri(uri, td->id);
                    regularDir = true;
                }
                else
                    _tcscpy(uri, td->fileName());
            }
            else
                _tcscpy(uri, path);
        }else if (path)
        {
            if (_tcsstr(path, _T("://")))
                 _tcscpy(uri, path); // another database
            else
                db->getTableUri(uri, path);
        }
    }
};

table* database::doOpenTable(openTablePrams* pm, const _TCHAR* ownerName)
{

    tabledef* td = pm->td;

    if (pm->isTransactd)
    {
        if (m_stat) return NULL;
        if ((database::compatibleMode() & database::CMP_MODE_MYSQL_NULL) &&
                m_impl->pluginVer().majorVersion < 3)
        {
            m_stat = STATUS_INVALID_NULLMODE;
            return NULL;
        }
        if (td && td->inUse() == 0)
        {
            td->m_useInMariadb = m_impl->mysqlVer().isMariaDB();
            td->m_srvMajorVer = (uchar_td)m_impl->mysqlVer().majorVersion;
            td->m_srvMinorVer = (uchar_td)m_impl->mysqlVer().minorVersion;
            td->calcReclordlen();
        }
    }

    short tableIndex = td ? td->id : -1;

    if (!m_impl->noPreloadSchema && !td)
    {
        m_stat = STATUS_TABLENAME_NOTFOUND;
        return NULL;
    }
    table* tb = createTableObject();
    if (pm->mysqlnull)
    {
        m_impl->openBuflen = 0;
        if (td)
            m_impl->openBuflen = td->recordlenServer() + (sizeof(ushort_td) * 2);
    }
    bool tableCreated = false;
    tb->open(pm->uri, (char_td)pm->mode, ownerName);
    if ((tb->m_stat == STATUS_TABLE_NOTOPEN) ||
        (tb->m_stat == ERROR_NOSPECIFY_TABLE))
    {
        if (pm->autoCreate)
        {
            createTable(tableIndex, pm->uri);
            if (m_stat != STATUS_SUCCESS)
            {
                tb->release();
                return NULL;
            }
            else
            {
                tb->open(pm->uri, (char_td)pm->mode);
                if ((ownerName) && (ownerName[0]))
                    tb->setOwnerName(ownerName);
                tableCreated = true;
            }
        }
        else
        {
            m_stat = tb->m_stat;
            tb->release();
            return NULL;
        }
    }
    if (tb->m_stat == 0)
    {
        // register tabledef to dbdef
        uint_td size;
        bool ret = true;
        if (pm->getDefaultImage || pm->getSchema)
            ret = defaultImageCopy(m_impl->dbDef->getBufferPtr(size), tableIndex);
        if (ret)
            tb->init(m_impl->dbDef->tableDefPtr(tableIndex), tableIndex, pm->regularDir, pm->mysqlnull); 
        else
            m_stat = STATUS_PROGRAM_ERROR;
    }
    if ((m_stat != 0) || (tb->m_stat != 0) ||
        !onTableOpened(tb, tableIndex, pm->mode, tableCreated))
    {
        m_stat = tb->m_stat;
        tb->release();
        return NULL;
    }
    return tb;
}

table* database::openTable(const _TCHAR* tableName, short mode, bool autoCreate,
                           const _TCHAR* ownerName, const _TCHAR* path)
{
    m_stat = checkOpened();
    if (m_stat) return NULL;

    openTablePrams pm(autoCreate);
 
    short tableIndex = m_impl->dbDef->tableNumByName(tableName);
    tabledef* td = NULL;
    if (tableIndex != -1)
        td = m_impl->dbDef->tableDefs(tableIndex);
    m_stat = pm.setParam(td, mode, isUseTransactd(), m_impl->isTableReadOnly, m_impl->noPreloadSchema);
    if (m_stat == 0)
    {
        if (td)
            pm.setPath(path, this);
        else
            pm.setPath(tableName, this);
        return doOpenTable(&pm, ownerName);
    }
    return NULL;
}

table* database::openTable(short tableIndex, short mode, bool autoCreate,
                           const _TCHAR* ownerName, const _TCHAR* path)
{
    m_stat = checkOpened();
    if (m_stat) return NULL;

    openTablePrams pm(autoCreate);
    tabledef* td = m_impl->dbDef->tableDefs(tableIndex);
    if (td)
    {
        if (pm.setParam(td, mode, isUseTransactd(), m_impl->isTableReadOnly, m_impl->noPreloadSchema) == 0)
        {
            pm.setPath(path, this);
            return doOpenTable(&pm, ownerName);
        }
        return NULL;
    }
    m_stat = STATUS_INVALID_TABLE_IDX;
    return NULL;
}

char* database::getSqlStringForCreateTable(const _TCHAR* tableName, char* retbuf, uint_td* size)
{
    retbuf[0] = 0x00;
    if (checkOpened()) return retbuf;

    short tableIndex = m_impl->dbDef->tableNumByName(tableName);
    tabledef* td = NULL;
    if (tableIndex != -1)
        td = m_impl->dbDef->tableDefs(tableIndex);

    if (!td || td->fieldCount == 0)
    {
        m_stat = STATUS_INVALID_FIELD_OFFSET;
        return retbuf;
    }
    if (td->size() > (int)*size)
    {
        m_stat = STATUS_BUFFERTOOSMALL;
        return retbuf;
    }
    
    _TCHAR uri[MAX_PATH];
    getTableUri(uri, td->id);

    if (!isTransactdUri(uri))
    {
        m_stat = STATUS_NOSUPPORT_OP;
        return retbuf;
    }

    if (setUseTransactd() == false)
    {
        m_stat = STATUS_NOSUPPORT_OP;
        return retbuf;
    }
    memcpy(retbuf, td, td->size());
    td = (tabledef*)retbuf;
    td->setFielddefsPtr();
    td->setKeydefsPtr();

    td->calcReclordlen(true);
    char buf2[MAX_PATH] = { 0x00 };
    _TCHAR posblk[128] = { 0x00 };

    const char* p = toServerUri(buf2, MAX_PATH, uri, isUseTransactd());
    
    m_stat = tdap(TD_TABLE_INFO, posblk, td, size, (void*)p, 
        (uchar_td)strlen(p), ST_SUB_GETSQL_BY_TABLEDEF);
    return retbuf;
}

bool database::createTable(const char* utf8Sql)
{
    if (checkOpened()) return false;
    if (isUseTransactd())
    {
        if (setUseTransactd() == false)
            m_stat = ERROR_LOAD_CLIBRARY;
        else
        {
            char buf2[MAX_PATH] = { 0x00 };
            _TCHAR posblk[128] = { 0x00 };
            const char* p = toServerUri(buf2, MAX_PATH, rootDir(), true);
            uint_td len = (uint_td)strlen(utf8Sql);
            m_stat = tdapEx(TD_CREATETABLE, posblk, (void*)utf8Sql, &len,
                 (void*)p, (uchar_td)strlen(p),
                 (m_impl->createExistNoCheck) ?
                 CR_SUBOP_BY_SQL : CR_SUBOP_BY_SQL_NOCKECK);
        }
    }
    else
        m_stat = STATUS_NOSUPPORT_OP;
    return (m_stat == 0);
}

bool database::createTable(short fileNum, const _TCHAR* uri)
{
    tabledef* td = m_impl->dbDef->tableDefs(fileNum);
    if (!td || td->fieldCount == 0)
    {
        m_stat = STATUS_INVALID_FIELD_OFFSET;
        return false;
    }
    if (uri && isTransactdUri(uri))
    {
        if (setUseTransactd() == false)
            return false;

        td->calcReclordlen(true);
        char buf2[MAX_PATH] = { 0x00 };
        _TCHAR posblk[128] = { 0x00 };

        const char* p = toServerUri(buf2, MAX_PATH, uri, isUseTransactd());

        m_stat = tdapEx(TD_CREATETABLE, posblk, td,
            &m_impl->dbDef->m_datalen, (void*)p, (uchar_td)strlen(p),
            (m_impl->createExistNoCheck) ?
                 CR_SUBOP_BY_TABLEDEF : CR_SUBOP_BY_TABLEDEF_NOCKECK);
    }
    else
    {
        const _TCHAR* buf;
        fileSpec* fs = (fileSpec*)malloc(1024);
        if (fs == NULL)
        {
            m_stat = STATUS_CANT_ALLOC_MEMORY;
            return false;
        }
        memset(fs, 0, 1024);
        m_impl->dbDef->getFileSpec(fs, fileNum);
        if (uri)
            buf = uri;
        else
            buf = td->fileName();
        nsdatabase::createTable(fs, 1024, buf,
            (m_impl->createExistNoCheck) ?
                CR_SUBOP_BY_FILESPEC : CR_SUBOP_BY_FILESPEC_NOCKECK);
        free(fs);
    }
    return (m_stat == 0);
}

short database::assignSchemaData(const dbdef* src)
{
    beginTrn();
    int Count = 1;

    dbdef* defDest = dbDef();
    int recordCount = src->tableCount();

    for (int i = 1; i <= src->tableCount(); i++)
    {
        const tabledef* td = const_cast<dbdef*>(src)->tableDefs(i);
        if (td)
        {
            tabledef tdtmp = *td;
            tdtmp.m_inUse = false;
            tdtmp.fieldCount = 0;
            tdtmp.keyCount = 0;
            defDest->insertTable(&tdtmp);
            if (defDest->stat())
                break;
            for (int j = 0; j < td->fieldCount; ++j)
            {
                fielddef& fd = td->fieldDefs[j];
                *defDest->insertField(td->id, j) = fd;
            }
            for (int j = 0; j < td->keyCount; ++j)
            {
                keydef& kd = td->keyDefs[j];
                *defDest->insertKey(td->id, j) = kd;
            }
            defDest->updateTableDef(td->id);
            if (defDest->stat())
                break;
        }
        bool Cancel = false;
        onCopyDataInternal(NULL, recordCount, Count, Cancel);
        if (Cancel)
            return -1;
        Count++;
    }

    if ((nstable::tdapErr((HWND)NULL, src->stat()) == 0) &&
        (defDest->stat() == 0))
    {
        endTrn();
        return 0;
    }
    abortTrn();
    if (nstable::tdapErr((HWND)NULL, src->stat()))
        return src->stat();

    return defDest->stat();
}

struct fieldChnageInfo
{
    fieldChnageInfo() : fieldnum(-1), changed(0) {}

    short fieldnum;
    bool changed;
};

void makeChangeInfo(const tabledef* ddef, const tabledef* sdef,
                    fieldChnageInfo* fci, bool& hasBlob)
{
    hasBlob = false;
    for (short i = 0; i < sdef->fieldCount; i++)
    {
        fielddef& fds = sdef->fieldDefs[i];
        for (short j = 0; j < ddef->fieldCount; j++)
        {
            fielddef& fdd = ddef->fieldDefs[j];
            if (strcmp(fdd.nameA(), fds.nameA()) == 0)
            {
                fci[i].fieldnum = j;
                if (!hasBlob && fdd.isBlob()) hasBlob = true;
                // mydatetime and mytimestmp mytime is no same binary format as different
                // mysql versions
                if ((fds.type != fdd.type)         ||
                    (fds.len != fdd.len)           ||
                    (fds.isBlob() || fdd.isBlob()) ||
                    (fds.type== ft_mytime || fdd.type== ft_mytime) ||
                    (fds.type== ft_mytimestamp || fdd.type== ft_mytimestamp)||
                    (fds.type== ft_mydatetime || fdd.type== ft_mydatetime))
                    fci[i].changed = true;
                break;
            }
            else
                fci[i].fieldnum = -1;
        }
    }
}

inline void database::copyEachFieldData(table* dest, table* src, fieldChnageInfo* fci)
{
    const tabledef* ddef = dest->tableDef();
    const tabledef* sdef = src->tableDef();

    for (int i = 0; i < sdef->fieldCount; i++)
    {
        int dindex = fci[i].fieldnum;
        fielddef& fds = sdef->fieldDefs[i];
        fielddef& fdd = ddef->fieldDefs[dindex];

        if (dindex != -1)
        {
            dest->setFVNull(dindex, src->getFVNull(i));
            // src variable len and last field;
            if ((fci[i].changed == false) || (fdd.type == ft_myfixedbinary))
            {
                int len = fds.len;
                if (fds.len > fdd.len)
                    len = fdd.len;
                /* Move 2 byte automaticaly by tabledef::pack() and unpack()
                   when field type is ft_myfixedbinary  */
                memcpy(dest->fieldPtr(dindex), src->fieldPtr(i), len);
            }
            else
            {
                if (fdd.maxVarDatalen() && fds.maxVarDatalen())
                {
                    uint_td size;
                    uint_td maxlen = fdd.maxVarDatalen();
                    const void* data = src->getFVbin(i, size);
                    if (maxlen < size)
                        size = maxlen;
                    dest->setFV(dindex, data, size);
                }
                else
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
    bookmark_td bm;
    src->stepLast();
    while (src->stat() == STATUS_SUCCESS)
    {
        bm = src->bookmark();
        if (src->stat() != STATUS_SUCCESS)
            break;
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

#define MOVE_TYPE_KEY    0
#define MOVE_TYPE_NONKEY -1
#define MOVE_TYPE_MULTI  -2

inline void moveNextRecord(table* src, short keyNum)
{
    if (keyNum >= MOVE_TYPE_KEY)
        src->seekNext();
    else if (keyNum == MOVE_TYPE_NONKEY)
        src->stepNext();
    else
        src->findNext();
}

inline char_td findUniqueKeynum(table* src)
{
    const tabledef* td =  src->tableDef();
    if (td->parentKeyNum != 0xff) return td->parentKeyNum;
    for (int i=0;i<td->keyCount;++i)
    {
        const keydef* kd = &td->keyDefs[i];
        if (kd->segments[0].flags.kf_duplicatable == false)
            return i;
    }
    return -1;
}

inline void moveFirstRecord(table* src, short keyNum)
{
    if (keyNum >= MOVE_TYPE_KEY)
    {
        src->setKeyNum((char_td)keyNum);
        src->seekFirst();
    }
    else if (keyNum == MOVE_TYPE_NONKEY)
        src->stepFirst();
    else
    {
        query q;
        q.all().bookmarkAlso(false).reject(0)
            .limit(src->isUseTransactd() ? 500 : 20);
        src->setQuery(&q);
        src->setKeyNum(findUniqueKeynum(src));
        src->findFirst();
    }
}

/* Copy from src to dest table.
 *  Copy as same field name.
 *  If turbo then copy use memcpy and offset dest of first address.
 *  if a src field is variable size binary, that dest field needs to be variable
 *  size binary.
 *  if src and dest fields are different type ,then a text copy is used.
 *  Bulkinsert use default, But not use it when dest table has blob(s).
 */
#pragma warn -8004

struct smartBulkInsert
{
    smartBulkInsert(table* tb, bool hasBlob) : m_tb(tb), m_hasBlob(hasBlob)
    {
        if (m_tb->isUseTransactd() && !m_hasBlob)
            m_tb->beginBulkInsert(BULKBUFSIZE*10);
    }

    short commit()
    {
        short ret = 0;
        if (m_tb->isUseTransactd() && !m_hasBlob)
            ret = m_tb->commitBulkInsert(false);
        m_tb = NULL;
        return ret;
    }

    ~smartBulkInsert()
    {
        if (m_tb && m_tb->isUseTransactd() && !m_hasBlob)
            m_tb->abortBulkInsert();
    }
private:
    table* m_tb;
    bool m_hasBlob;

};

short database::copyTableData(table* dest, table* src, bool turbo,
                              short keyNum, int maxSkip)
{
    src->setKeyNum((char_td)keyNum);
    dest->setKeyNum(-1);
    const tabledef* ddef = dest->tableDef();
    const tabledef* sdef = src->tableDef();
    ushort_td ins_rows = 0;
    bool repData = (_tcsstr(ddef->fileName(), _T("rep.dat"))) ? true : false;
    int skipCount = 0, count = 0;
    int recordCount = src->recordCount();
    fieldChnageInfo fci[256];
    bool hasBlob = false;
    makeChangeInfo(ddef, sdef, fci, hasBlob);
    moveFirstRecord(src, keyNum);

    {// smartBulkInsert commit scope
        smartBulkInsert smi(dest, hasBlob);
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
                if (dest->m_buflen  < src->datalen())
                    return STATUS_CANT_ALLOC_MEMORY;

                memcpy((char*)dest->m_pdata, src->data(), src->datalen());
            }
            else
                copyEachFieldData(dest, src, fci);

            if (repData)
            {
                dest->m_datalen = src->m_datalen;
                dest->tdap(TD_REC_INSERT);
            }
            else
                ins_rows += dest->insert(true);

            if (dest->stat() != STATUS_SUCCESS)
            {
                if (skipCount != maxSkip)
                {
                    if (dest->stat() == STATUS_INVALID_VALLEN)
                        skipCount++;
                    else if (dest->stat() == STATUS_DUPPLICATE_KEYVALUE)
                        skipCount++;
                    else
                        return dest->stat();
                }else
                    return dest->stat();
            }
            else
                count++;
            bool cancel = false;
            onCopyDataInternal(dest, recordCount, count, cancel);
            if (cancel)
                return -1;

            moveNextRecord(src, keyNum);
        }
        if (dest->stat() == 0) smi.commit();
        if (dest->stat() != STATUS_SUCCESS)
            return dest->stat();
    }
    if ((skipCount) && (maxSkip == -1))
    {
        bool cancel = false;
        onCopyDataInternal(dest, -1, count, cancel);
        if (cancel)
            return -1;
    }

    if (src->stat() == 9)
        return 0;
    return src->stat();
}
#pragma warn .8004

void database::doConvertTable(short TableIndex, bool Turbo,
                              const _TCHAR* OwnerName)
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

    tabledef* td = m_impl->dbDef->tableDefs(TABLE_NUM_TMP);
    td->setFielddefsPtr();
    td->setKeydefsPtr();
    short id = td->id;
    td->id = TABLE_NUM_TMP;
    short mode = TD_OPEN_EXCLUSIVE +
                    (isUseTransactd() ? TD_OPEN_MASK_MYSQL_NULL: 0);

    src = openTable(TABLE_NUM_TMP, mode, false, OwnerName);
    td->id = id;
    if (!src)
        return;
    td = m_impl->dbDef->tableDefs(TableIndex);
    short len = td->recordlen();

    td->preAlloc =
        (ushort_td)(src->recordCount() / td->pageSize / len);
    td->flags.bit2 = true;

    _tcscpy(szTempPath, getTableUri(buf, TableIndex));
    _TCHAR* p = _tcsstr(szTempPath, _T("dbfile="));
    if (p == 0)
        p = szTempPath;
    else
        p += 7;

    _TCHAR* p2 = _tcschr(p, _T('.'));
    if (p2 == 0)
        p2 = p + _tcslen(p);
    *p2 = 0x00;

    _tcscat(szTempPath, _T("_conv_dest_tmp"));

    createTable(TableIndex, szTempPath);
    dest = openTable(TableIndex, mode, true, NULL, szTempPath);
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
                *pireod = 0x00;
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

void database::convertTable(short tableIndex, bool turbo,
                            const _TCHAR* ownername)
{
    doConvertTable(tableIndex, turbo, ownername);
}

bool database::existsTableFile(short TableIndex, const _TCHAR* OwnerName)
{

    if (TableIndex == TABLE_NUM_TMP)
    {
        tabledef* td = m_impl->dbDef->tableDefs(TABLE_NUM_TMP);
        if (!td)
        {
            m_stat = STATUS_INVALID_TABLE_IDX;
            return false;
        }
        td->setFielddefsPtr();
        td->setKeydefsPtr();
    }
    table* bao = openTable(TableIndex, TD_OPEN_READONLY, false, OwnerName);
    bool ret = false;
    if (m_stat == STATUS_TABLE_NOTOPEN)
        ret = false;
    else if (m_stat == STATUS_INVALID_OWNERNAME)
        ret = true;
    else if (m_stat == STATUS_SUCCESS)
        ret = true;
    if (bao)
        bao->release();
    m_stat = 0;
    return ret;
}

void database::setCompatibleMode(int mode)
{
    dbimple::m_compatibleMode = mode;
}

int database::compatibleMode()
{
    return  dbimple::m_compatibleMode;
}

} // namespace client
} // namespace btrv
} // namespace protocol
} // namespace db
} // namespace bzs
