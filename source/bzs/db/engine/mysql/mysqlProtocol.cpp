/* =================================================================
 Copyright (C) 2016 BizStation Corp All rights reserved.

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
/* MySQL 5.5 and Mariadb are no-rtti */
#ifdef __GNUC__
# pragma implementation "mysqlInternal.h" 
#endif

#include "mysqlProtocol.h"
#include <bzs/env/crosscompile.h>
#include <bzs/db/protocol/tdap/tdapcapi.h>
#include <bzs/db/IBlobBuffer.h>
#include "mysqlThd.h"

#if defined(USE_BINLOG_VAR) && (!defined(MARIADB_BASE_VERSION) &&  MYSQL_VERSION_ID > 50600)
#  include "sql/binlog.h"
#endif

//----------------------------------------------------------------------
//  Implement dummyProtocol
//----------------------------------------------------------------------
#if defined(MYSQL_5_7)
#  define Protocol_mysql Protocol
#  define CP_PROTOCOL PROTOCOL_PLUGIN
#else
#  define Protocol_mysql Protocol
#  define CP_PROTOCOL PROTOCOL_BINARY
#endif


#pragma GCC diagnostic ignored "-Woverloaded-virtual"

class dummyProtocol : public Protocol_mysql
{
    THD* m_thd;
    Protocol_mysql* m_backup;
    
public:
#if defined(MYSQL_5_7)
    inline dummyProtocol(THD *thd_arg) : Protocol_mysql()
    {
        m_thd = thd_arg;
        m_backup = m_thd->get_protocol();
        m_thd->set_protocol(this);
    }
    inline virtual ~dummyProtocol()
    {
        m_thd->set_protocol(m_backup);    
    }
#else
    inline dummyProtocol(THD *thd_arg) : Protocol_mysql(thd_arg)
    {
        m_thd = thd_arg;
        m_backup = m_thd->protocol;
        m_thd->protocol = this;
    }
    inline virtual ~dummyProtocol()
    {
        m_thd->protocol = m_backup;       
    }
#endif
    bool send_result_set_metadata(List<Item> *list, uint flags){return false;}
    virtual bool write(){return false;};
    virtual void prepare_for_resend(){}
    virtual bool store_null(){return false;}
    virtual bool store_tiny(longlong from){return store_longlong(from, false);}
    virtual bool store_short(longlong from){return store_longlong(from, false);}
    virtual bool store_long(longlong from){return store_longlong(from, false);}
    virtual bool store_decimal(const my_decimal *){return false;}
    virtual bool store(float from, uint32 decimals, String *buffer){return false;}
    virtual bool store(double from, uint32 decimals, String *buffer){return false;}
    virtual bool store(MYSQL_TIME *time, uint precision){return false;}
    virtual bool store_date(MYSQL_TIME *time){return false;}
    virtual bool store_time(MYSQL_TIME *time, uint precision){return false;}
    virtual bool store(Field *field){return false;}
    virtual bool store(const char *from, size_t length, const CHARSET_INFO *fromcs,
                     const CHARSET_INFO* /*tocs*/){return false;}

