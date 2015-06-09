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
        m_uri = uri;
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
    return m_records;
}

const connMgr::records& connMgr::definedDatabases()
{
    m_keynum = TD_STSTCS_DATABASE_LIST;
    return getRecords();
}

const connMgr::records& connMgr::schemaTables(const char* dbname)
{
    m_keynum = TD_STSTCS_SCHEMA_TABLE_LIST;
    allocBuffer();
    char tmp[128];
    strcpy_s(tmp, 128, dbname);
    m_keybuf = tmp;
    m_keylen = 128;
    tdap(TD_STASTISTICS);
    if (m_stat == 0)
        m_records.resize(m_datalen / sizeof(connMgr::record));
    m_keybuf = &m_params[0];
    m_keylen = sizeof(m_params);
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

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs
