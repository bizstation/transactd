#ifndef BZS_DB_ENGINE_MYSQL_ERRORMESSAGE_H
#define BZS_DB_ENGINE_MYSQL_ERRORMESSAGE_H
/* =================================================================
 Copyright (C) 2012 2013 BizStation Corp All rights reserved.

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

#include <string>
#include <bzs/rtl/exception.h>

namespace bzs
{
namespace db
{
namespace engine
{
namespace mysql
{

const char* errorMessage(int errorCode);
void printWarningMessage(const int* errorCode, const std::string* msg);

} // namespace mysql
} // namespace engine
} // namespace db
} // namespace bzs

#endif // BZS_DB_ENGINE_MYSQL_ERRORMESSAGE_H
