/*=================================================================
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
=================================================================*/
#include "ha.h"
#include <boost/thread.hpp>
#include "mysqlInternal.h"
#include <bzs/env/fileopen.h>
#include <bzs/db/protocol/tdap/tdapcapi.h>

/* implemnts in transactd.cpp */
extern unsigned int g_startup_ha;

unsigned int g_ha;
bool trx_blocking = trx_noblock;

namespace bzs
{
namespace db
{
namespace engine
{
namespace mysql
{

#define roleInfoName "transactd_srv_master"


boost::recursive_try_mutex g_tmtx_ha;
boost::try_mutex g_tmtx_trx;

int getRoleFromFile()
{
    int ret = -1;
    char filename[FN_REFLEN];
    fn_format(filename, roleInfoName, "", ".info", MY_REPLACE_EXT | MY_UNPACK_FILENAME);
    FILE* fp = fileOpen(filename, "r");
    if (fp)
    {
        char buf[20];
        char* p = fgets(buf, 20, fp);
        if (p && *p)
            ret = atol(p);
        fclose(fp);
    }
    return ret;
}

bool saveRoleToFile(int role)
{
    char filename[FN_REFLEN];
    fn_format(filename, roleInfoName, "", ".info", MY_REPLACE_EXT | MY_UNPACK_FILENAME);
    FILE* fp = fileOpen(filename, "w");
    if (fp)
    {
        const char* p = (role == HA_ROLE_MASTER) ? "1" : 
                    ((role == HA_ROLE_NONE) ? "2" : "0");
        fputs(p, fp);
        fclose(fp);
        return true;
    }
    return false;
}

int initHa()
{
    boost::recursive_try_mutex::scoped_lock lck(g_tmtx_ha);
    g_ha = g_startup_ha & ~HA_RESTORE_ROLE;
    if (g_startup_ha & HA_RESTORE_ROLE)
    {
        int ret = getRoleFromFile();
        if (ret != -1)
            g_ha |= (unsigned int)ret;
    }
    return g_ha;
}

bool haLock()
{
    return g_tmtx_ha.try_lock();
}

bool haUnlock()
{
#ifdef _WIN32
    if (g_tmtx_ha.locking_thread_id)
    {
        g_tmtx_ha.unlock();
        return true;
    }
    return false;
#else
    g_tmtx_ha.unlock();
    return true;
#endif
}

bool setRole(int role)
{
    int ret = getRole();
    if (ret != role)
    {
        if (g_tmtx_ha.try_lock())
        {
            g_ha &= ~(HA_ROLE_MASTER | HA_ROLE_NONE);
            g_ha |= role;
            if (saveRoleToFile(role))
            {
                g_tmtx_ha.unlock();
                return true;
            }
        }
        return false;
    }
    return true;
}

int getRole()
{
    return (g_ha & (HA_ROLE_MASTER | HA_ROLE_NONE));
}

bool setEnableFailover(bool enable)
{
    if (g_tmtx_ha.try_lock())
    {
        if (enable)
            g_ha |= HA_ENABLE_FAILOVER;
        else
            g_ha &= ~HA_ENABLE_FAILOVER;
        g_tmtx_ha.unlock();
        return true;
    }
    return false;
}

bool getEnableFailover()
{
    return (g_ha & HA_ENABLE_FAILOVER) != 0;
}

bool setTrxBlock(bool v)
{
    if (g_tmtx_trx.try_lock())
    {
        trx_blocking = v ? trx_block : trx_noblock;
        g_tmtx_trx.unlock();
        return true;
    }
    return false;
}


} // namespace mysql
} // namespace engine
} // namespace db
} // namespace bzs