    virtual bool send_out_parameters(List<Item_param> *sp_params){return false;}
    virtual Protocol::enum_protocol_type type(void){ return CP_PROTOCOL; };
#ifdef MARIADB_BASE_VERSION      //Mariadb 5.5 10.0 10.1
    virtual bool store(MYSQL_TIME *time, int decimals){return false;}
    virtual bool store_time(MYSQL_TIME *time, int decimals){ return false;}
#elif defined(MYSQL_5_5)
    virtual bool store_time(MYSQL_TIME *time){return true;};
    virtual bool store(MYSQL_TIME *time){return true;}
    virtual bool store(const char *from, size_t length, 
    CHARSET_INFO *fromcs, CHARSET_INFO *tocs){return false;}
#elif defined(MYSQL_5_7) 
    bool store_decimal(const my_decimal *, uint, uint){ return true; }
    bool store(Proto_field *){ return true; }
    void start_row(){}
    int read_packet(void){ return 0; }
    int get_command(COM_DATA *, enum_server_command *){ return m_thd->lex->sql_command; }
    enum_vio_type connection_type(void){ return VIO_TYPE_PLUGIN; }
    ulong get_client_capabilities(void){ return 0; }
    bool has_client_capability(unsigned long){ return false; }
    bool connection_alive(void){ return false; }
    bool end_row(void){ return false; }
    void abort_row(void){}
    void end_partial_result_set(void){}
    int shutdown(bool){ return 0; }
    SSL_handle get_ssl(void){ return NULL; }
    uint get_rw_status(void){ return 0; }
    bool get_compression(void){ return false; }
    bool start_result_metadata(uint, uint, const CHARSET_INFO *){ return false; }
    bool send_field_metadata(Send_field *, const CHARSET_INFO *){ return false; }
    bool end_result_metadata(void){ return false; }
    bool send_ok(uint, uint, ulonglong, ulonglong, const char *){ return false; }
    bool send_eof(uint, uint){ return false; }
    bool send_error(uint, const char *, const char *){ return false; }
#endif
};

//----------------------------------------------------------------------
//  class masterStatus for windows mysql 5.6 only
//----------------------------------------------------------------------
#if defined(NOTUSE_BINLOG_VAR)
class masterStatus : public dummyProtocol
{
    binlogPos* m_bpos;
    bool m_writed;
public:
    inline masterStatus(THD *thd_arg, binlogPos* bpos) : 
        dummyProtocol(thd_arg), m_bpos(bpos), m_writed(false) {}
    bool store_longlong(longlong from, bool unsigned_flag)
    {
        m_bpos->pos = (ulonglong)from;
        m_bpos->type = REPL_POSTYPE_POS;
        return false;
    }

#if (MYSQL_VERSION_ID < 50600 || defined(MARIADB_BASE_VERSION)) // mariadb 5.5 
    bool store(const char *from, size_t length, CHARSET_INFO *cs)
    {
        if (!m_writed)
        {
            strncpy(m_bpos->filename, from, BINLOGNAME_SIZE); 
            m_writed = true;
        }
        return false;
    }
#else
    bool store(const char *from, size_t length, const CHARSET_INFO *cs)
    {
        if (!m_writed)
        {
            strncpy(m_bpos->filename, from, BINLOGNAME_SIZE);
            m_writed = true;
        }
        return false;
    }
#endif
};
#endif // NOTUSE_BINLOG_VAR

//----------------------------------------------------------------------
//  class safe_commit_lock  commit lock for binlogPos
//----------------------------------------------------------------------
safe_commit_lock::safe_commit_lock(THD* thd): m_thd(thd), m_commits_lock(NULL)
{

}

bool safe_commit_lock::lock()
{
    if (m_thd)
    {
        MDL_request mdl_request;
        #if ((MYSQL_VERSION_NUM > 50700) && !defined(MARIADB_BASE_VERSION))
        mdl_request.init_with_source(MDL_key::COMMIT, "", "", MDL_SHARED, MDL_EXPLICIT, __FILE__, __LINE__);
        #else
        mdl_request.init(MDL_key::COMMIT, "", "", MDL_SHARED, MDL_EXPLICIT);
        #endif
        if (m_thd->mdl_context.acquire_lock(&mdl_request,
                                    m_thd->variables.lock_wait_timeout))
            return false;
        m_commits_lock = mdl_request.ticket;
    }
    return true;
}

safe_commit_lock::~safe_commit_lock()
{
    if (m_commits_lock)
    {
        m_thd->mdl_context.release_lock(m_commits_lock);
        m_commits_lock= NULL;
    }
}


#ifdef NOTUSE_BINLOG_VAR 
    inline short getBinlogPosInternal(THD* currentThd, binlogPos* bpos, THD* tmpThd)
    {
        short result = 0;
        {
            attachThd(tmpThd);
            copyGrant(tmpThd, currentThd, NULL);
            masterStatus p(tmpThd, bpos); 
            cp_query_command(tmpThd, "show master status");
            if (tmpThd->is_error())
                result = tmpThd->cp_get_sql_error();
            cp_lex_clear(tmpThd);
        }
        attachThd(currentThd);
        return result;
    }
#endif

#ifdef USE_BINLOG_GTID
    inline short getBinlogPosInternal(THD* currentThd, binlogPos* bpos, THD* /*tmpThd*/)
    {
        if (mysql_bin_log.is_open())
        {
            rpl_gtid gtid;
            bpos->type = REPL_POSTYPE_MARIA_GTID;
            if (mysql_bin_log.lookup_domain_in_binlog_state(currentThd->variables.gtid_domain_id,  &gtid))
            {
                sprintf_s(bpos->gtid, GTID_SIZE, "%u-%u-%llu", gtid.domain_id, gtid.server_id, gtid.seq_no); 
                size_t dir_len = dirname_length(mysql_bin_log.get_log_fname());
                strncpy(bpos->filename, mysql_bin_log.get_log_fname() + dir_len, BINLOGNAME_SIZE);
                bpos->pos = my_b_tell(mysql_bin_log.get_log_file());
                bpos->filename[BINLOGNAME_SIZE-1] = 0x00;
            }
        }
        return 0;
    }
#endif

#ifdef USE_BINLOG_VAR
    // Linux MySQL can access to the mysql_bin_log variable
    inline short getBinlogPosInternal(THD* , binlogPos* bpos, THD* /*tmpThd*/)
    {
        if (mysql_bin_log.is_open())
        {
            size_t dir_len = dirname_length(mysql_bin_log.get_log_fname());
            strncpy(bpos->filename, mysql_bin_log.get_log_fname() + dir_len, BINLOGNAME_SIZE);
            bpos->pos = my_b_tell(mysql_bin_log.get_log_file());
            bpos->filename[BINLOGNAME_SIZE-1] = 0x00;
            bpos->type = REPL_POSTYPE_POS;
        }
        return 0;
    }
#endif //USE_BINLOG_VAR

short getBinlogPos(THD* thd, binlogPos* bpos, THD* tmpThd)
{
    #ifndef NOTUSE_BINLOG_VAR
    safe_mysql_mutex_lock lck(mysql_bin_log.get_log_lock());
    #endif
    return getBinlogPosInternal(thd, bpos, tmpThd);
}

int execSql(THD* thd, const char* sql)
{
    thd->variables.lock_wait_timeout = OPEN_TABLE_TIMEOUT_SEC;
    thd->clear_error();
    int result = cp_query_command(thd, (char*)sql);
    if (thd->is_error())
        result = thd->cp_get_sql_error();
    cp_lex_clear(thd); // reset values for insert
    return result;
}

