#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_HOSTNAME_RESOLVER_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_HOSTNAME_RESOLVER_H
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
#define RV_SUCCESS               0
#define RV_SLAVE_HOSTS_NOT_FOUND 1
#define RV_INVALID_SLAVES        2
#define RV_REGISTER_FUNC_ERROR   3

// Slaves and slaveHosts are comma separated names.
DLLLIB int startResolver(const char* master, const char* slaves, 
                   const char* slaveHostsWithPort, short slaveNum,
                   const char* userName,
                   const char* password);
DLLLIB void addPortMap(short mysqlPort, short transactdPort);
DLLLIB void clearPortMap();
DLLLIB void stopResolver();
DLLLIB void clearResolver();
DLLLIB const char* masterHost();
DLLLIB const char* slaveHost();


} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs
#endif // BZS_DB_PROTOCOL_TDAP_CLIENT_HOSTNAME_RESOLVER_H