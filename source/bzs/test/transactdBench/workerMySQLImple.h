#ifndef BZS_TEST_BENCH_WORKERMYSQLIMPLE_H
#define BZS_TEST_BENCH_WORKERMYSQLIMPLE_H
/* =================================================================
 Copyright (C) 20014 BizStation Corp All rights reserved.

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
#include <mysql.h>

namespace bzs
{
namespace test
{
namespace worker
{

namespace mysql
{

#define USE_SHARED_PREPAREDSTATEMENT

#define MYSQL_READ_ONE 10
#define MYSQL_INSERT_ONE 11
#define MYSQL_QUERY 12
#define MYSQL_RECORDSET_COUNT 30

class connectParam
{
public:
    connectParam(){};
    connectParam(const _TCHAR* host, const _TCHAR* dbname, const _TCHAR* user,
                 const _TCHAR* pass = _T(""))

    {
        port = 0;
#ifdef _UNICODE
        char tmp[256];
        WideCharToMultiByte(CP_UTF8, 0, host, -1, tmp, 256, NULL, NULL);
        hostname = tmp;
        WideCharToMultiByte(CP_UTF8, 0, dbname, -1, tmp, 256, NULL, NULL);
        database = tmp;
        WideCharToMultiByte(CP_UTF8, 0, user, -1, tmp, 256, NULL, NULL);
        username = tmp;
        WideCharToMultiByte(CP_UTF8, 0, pass, -1, tmp, 256, NULL, NULL);
        passwd = tmp;
#else
        hostname = host;
        database = dbname;
        username = user;
        passwd = pass;
#endif
    }

    std::string hostname;
    std::string username;
    std::string passwd;
    std::string database;
    unsigned long port;
};

const char* readOneQuery = "select * from user where id = ? \n";
/*const char* queryOneQuery = "select `user`.`id` \n"
                                        ",`user`.`%s` as `name`  \n"
                                        ",`extention`.`comment` \n"
                                        ",`groups`.`name` as `group_name` \n"
                                        " from `user` INNER JOIN `extention` \n"
                                                        " ON `user`.`id`  =
   `extention`.`id` \n"
                                        " LEFT JOIN `groups` \n"
                                                        " ON `user`.`group`  =
   `groups`.`code` \n"
                                        " where `user`.`id` >= ? \n"
                                        "and `user`.`id` < ?";
*/
const char* queryOneQuery =
    "select `user`.`id` ,`user`.`%s` as `name`"
    ",`groups`.`name` as `group_name`"
    " from `user` LEFT JOIN `groups` ON `user`.`group`  = `groups`.`code`"
    "where `user`.`id` >= ? and `user`.`id` < ?";

/*
const char* queryOneQuery = "select `user`.`id` ,`user`.`%s` as `name`"
        " from `user`"
        " where `user`.`id` >= ? and `user`.`id` < ?";
*/
class worker : public workerBase
{
    struct readResult
    {
        int id;
        char name[41];
        int group;
        union
        {
            char tel[43];
            char group_name[43];
        };
        char comment[256];
    };

    const connectParam& m_parmas;
    MYSQL* m_mysql;
    readResult m_result;
    int m_bindParam;
    int m_bindParam2;
    MYSQL_STMT* m_stmt;
    std::vector<readResult> m_resultset;

    void bindParam(MYSQL_STMT* stmt)
    {
        if (m_functionNumber == MYSQL_READ_ONE)
        {
            MYSQL_BIND bind[1];
            memset(bind, 0, sizeof(MYSQL_BIND));
            bind[0].buffer_type = MYSQL_TYPE_LONG;
            bind[0].buffer = &m_bindParam;
            bind[0].is_null = 0;
            if (mysql_stmt_bind_param(stmt, bind))
                printf("error: %s\n", mysql_stmt_error(stmt));
        }
        else if (m_functionNumber == MYSQL_QUERY)
        {
            MYSQL_BIND bind[2];
            memset(bind, 0, sizeof(MYSQL_BIND) * 2);
            bind[0].buffer_type = MYSQL_TYPE_LONG;
            bind[0].buffer = &m_bindParam;
            bind[0].is_null = 0;
            bind[1].buffer_type = MYSQL_TYPE_LONG;
            bind[1].buffer = &m_bindParam2;
            bind[1].is_null = 0;
            if (mysql_stmt_bind_param(stmt, bind))
                printf("error: %s\n", mysql_stmt_error(stmt));
        }
    }

    void bindOutput(MYSQL_STMT* stmt)
    {

        MYSQL_BIND result[4];
        memset(result, 0, sizeof(MYSQL_BIND) * 4);

        result[0].buffer_type = MYSQL_TYPE_LONG;
        result[0].buffer = &m_result.id;
        result[0].is_null = 0;
        result[1].buffer_type = MYSQL_TYPE_VAR_STRING;
        result[1].buffer = m_result.name;
        result[1].buffer_length = 41;
        result[1].is_null = 0;
        if (m_functionNumber == MYSQL_QUERY)
        {
            result[2].buffer_type = MYSQL_TYPE_VAR_STRING;
            result[2].buffer = &m_result.comment;
            result[2].buffer_length = 256;
        }
        else
        {
            result[2].buffer_type = MYSQL_TYPE_LONG;
            result[2].buffer = &m_result.group;
        }
        result[2].is_null = 0;
        result[3].buffer_type = MYSQL_TYPE_VAR_STRING;
        result[3].buffer = m_result.tel; // or group_name
        result[3].buffer_length = 43;
        result[3].is_null = 0;

        if (mysql_stmt_bind_result(stmt, result))
            printf("error: %s\n", mysql_stmt_error(stmt));
    }

