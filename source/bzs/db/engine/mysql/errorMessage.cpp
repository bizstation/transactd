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
#include <my_config.h>
#include "errorMessage.h"
#undef PACKAGE
#include "mysqlInternal.h"
#include <bzs/db/protocol/tdap/tdapcapi.h>

namespace bzs
{
namespace db
{
namespace engine
{
namespace mysql
{

const char* errorMessage(int errorCode)
{
    switch (errorCode)
    {
    case STATUS_PROGRAM_ERROR:
        return "Program error";
    case STATUS_CANNOT_LOCK_TABLE:
        return "Can not open the table Allrady locked";
    case STATUS_TABLE_NOTOPEN:
        return "Can not open the table";
    case STATUS_INVALID_KEYNAME:
        return "Invalid key name";
    case HA_ERR_KEY_NOT_FOUND:
        return "Did not find key on read or update";
    case STATUS_INVALID_LOCKTYPE:
        return "locktype is diffarent to got locks";
    case STATUS_INVALID_DATASIZE:
        return "Invalid data buffer size";
    case STATUS_TABLE_EXISTS_ERROR:
        return "Can not check if exists table";
    }
    return "";
}

void printWarningMessage(const int* errorCode, const std::string* message)
{
    int code = errorCode ? *errorCode : 0;
    std::string msg = errorMessage(code);
    if (message)
        msg += *message;

    if ((code != STATUS_TABLE_NOTOPEN) && (code != STATUS_INVALID_BOOKMARK))
        sql_print_warning("Transactd: %s", msg.c_str());
}

} // namespace mysql
} // namespace engine
} // namespace db
} // namespace bzs
