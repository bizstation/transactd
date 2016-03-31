/*=================================================================
   Copyright (C) 2013-2016 BizStation Corp All rights reserved.

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
#include "connMgr.h"
#include "database.h"

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

static const _TCHAR* SYSVAR_NAME[TD_VAR_SIZE] =
{
    _T("database_version"),
    _T("transactd_version"),
    _T("address"),
    _T("port"),
    _T("hostcheck_username"),
    _T("max_tcp_connections"),
    _T("table_name_lowercase"),
    _T("pool_threads"),
    _T("tcp_server_type"),
    _T("lock_wait_timeout"),
    _T("transaction_isolation"),
    _T("auth_type"),
    _T("pipe_comm_sharemem_size"),
    _T("max_pipe_connections"),
    _T("use_piped_local"),
    _T("hs_port"),
    _T("use_handlersocket"),
    _T("timestamp_always)")
};

static const _TCHAR* SLAVE_STATUS_NAME[SLAVE_STATUS_DEFAULT_SIZE] =
{
    _T("Slave_IO_State"),
    _T("Master_Host"),
    _T("Master_User"),
    _T("Master_Port"),
    _T("Connect_Retry"),
    _T("Master_Log_File"),
    _T("Read_Master_Log_Pos"),
    _T("Relay_Log_File"),
    _T("Relay_Log_Pos"),
    _T("Relay_Master_Log_File"),
    _T("Slave_IO_Running"),
    _T("Slave_SQL_Running"),
    _T("Replicate_Do_DB"),
    _T("Replicate_Ignore_DB"),
    _T("Replicate_Do_Table"),
    _T("Replicate_Ignore_Table"),
    _T("Replicate_Wild_Do_Table"),
    _T("Replicate_Wild_Ignore_Table"),
    _T("Last_Errno"),
    _T("Last_Error"),
    _T("Skip_Counter"),
    _T("Exec_Master_Log_Pos"),
    _T("Relay_Log_Space"),
    _T("Until_Condition"),
    _T("Until_Log_File"),
    _T("Until_Log_Pos"),
    _T("Master_SSL_Allowed"),
    _T("Master_SSL_CA_File"),
    _T("Master_SSL_CA_Path"),
    _T("Master_SSL_Cert"),
    _T("Master_SSL_Cipher"),
    _T("Master_SSL_Key"),
    _T("Seconds_Behind_Master"),
    _T("Master_SSL_Verify_Server_Cert"),
    _T("Last_IO_Errno"),
    _T("Last_IO_Error"),
    _T("Last_SQL_Errno"),
    _T("Last_SQL_Error"),
    _T("Replicate_Ignore_Server_Ids"),
    _T("Master_Server_Id" ),
};

#pragma pack(push, 1)
pragma_pack1
struct oldRrecord
{
    __int64 conId;                      // 8 byte
    unsigned int id;                    // 4 byte
    unsigned short db;                  // 2 byte
    short type;                         // 2 byte
    char name[CON_REC_VALUE_SIZE];      // 67 byte
    union
    {
        char status;                    // 1 byte
        struct
        {
            char inTransaction : 1;
            char inSnapshot : 1;
            char openNormal : 1;
            char openReadOnly : 1;
            char openEx : 1;
            char openReadOnlyEx : 1;
            char dummy : 2;
        };
    };
    unsigned int readCount;             // 4 byte
    unsigned int updCount;              // 4 byte
    unsigned int delCount;              // 4 byte    
    unsigned int insCount;              // 4 byte 

};                                      // 32 + 68 = 100
#pragma pack(pop)
pragma_pop

connMgr::connMgr(database* db) : nstable(db)
{
    m_db = db;
    m_keybuf = &m_params[0];
    m_params[0] = 0;
    m_params[1] = 0;
    m_keylen = sizeof(m_params);
}

connMgr::~connMgr() {}

database* connMgr::db() const
{
    return m_db;
}

void connMgr::convertFromOldFormat(bool isInUseTable)
{
    if (m_datalen >= sizeof(oldRrecord))
    {
        oldRrecord* rec = (oldRrecord*)m_pdata;
        oldRrecord* end = rec + (m_datalen / sizeof(oldRrecord));
        int i = 0;
        while (rec != end)
        {
            oldRrecord tmp = *rec;
            m_records[i].conId =  tmp.conId;
            m_records[i].id =  tmp.id;
            m_records[i].db =  tmp.db;
            m_records[i].type =  tmp.type;
            strncpy(m_records[i].name, tmp.name, CON_REC_VALUE_SIZE);
            if (isInUseTable)
            {
                m_records[i].readCount =  tmp.readCount;
                m_records[i].updCount =  tmp.updCount;
                m_records[i].delCount =  tmp.delCount;
                m_records[i].insCount =  tmp.insCount;
            }
            ++rec;
            ++i;
        }
        m_datalen =  sizeof(connMgr::record) * i;
    }
}

bool connMgr::connect(const _TCHAR* uri)
{
    bool ret = m_db->connect(uri, true);
    m_stat = m_db->stat();
    if (m_stat == 0)
    {
        m_uri = uri;
        btrVersions vs;
        m_db->getBtrVersion(&vs);
        m_pluginVer = vs.versions[VER_IDX_PLUGIN];
    }
    return ret;
}

void connMgr::disconnect()
{
    if (m_uri != _T(""))
    {
        m_db->disconnect(m_uri.c_str());
        m_stat = m_db->stat();
        if (m_stat == 0)
            m_uri = _T("");
    }
}

void connMgr::allocBuffer()
{
    m_records.resize(64000 / sizeof(connMgr::record));

    m_datalen = m_buflen =
        (uint_td)(m_records.size() * sizeof(connMgr::record));
    m_pdata = (void*)&m_records[0];
    memset(m_pdata, 0, m_datalen);
    setIsOpen(true);
}

const connMgr::records& connMgr::getRecords(bool isInUseTable)
{
    tdap(TD_STASTISTICS);
    if (m_stat == 0 && *((int*)m_keybuf) != sizeof(connMgr::record))
        convertFromOldFormat(isInUseTable);
    if (m_stat == 0)
        m_records.resize(m_datalen / sizeof(connMgr::record));
    else
        m_records.resize(0);
    return m_records;
}

const connMgr::records& connMgr::databases()
{
    m_keynum = TD_STSTCS_DATABASE_LIST;
    allocBuffer();
    return getRecords();
}

const connMgr::records& connMgr::doDefinedTables(const _TCHAR* dbname, int type)
{
    m_keynum = type;
    allocBuffer();
    char tmp[128];
#ifdef _UNICODE
    WideCharToMultiByte(CP_UTF8, 0, dbname,-1, tmp, 128, NULL, NULL);
#else
    strcpy_s(tmp, 128, dbname);
#endif
    m_keybuf = tmp;
    m_keylen = 128;
    getRecords();
    m_keybuf = &m_params[0];
    m_keylen = sizeof(m_params);
    return m_records;
}

const connMgr::records& connMgr::tables(const _TCHAR* dbname)
{
    if ((m_pluginVer.majorVersion >= 3) && (m_pluginVer.minorVersion >= 2))
        return doDefinedTables(dbname, TD_STSTCS_TABLE_LIST);
    m_stat = STATUS_NOSUPPORT_OP;
    m_records.resize(0);
    return m_records;
}

const connMgr::records& connMgr::views(const _TCHAR* dbname)
{
    if ((m_pluginVer.majorVersion >= 3) && (m_pluginVer.minorVersion >= 2))
        return doDefinedTables(dbname, TD_STSTCS_VIEW_LIST);
    m_stat = STATUS_NOSUPPORT_OP;
    m_records.resize(0);
    return m_records;
}

const connMgr::records& connMgr::schemaTables(const _TCHAR* dbname)
{
    return doDefinedTables(dbname, TD_STSTCS_SCHEMA_TABLE_LIST);
}

const connMgr::records& connMgr::slaveStatus()
{
    if ((m_pluginVer.majorVersion >= 3) && (m_pluginVer.minorVersion >= 2))
    {
        m_keynum = TD_STSTCS_SLAVE_STATUS;
        allocBuffer();
        getRecords();
        if (m_records.size() > SLAVE_STATUS_DEFAULT_SIZE)
            m_records.resize(SLAVE_STATUS_DEFAULT_SIZE);
        return m_records;
    }
    m_stat = STATUS_NOSUPPORT_OP;
    m_records.resize(0);
    return m_records;
}

const connMgr::records& connMgr::sysvars()
{
    m_keynum = TD_STSTCS_SYSTEM_VARIABLES;
    allocBuffer();
    return getRecords();
}

const connMgr::records& connMgr::connections()
{
    m_keynum = TD_STSTCS_READ;
    m_params[0] = 0;
    m_params[1] = 0;
    allocBuffer();
    return getRecords();
}

const connMgr::records& connMgr::inUseDatabases(__int64 connid)
{
    m_keynum = TD_STSTCS_READ;
    m_params[0] = connid;
    m_params[1] = -1;
    allocBuffer();
    return getRecords();
}

const connMgr::records& connMgr::inUseTables(__int64 connid, int dbid)
{
    m_keynum = TD_STSTCS_READ;
    m_params[0] = connid;
    m_params[1] = dbid;
    allocBuffer();
    return getRecords(true);
}

void connMgr::postDisconnectOne(__int64 connid)
{
    allocBuffer();
    m_keynum = TD_STSTCS_DISCONNECT_ONE;
    record& rec = m_records[0];
    rec.conId = connid;
    tdap(TD_STASTISTICS);
}

void connMgr::postDisconnectAll()
{
    m_keynum = TD_STSTCS_DISCONNECT_ALL;
    tdap(TD_STASTISTICS);
}

short_td connMgr::stat()
{
    return m_stat;
}

connMgr* connMgr::create(database* db)
{
    return new connMgr(db);
}

void connMgr::removeSystemDb(connMgr::records& recs)
{
    for (int i=(int)recs.size() -1; i >= 0; --i)
    {
        if ((strcmp(recs[i].name, "mysql") == 0) ||
            (strcmp(recs[i].name, "performance_schema")==0) ||
            (strcmp(recs[i].name, "information_schema")==0) ||
            (strcmp(recs[i].name, "sys")==0))
        recs.erase(recs.begin() + i);
    }
}

const _TCHAR* connMgr::sysvarName(uint_td index)
{
    if (index < TD_VAR_SIZE)
        return SYSVAR_NAME[index];
    return _T("");
}

const _TCHAR* connMgr::slaveStatusName(uint_td index)
{
    if (index < SLAVE_STATUS_DEFAULT_SIZE)
        return SLAVE_STATUS_NAME[index];
    return _T("");
}

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs
