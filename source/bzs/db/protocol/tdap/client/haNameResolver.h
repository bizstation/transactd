#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_HANAME_RESOLVER_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_HANAME_RESOLVER_H
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
#include "export.h"

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

/* haNameResolver::start() result code */
#define THNR_SUCCESS               0
#define THNR_SLAVE_HOSTS_NOT_FOUND 1
#define THNR_INVALID_HOSTS         2
#define THNR_REGISTER_FUNC_ERROR   3


#define THNR_OPT_DISABLE_CALL_FAILOVER    1
#define THNR_OPT_MASTER_CAN_CONCUR_SLAVE  2
#define THNR_OPT_FO_READONLY_CONTROL      4

class DLLLIB haNameResolver
{
private:
    haNameResolver();
    haNameResolver(const haNameResolver&);
    ~haNameResolver();
    haNameResolver& operator=(const haNameResolver&);
public:

    /* The slaves and the slaveHostsWithPort can be specified comma separated names. */
    static int start(const char* master, const char* slaves,
                   const char* slaveHostsWithPort, short slaveNum,
                   const char* userName,
                   const char* password,
                   int option = 0);
    static void addPortMap(short mysqlPort, short transactdPort);
    static void clearPortMap();
    static void stop();
    static const char* master();
    static const char* slave();
};

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#endif // BZS_DB_PROTOCOL_TDAP_CLIENT_HANAME_RESOLVER_H