//----------------------------------------------------------------------
//  slaveStatus  
//----------------------------------------------------------------------
using namespace bzs::db::transactd;
class slaveStatus : public dummyProtocol
{
    connection::records& m_records;
    bzs::db::IblobBuffer* m_bb;
    int m_blobfields;
    connection::record& getRec()
    {
        m_records.push_back(connection::record());
        return m_records[m_records.size() - 1];
    }
public:
    inline slaveStatus(THD *thd_arg, connection::records& recs, bzs::db::IblobBuffer* bb) : 
        dummyProtocol(thd_arg), m_records(recs), m_bb(bb), m_blobfields(0)
    {
        m_records.clear();   
    }
    ~slaveStatus()
    {
        m_bb->setFieldCount(m_blobfields);
    }
    bool store_longlong(longlong from, bool unsigned_flag)
    {
        connection::record& rec = getRec();
        rec.type = 0;
        rec.longValue = from; 
        return false;
    }
    bool store_null()
    {
        connection::record& rec = getRec();
        rec.type = 1;
        strncpy(rec.name, "NULL", CON_REC_VALUE_SIZE); 
        return false;
    }
    bool str_store(const char *from, size_t length)
    {
        connection::record& rec = getRec();
        rec.type = 1;
        if (length && from)
        {
            if (length >= CON_REC_VALUE_SIZE)
            {
                ++m_blobfields;
                rec.type = 2;
                m_bb->addBlob((unsigned int)length, (unsigned short)(m_records.size() -1),
                        (const unsigned char*)from);
            }else
                strncpy(rec.name, from, CON_REC_VALUE_SIZE);
        }
        return false;
    }

#if (MYSQL_VERSION_ID < 50600 || defined(MARIADB_BASE_VERSION)) // MySQL 5.5 
    bool store(const char *from, size_t length, CHARSET_INFO *cs)
    {
        return str_store(from, length);
    }
#else
    bool store(const char *from, size_t length, const CHARSET_INFO *cs)
    {
        return str_store(from, length);
    }
#endif
};

int getSlaveStatus(THD* thd, connection::records& recs, bzs::db::IblobBuffer* bb)
{
    slaveStatus ss(thd, recs, bb);
    return execSql(thd, "show slave status");
}

#pragma GCC diagnostic warning "-Woverloaded-virtual"


//----------------------------------------------------------------------
//  database list
//----------------------------------------------------------------------
inline void appenDbList(connection::records& recs, LEX_STRING* db_name)
{
    recs.push_back(connection::record());
    connection::record& rec = recs[recs.size() - 1];
    strncpy(rec.name, db_name->str, 64);
    rec.name[64] = 0x00;
}

void readDbList(THD* thd, connection::records& recs)
{
    SQL_Strings files;
    db_list(thd, &files);
#if (defined(MARIADB_10_0) || defined(MARIADB_10_1))
    for (int i = 0; i < (int)files.elements(); ++i)
        appenDbList(recs, files.at(i));
#else
    List_iterator_fast<LEX_STRING> it(files);
    LEX_STRING* db_name;
    while ((db_name = it++))
        appenDbList(recs, db_name);
#endif
}

//----------------------------------------------------------------------
//  security
//----------------------------------------------------------------------

bool setGrant(THD* thd, const char* host, const char* user,  const char* db)
{
    // sctx->master_access and sctx->db_access
    return (acl_getroot(cp_security_ctx(thd), cp_strdup(user, MYF(0)),
    cp_strdup(host, MYF(0)), cp_strdup(host, MYF(0)), (char*)db)) == false;
}

bool copyGrant(THD* thd, THD* thdSrc, const char* db)
{
    Security_context* sctx = cp_security_ctx(thdSrc);
    if (sctx->cp_master_accsess() == (ulong)~NO_ACCESS)
    {
        cp_security_ctx(thd)->skip_grants();
        return true;
    }
    return setGrant(thd, sctx->cp_priv_host(), sctx->cp_priv_user(), db);
}

void setDbName(THD* thd, const char* name)
{
    cp_set_db(thd, name);
}

/*
class safe_global_read_lock
{
    THD* m_thd;
public:
    safe_global_read_lock(THD* thd): m_thd(thd){}
    bool lock()
    {
        if (m_thd->global_read_lock.lock_global_read_lock(m_thd))
        {
            m_thd = NULL;
            return false;
        }
        #ifdef NOTUSE_BINLOG_VAR
        close_cached_tables(NULL, NULL, FALSE , 50000000L);
        if (m_thd->global_read_lock.make_global_read_lock_block_commit(m_thd))
        {
            m_thd->global_read_lock.unlock_global_read_lock(m_thd);
            m_thd = NULL;
            return false;
        }
        #endif
        return true;
    }
    ~safe_global_read_lock()
    {
        if (m_thd)
            m_thd->global_read_lock.unlock_global_read_lock(m_thd);
    }
};*/

