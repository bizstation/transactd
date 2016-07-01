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
    _T("timestamp_always"),
    _T("startup_ha")
};

static const _TCHAR* STATUSVAR_NAME[TD_SVAR_SIZE] =
{
    _T("tcp_connections"),
    _T("tcp_wait_threads"),
    _T("tpool_connections"),
    _T("tpool_threads"),
    _T("pipe_connections"),
    _T("pipe_wait_threads"),
    _T("cur_open_databases"),
    _T("ha"),
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

static const _TCHAR* SLAVE_STATUS_NAME_EX[SLAVE_STATUS_EX_SIZE] =
{
    _T("Master_UUID"),
    _T("Master_Info_File"),
    _T("SQL_Delay"),
    _T("SQL_Remaining_Delay"),
    _T("Slave_SQL_Running_State"),
    _T("Master_Retry_Count"),
    _T("Master_Bind"),
    _T("Last_IO_Error_Timestamp"),
    _T("Last_SQL_Error_Timestamp"),
    _T("Master_SSL_Crl"),
    _T("Master_SSL_Crlpath"),
    _T("Retrieved_Gtid_Set"),
    _T("Executed_Gtid_Set"),
    _T("Auto_Position"),
    _T("Replicate_Rewrite_DB"),
    _T("Channel_Name"),
};

static const _TCHAR* SLAVE_STATUS_NAME_EX_MA[SLAVE_STATUS_EX_MA_SIZE] =
{
    _T("Master_SSL_Crl"),
    _T("Master_SSL_Crlpath"),
    _T("Using_Gtid"),
    _T("Gtid_IO_Pos"),
    _T("Replicate_Do_Domain_Ids"),
    _T("Replicate_Ignore_Domain_Ids"),
    _T("Parallel_Mode"),
    _T("Retried_transactions"),
    _T("Max_relay_log_size"),
    _T("Executed_log_entries"),
    _T("Slave_received_heartbeats"),
    _T("Slave_heartbeat_period"),
    _T("Gtid_Slave_Pos"),
};

static const _TCHAR* EXTENDED_VAR_NAME[TD_EXTENDED_VAR_SIZE] =
{
    _T("MySQL_Gtid_Mode"),
    _T("Binlog_File"),
    _T("Binlog_Position"),
    _T("Executed_Gtid_Set/Gtid_Cur_Pos")
};

class stringBuffer
{
	friend class connMgr;
	std::vector<char> m_buf;
public:
	inline void resize(size_t size) { m_buf.resize(size); }
	inline char* ptr() { return &m_buf[0]; }
	inline size_t size() const { return m_buf.size(); }
};

//-----------------------------------------------------------------------------
//    class connRecords
//-----------------------------------------------------------------------------
connRecords::connRecords(){}

connRecords::connRecords(const connRecords& r) :
    m_records(r.m_records), m_buf(r.m_buf) {}

connRecords& connRecords::operator=(const connRecords& r)
{
    if (this != &r)
    {
    	m_records = r.m_records;
	    m_buf = r.m_buf;
    }
    return *this;
}

void connRecords::clear() { m_records.clear(); m_buf.reset(); }

const connRecords::record& connRecords::operator[] (size_t index) const
{
    return m_records[index];
}

connRecords::record& connRecords::operator[] (size_t index)
{
    return m_records[index];
}

size_t connRecords::size() const { return m_records.size(); }

connRecords* connRecords::create(){ return new connRecords();}

connRecords* connRecords::create(const connRecords& r){ return new connRecords(r);}

void connRecords::release(){ delete this;}

//-----------------------------------------------------------------------------

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
    m_datalen = m_buflen = 0;
}

connMgr::~connMgr() {setIsOpen(false);}

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
        setIsOpen(true);
        m_uri = uri;
        btrVersions vs;
        m_db->getBtrVersion(&vs);
        m_pluginVer = vs.versions[VER_IDX_PLUGIN];
        m_serverVer = vs.versions[VER_IDX_DB_SERVER];
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
        {
            m_uri = _T("");
            setIsOpen(false);
        }
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
    if (m_keynum < TD_STSTCS_SLAVE_STATUS)
    {
        if (m_stat == 0 && *((int*)m_keybuf) != (int)sizeof(connMgr::record))
            convertFromOldFormat(isInUseTable);
    }
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

void connMgr::setBlobFieldPointer(const blobHeader* hd)
{
    if (!hd) return;
    assert(hd->curRow < hd->rows);
    m_records.m_buf.reset(new stringBuffer());
    m_records.m_buf->resize(hd->dataSize + 100);
    const blobField* f = hd->nextField;

    char* p = m_records.m_buf->ptr();
    for (int i = 0; i < hd->fieldCount; ++i)
    {
        memcpy(p, f->data(), f->size);
        m_records[f->fieldNum].longValue = (__int64)p;
        p += f->size;
        *p = 0x00;
        ++p;
        f = f->next();
    }
}

