/*=================================================================
   Copyright (C) 2013 BizStation Corp All rights reserved.

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

const char* SLAVE_STATUS_NAME[SLAVE_STATUS_DEFAULT_SIZE] =
{
    "Slave_IO_State",
    "Master_Host",
    "Master_User",
    "Master_Port",
    "Connect_Retry",
    "Master_Log_File",
    "Read_Master_Log_Pos",
    "Relay_Log_File",
    "Relay_Log_Pos",
    "Relay_Master_Log_File",
    "Slave_IO_Running",
    "Slave_SQL_Running",
    "Replicate_Do_DB",
    "Replicate_Ignore_DB",
    "Replicate_Do_Table",
    "Replicate_Ignore_Table",
    "Replicate_Wild_Do_Table",
    "Replicate_Wild_Ignore_Table",
    "Last_Errno",
    "Last_Error",
    "Skip_Counter",
    "Exec_Master_Log_Pos",
    "Relay_Log_Space",
    "Until_Condition",
    "Until_Log_File",
    "Until_Log_Pos",
    "Master_SSL_Allowed",
    "Master_SSL_CA_File",
    "Master_SSL_CA_Path",
    "Master_SSL_Cert",
    "Master_SSL_Cipher",
    "Master_SSL_Key",
    "Seconds_Behind_Master",
    "Master_SSL_Verify_Server_Cert",
    "Last_IO_Errno",
    "Last_IO_Error",
    "Last_SQL_Errno",
    "Last_SQL_Error",
    "Replicate_Ignore_Server_Ids",
    "Master_Server_Id" ,
};

connMgr::connMgr(database* db) : nstable(db)
{
    m_db = db;
    m_keybuf = &m_params[0];
    m_params[0] = 0;
    m_params[1] = 0;
    m_keylen = sizeof(m_params);
}

connMgr::~connMgr()
{
}

database* connMgr::db() const
{
    return m_db;
}

void connMgr::connect(const _TCHAR* uri)
{
    m_db->connect(uri, true);
    m_stat = m_db->stat();
    if (m_stat == 0)
    {
        m_uri = uri;
        btrVersions vs;
        m_db->getBtrVersion(&vs);
        m_pluginVer = vs.versions[VER_IDX_PLUGIN];

    }
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

const connMgr::records& connMgr::getRecords()
{
    allocBuffer();
    tdap(TD_STASTISTICS);
    if (m_stat == 0)
        m_records.resize(m_datalen / sizeof(connMgr::record));
    else
        m_records.resize(0);
    return m_records;
}

const connMgr::records& connMgr::definedDatabases()
{
    m_keynum = TD_STSTCS_DATABASE_LIST;
    return getRecords();
}

const connMgr::records& connMgr::doDefinedTables(const char* dbname, int type)
{
    m_keynum = type;
    allocBuffer();
    char tmp[128];
    strcpy_s(tmp, 128, dbname);
    m_keybuf = tmp;
    m_keylen = 128;
    tdap(TD_STASTISTICS);
    if (m_stat == 0)
        m_records.resize(m_datalen / sizeof(connMgr::record));
    else
        m_records.resize(0);
    m_keybuf = &m_params[0];
    m_keylen = sizeof(m_params);
    return m_records;
}

const connMgr::records& connMgr::definedTables(const char* dbname)
{
    if ((m_pluginVer.majorVersion >= 3) && (m_pluginVer.minorVersion >= 2))
        return doDefinedTables(dbname, TD_STSTCS_TABLE_LIST);
    m_stat = STATUS_NOSUPPORT_OP;
    m_records.resize(0);
    return m_records;
}

const connMgr::records& connMgr::definedViews(const char* dbname)
{
    if ((m_pluginVer.majorVersion >= 3) && (m_pluginVer.minorVersion >= 2))
        return doDefinedTables(dbname, TD_STSTCS_VIEW_LIST);
    m_stat = STATUS_NOSUPPORT_OP;
    m_records.resize(0);
    return m_records;
}

const connMgr::records& connMgr::schemaTables(const char* dbname)
{
    return doDefinedTables(dbname, TD_STSTCS_SCHEMA_TABLE_LIST);
}

const connMgr::records& connMgr::slaveStatus()
{
    if ((m_pluginVer.majorVersion >= 3) && (m_pluginVer.minorVersion >= 2))
    {
        m_keynum = TD_STSTCS_SLAVE_STATUS;
        return getRecords();
    }
    m_stat = STATUS_NOSUPPORT_OP;
    m_records.resize(0);
    return m_records;
}

const connMgr::records& connMgr::sysvars()
{
    m_keynum = TD_STSTCS_SYSTEM_VARIABLES;
    return getRecords();
}

const connMgr::records& connMgr::connections()
{
    m_keynum = TD_STSTCS_READ;
    m_params[0] = 0;
    m_params[1] = 0;
    return getRecords();
}

const connMgr::records& connMgr::databases(__int64 connid)
{
    m_keynum = TD_STSTCS_READ;
    m_params[0] = connid;
    m_params[1] = -1;
    return getRecords();
}

const connMgr::records& connMgr::tables(__int64 connid, int dbid)
{
    m_keynum = TD_STSTCS_READ;
    m_params[0] = connid;
    m_params[1] = dbid;
    return getRecords();
}

void connMgr::disconnectOne(__int64 connid)
{
    allocBuffer();
    m_keynum = TD_STSTCS_DISCONNECT_ONE;
    record& rec = m_records[0];
    rec.conId = connid;
    tdap(TD_STASTISTICS);
}

void connMgr::disconnectAll()
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

void removeSystemDb(connMgr::records& recs)
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

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs
