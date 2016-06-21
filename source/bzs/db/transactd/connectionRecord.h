#ifndef BZS_DB_TRANSACTD_CONNECTIONRECORD_H
#define BZS_DB_TRANSACTD_CONNECTIONRECORD_H
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
#include <bzs/env/compiler.h>
#include <bzs/env/crosscompile.h>
#include <vector>

#pragma pack(push, 1)
pragma_pack1

namespace bzs
{
namespace db
{
namespace transactd
{
namespace connection
{

/* Slave status row(field) index 
record::type data type  
   0 longlong to longValue
   1 string   to value
*/
#define SLAVE_STATUS_IO_STATE                    0
#define SLAVE_STATUS_MASTER_HOST                 1
#define SLAVE_STATUS_MASTER_USER                 2
#define SLAVE_STATUS_MASTER_PORT                 3
#define SLAVE_STATUS_CONNECT_RETRY               4
#define SLAVE_STATUS_MASTER_LOG_FILE             5 
#define SLAVE_STATUS_READ_MASTER_LOG_POS         6
#define SLAVE_STATUS_RELAY_LOG_FILE              7
#define SLAVE_STATUS_RELAY_LOG_POS               8
#define SLAVE_STATUS_RELAY_MASTER_LOG_FILE       9
#define SLAVE_STATUS_SLAVE_IO_RUNNING            10
#define SLAVE_STATUS_SLAVE_SQL_RUNNING           11
#define SLAVE_STATUS_REPLICATE_DO_DB             12
#define SLAVE_STATUS_REPLICATE_IGNORE_DB         13 
#define SLAVE_STATUS_REPLICATE_DO_TABLE          14
#define SLAVE_STATUS_REPLICATE_IGNORE_TABLE      15
#define SLAVE_STATUS_REPLICATE_WILD_DO_TABLE     16
#define SLAVE_STATUS_REPLICATE_WILD_IGNORE_TABLE 17
#define SLAVE_STATUS_LAST_ERRNO                  18
#define SLAVE_STATUS_LAST_ERROR                  19
#define SLAVE_STATUS_SKIP_COUNER                 20
#define SLAVE_STATUS_EXEC_MASTER_LOG_POS         21 
#define SLAVE_STATUS_RELAY_LOG_SPACE             22
#define SLAVE_STATUS_UNTIL_CONDITION             23
#define SLAVE_STATUS_UNTIL_LOG_FILE              24
#define SLAVE_STATUS_UNTIL_LOG_POS               25
#define SLAVE_STATUS_MASTER_SSL_ALLOWED          26  
#define SLAVE_STATUS_MASTER_SSL_CA_FILE          27
#define SLAVE_STATUS_MASTER_SSL_CA_PATH          28
#define SLAVE_STATUS_MASTER_SSL_CERT             29
#define SLAVE_STATUS_MASTER_SSL_CIPHER           30 
#define SLAVE_STATUS_MASTER_SSL_KEY              31
#define SLAVE_STATUS_SECONDS_BEHIND_MASTER       32
#define SLAVE_STATUS_MASTER_SSL_VERIFY_SERVER_CERT 33 
#define SLAVE_STATUS_LAST_IO_ERRNO               34
#define SLAVE_STATUS_LAST_IO_ERROR               35
#define SLAVE_STATUS_LAST_SQL_ERRNO              36
#define SLAVE_STATUS_LAST_SQL_ERROR              37
#define SLAVE_STATUS_REPLICATE_IGNORE_SERVER_IDS 38
#define SLAVE_STATUS_MASTER_SERVER_ID            39
#define SLAVE_STATUS_DEFAULT_SIZE                40

//#ifdef MARIADB_BASE_VERSION
#  define SLAVE_STATUS_MA_MASTER_SSL_CRL              40
#  define SLAVE_STATUS_MA_MASTER_SSL_CRLPATH          41
#  define SLAVE_STATUS_MA_USING_GTID                  42
#  define SLAVE_STATUS_MA_GTID_IO_POS                 43
#  define SLAVE_STATUS_MA_REPLICATE_DO_DOMAIN_IDS     44
#  define SLAVE_STATUS_MA_REPLICATE_IGNORE_DOMAIN_IDS 45
#  define SLAVE_STATUS_MA_PARALLEL_MODE               46
#  define SLAVE_STATUS_MA_RETRIED_TRANSACTIONS        47
#  define SLAVE_STATUS_MA_MAX_RELAY_LOG_SIZE          48
#  define SLAVE_STATUS_MA_EXECUTED_LOG_ENTRIES        49
#  define SLAVE_STATUS_MA_SLAVE_RECV_HEARTBEATS       50
#  define SLAVE_STATUS_MA_SLAVE_HEARTBEATS_PERIOD     51
#  define SLAVE_STATUS_MA_GTID_SLAVE_POS              52
#  define SLAVE_STATUS_MA_SIZE                        53
//#else
#  define SLAVE_STATUS_MASTER_UUID               40
#  define SLAVE_STATUS_MASTER_INFO_FILE          41
#  define SLAVE_STATUS_SQL_DELAY                 42
#  define SLAVE_STATUS_SQL_REMAINING_DELAY       43
#  define SLAVE_STATUS_SQL_RUNNING_STATE         44
#  define SLAVE_STATUS_MASTER_RETRY_COUNT        45
#  define SLAVE_STATUS_MASTER_BIND               46
#  define SLAVE_STATUS_LAST_IO_ERROR_TIMESTAMP   47
#  define SLAVE_STATUS_LAST_SQL_ERROR_TIMESTAMP  48
#  define SLAVE_STATUS_MASTER_SSL_CRL            49
#  define SLAVE_STATUS_MASTER_SSL_CRLPATH        50
#  define SLAVE_STATUS_RETRIEVED_GTID_SET        51
#  define SLAVE_STATUS_EXECUTED_GTID_SET         52
#  define SLAVE_STATUS_AUTO_POSITION             53
#  define SLAVE_STATUS_REPLICATE_REWRITE_DB      54
#  define SLAVE_STATUS_CHANNEL_NAME              55
#  define SLAVE_STATUS_SIZE                      56
//#endif
#define SLAVE_STATUS_EX_MA_SIZE SLAVE_STATUS_MA_SIZE - SLAVE_STATUS_DEFAULT_SIZE
#define SLAVE_STATUS_EX_SIZE    SLAVE_STATUS_SIZE - SLAVE_STATUS_DEFAULT_SIZE




#define CON_REC_VALUE_SIZE 67

struct record
{
    record() : conId(0), id(0), db(0), updCount(0), status(0)
    {
        name[0] = 0x00;
    }
    void reset()
    {
        name[0] = 0x00;
        conId = 0;
        id = 0;
        db = 0;
        updCount = 0;
        status = 0;
    }
    union
    {
        __int64 conId;                      // 8 byte
        __int64 longValue;
        struct
        {
            unsigned int delCount;                  
            unsigned int insCount;                  
        };
    };
    unsigned int id;                        // 4 byte
    union
    {
        unsigned int db;
        unsigned int readCount;             // 4 byte
    };
    union
    {
        unsigned int updCount;              // 4 byte
        unsigned int type;                           
    };
    
    char name[CON_REC_VALUE_SIZE];           // 67 byte
    union
    {
        char status;                        // 1 byte
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


    #ifdef _UNICODE
    inline const wchar_t* t_name(wchar_t* buf, int size) const
    {
        MultiByteToWideChar(CP_UTF8, 0, name, -1, buf, size);
        return buf;
    }
    const wchar_t* value(wchar_t* buf, int size) const
    {
        if (type == 0)
        {
            _i64tow_s(longValue, buf, size, 10);
            return buf;
        }else if (type == 1)
        {
            MultiByteToWideChar(CP_UTF8, 0, name, -1, buf, size);
            return buf;
        }
        MultiByteToWideChar(CP_UTF8, 0, (char*)longValue, -1, buf, size);
        return buf;
    }
    #endif
    inline const char* t_name(char* /*buf*/, int /*size*/) const
    {
        return name;
    }
    const char* value(char* buf, int size) const
    {
        if (type == 0)
        {
            _i64toa_s(longValue, buf, size, 10);

        }else if (type == 1)
            strcpy_s(buf, size, name);
        else
            strcpy_s(buf, size, (char*)longValue);
        return buf;
    }
    const char* value_ptr() const
    {
        if (type == 0)
            return (const char*)&longValue;
        else if(type == 1)
            return name;
        return (const char*)longValue;
    }

};                                          // 20 + 68 = 88

#ifdef MYSQL_DYNAMIC_PLUGIN
typedef std::vector<record> records;
#endif

} // connection
} // transactd
} // db
} // bzs

#pragma pack(pop)
pragma_pop

#endif // BZS_DB_TRANSACTD_CONNECTIONRECORD_H