    MYSQL_STMT* init(const char* query)
    {
#ifdef LINUX
        const char* fd_name = "名前";
#else
        char fd_name[30];
        WideCharToMultiByte(CP_UTF8, 0, L"名前", -1, fd_name, 30, NULL, NULL);
#endif
        char tmp[512];
        if (m_functionNumber == MYSQL_QUERY)
        {
            sprintf_s(tmp, 512, query, fd_name);
            query = tmp;
        }

        MYSQL_STMT* stmt = mysql_stmt_init(m_mysql);
        if (!stmt)
            printf("error: %s\n", mysql_error(m_mysql));
        if (mysql_stmt_prepare(stmt, query, (unsigned long)strlen(query)))
            printf("error: %s\n", mysql_error(m_mysql));
        bindParam(stmt);
        bindOutput(stmt);
        
        return stmt;
    }

    void readOne(MYSQL_STMT* stmt)
    {
        m_bindParam = m_id % 20000 + 1;
        mysql_stmt_execute(stmt);
        int ret=0;
        ret = mysql_stmt_fetch(stmt);
        if (ret)
            printf("error: %s\n", mysql_error(m_mysql));
    }

    void insertOne()
    {
        char tmp[256];
        sprintf_s(tmp, 256, "insert into cache (value) values(%d)", m_id);
        int ret;
        ret = mysql_query(m_mysql, tmp);
        if (ret)
            printf("error: %s\n", mysql_error(m_mysql));
    }

    void queryOne(MYSQL_STMT* stmt)
    {
        int v = rand() % 15000 + 1;
        m_bindParam = v;
        m_bindParam2 = v + MYSQL_RECORDSET_COUNT;
        int ret;
        ret = mysql_stmt_execute(stmt);
        if (ret)
            printf("error: %s\n", mysql_error(m_mysql));
        int count = 0;
        m_resultset.clear();
        while ((ret = mysql_stmt_fetch(stmt)) == 0)
        {
            m_resultset.push_back(m_result);
            ++count;
        }
        if (count != MYSQL_RECORDSET_COUNT)
            printf("query read error! id = %d \n", m_id);
    }

public:
    worker(int id, int loopCount, int functionNumber, const connectParam& param,
           boost::barrier& sync)
        : workerBase(id, loopCount, functionNumber, sync), m_parmas(param)
    {
        boost::mutex::scoped_lock lck(m_mutex);
       
         
        m_mysql = mysql_init(NULL);
        int v = 1;
        if (NULL == mysql_real_connect(
                        m_mysql, m_parmas.hostname.c_str(),
                        m_parmas.username.c_str(), m_parmas.passwd.c_str(),
                        m_parmas.database.c_str(), m_parmas.port, NULL, 0))
            printf("error: %s\n", mysql_error(m_mysql));
    }
    

    void initExecute()
    {
        mysql_thread_init();
        mysql_set_character_set(m_mysql, "utf8"); 

#ifdef USE_SHARED_PREPAREDSTATEMENT
        if (m_functionNumber == MYSQL_READ_ONE)
            m_stmt = init(readOneQuery);
        else if (m_functionNumber == MYSQL_QUERY)
        {
            m_stmt = init(queryOneQuery);
            queryOne(m_stmt);
        }
#endif
    }

    void endExecute()
    {
#ifdef USE_SHARED_PREPAREDSTATEMENT
        if (m_functionNumber != MYSQL_INSERT_ONE)
            mysql_stmt_close(m_stmt);
#endif
        mysql_close(m_mysql);
        mysql_thread_end();
    }

    void doExecute()
    {
        if (m_functionNumber == MYSQL_READ_ONE)
        {
            for (int i = 0; i < m_loopCount; ++i)
            {
#ifndef USE_SHARED_PREPAREDSTATEMENT
                m_stmt = init(readOneQuery);
#endif
                readOne(m_stmt);
#ifndef USE_SHARED_PREPAREDSTATEMENT
                mysql_stmt_close(m_stmt);
#endif
            }
        }
        else if (m_functionNumber == MYSQL_INSERT_ONE)
        {
            for (int i = 0; i < m_loopCount; ++i)
                insertOne();
        }
        else
        {
            for (int i = 0; i < m_loopCount; ++i)
            {
#ifndef USE_SHARED_PREPAREDSTATEMENT
                m_stmt = init(queryOneQuery);
#endif
                queryOne(m_stmt);
#ifndef USE_SHARED_PREPAREDSTATEMENT
                mysql_stmt_close(m_stmt);
#endif
            }
        }
    }
};

class mysqlInit
{
public:
    mysqlInit() { mysql_library_init(0, NULL, NULL); }
    ~mysqlInit() { mysql_library_end(); }
};

} // namespace transactd
} // namespace worker
} // namespace test
} // namespace bzs

#endif // BZS_TEST_BENCH_WORKERMYSQLIMPLE_H
