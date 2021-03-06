#ifndef BZS_DB_ENGINE_MYSQL_HA_H
#define BZS_DB_ENGINE_MYSQL_HA_H
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

const static bool trx_block = false;
const static bool trx_noblock = true;
extern bool trx_blocking;

namespace bzs
{
namespace db
{
namespace engine
{
namespace mysql
{

int initHa();
bool haLock();
bool haUnlock();
bool setRole(int isMaster);
int  getRole();
bool setEnableFailover(bool enable);
bool getEnableFailover();
bool setTrxBlock(bool v);
inline bool isTrxBlocking() { return trx_blocking == trx_block;}

} // namespace mysql
} // namespace engine
} // namespace db
} // namespace bzs

#endif // BZS_DB_ENGINE_MYSQL_HA_H