const connMgr::records& connMgr::blobOperation(int op)
{
    if ((m_pluginVer.majorVersion >= 3) && (m_pluginVer.minorVersion >= 2))
    {
        m_keynum = op;
        allocBuffer();
        getRecords();
        // set blob pointers
        setBlobFieldPointer(getBlobHeader());
        return m_records;
    }
    m_stat = STATUS_NOSUPPORT_OP;
    m_records.resize(0);
    return m_records;
}

const connMgr::records& connMgr::slaveStatus(const char* channel)
{
    char ch[65] = {0};
    if (channel)
        strcpy_s(ch, 65, channel);
    m_keybuf = (void*)ch;
    m_keylen = 65;
    blobOperation(TD_STSTCS_SLAVE_STATUS);
    m_keybuf = &m_params[0];
    m_keylen = sizeof(m_params);
    return m_records;
}
#ifdef _UNICODE
const connMgr::records& connMgr::slaveStatus(const wchar_t* channel)
{
    char tmp[128] = {0};
    if (channel)
        WideCharToMultiByte(CP_UTF8, 0, channel,-1, tmp, 128, NULL, NULL);
    return slaveStatus(tmp);
}
#endif


const connMgr::records& connMgr::channels(bool withLock )
{
    m_keynum = withLock ? 
                TD_STSTCS_SLAVE_CHANNELS_LOCK :TD_STSTCS_SLAVE_CHANNELS;
    allocBuffer();
    return getRecords();
}

const connMgr::records& connMgr::slaveHosts()
{
    return blobOperation(TD_STSTCS_SLAVE_HOSTS);
}

const connMgr::records& connMgr::extendedvars()
{
    return blobOperation(TD_STSTCS_EXTENDED_VARIABLES);
}

const connMgr::records& connMgr::sysvars()
{
    m_keynum = TD_STSTCS_SYSTEM_VARIABLES;
    allocBuffer();
    return getRecords();
}

const connMgr::records& connMgr::statusvars()
{
    m_keynum = TD_STSTCS_STATUS_VARIABLES;
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
    m_keynum = TD_STSTCS_DISCONNECT_ONE;
    m_params[0] = connid;
    m_datalen = 0;
    tdap(TD_STASTISTICS);
}

void connMgr::postDisconnectAll()
{
    m_keynum = TD_STSTCS_DISCONNECT_ALL;
    m_datalen = 0;
    tdap(TD_STASTISTICS);
}

bool connMgr::haLock()
{
    m_keynum = TD_STSTCS_HA_LOCK;
    m_datalen = 0;
    tdap(TD_STASTISTICS);
    return stat() == 0;
}

void connMgr::haUnlock()
{
    m_keynum = TD_STSTCS_HA_UNLOCK;
    m_datalen = 0;
    tdap(TD_STASTISTICS);

}

bool connMgr::setRole(int v)
{
    m_keynum = (v == HA_ROLE_MASTER) ? TD_STSTCS_HA_SET_ROLEMASTER :
                (v == HA_ROLE_SLAVE) ? TD_STSTCS_HA_SET_ROLESLAVE :
                                        TD_STSTCS_HA_SET_ROLENONE;
    m_datalen = 0;
    tdap(TD_STASTISTICS);
    return stat() == 0;
}

bool connMgr::setTrxBlock(bool v)
{
    m_keynum = v ? TD_STSTCS_HA_SET_TRXBLOCK :
                          TD_STSTCS_HA_SET_TRXNOBLOCK;
    m_datalen = 0;
    tdap(TD_STASTISTICS);
    return stat() == 0;
}

bool connMgr::setEnableFailover(bool v)
{
    m_keynum = v ? TD_STSTCS_HA_ENABLE_FO :
                          TD_STSTCS_HA_DISBLE_FO;
    m_datalen = 0;
    tdap(TD_STASTISTICS);
    return stat() == 0;
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
        recs.erase(/*recs.begin() +*/ i);
    }
}

const _TCHAR* connMgr::sysvarName(uint_td index)
{
    if (index < TD_VAR_SIZE)
        return SYSVAR_NAME[index];
    return _T("");
}

const _TCHAR* connMgr::statusvarName(uint_td index)
{
    if (index < TD_SVAR_SIZE)
        return STATUSVAR_NAME[index];
    return _T("");
}

const _TCHAR* slaveStatusName1(uint_td index)
{
    if (index < SLAVE_STATUS_DEFAULT_SIZE)
        return SLAVE_STATUS_NAME[index];
    return _T("");
}

const _TCHAR* connMgr::slaveStatusName(uint_td index) const
{
    bool mariadb = m_serverVer.isMariaDB();
    if (index < SLAVE_STATUS_DEFAULT_SIZE)
        return slaveStatusName1(index);
    index -= SLAVE_STATUS_DEFAULT_SIZE;
    if(mariadb &&  (index < SLAVE_STATUS_EX_MA_SIZE))
        return SLAVE_STATUS_NAME_EX_MA[index];
    else if(!mariadb &&  (index < SLAVE_STATUS_EX_SIZE))
        return SLAVE_STATUS_NAME_EX[index];
    return _T("");
}

const _TCHAR* connMgr::extendedVarName(uint_td index)
{
    if (index < TD_EXTENDED_VAR_SIZE)
        return EXTENDED_VAR_NAME[index];
    return _T("");
}

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs
