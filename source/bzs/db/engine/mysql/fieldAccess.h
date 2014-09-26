#ifndef BZS_DB_ENGINE_MYSQL_FIELDACCESS_H
#define BZS_DB_ENGINE_MYSQL_FIELDACCESS_H
/* =================================================================
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
 ================================================================= */
#include "mysqlInternal.h"

namespace bzs
{
namespace db
{
namespace engine
{
namespace mysql
{

/**
 *	var or bolob type sub functions;
 */
inline uint var_bytes(Field* fd)
{
    return (uint)HA_VARCHAR_PACKLENGTH(fd->field_length);
}

/** sizeof var text body
 */
inline uint var_strlen(const uchar* ptr, uint varlen)
{
    return (varlen == 1) ? *ptr : *((const unsigned short*)ptr);
}

inline uint var_strlen(Field* fd)
{
    return var_strlen(fd->ptr, var_bytes(fd));
}

/** sizeof var text body + size bytes
 */
inline uint var_total_len(Field* fd)
{
    uint varlen = var_bytes(fd);
    return var_strlen(fd->ptr, varlen) + varlen;
}

inline uint var_total_len(const uchar* ptr, uint varlen)
{
    return var_strlen(ptr, varlen) + varlen;
}

/** var text body length on key buffer
 *  size bytes is 2 byte fixed in key buffer.
 */
inline uint var_strlen_key(const uchar* ptr)
{
    return *((const unsigned short*)ptr);
}

/** return Is type var type
 */
inline bool isVarType(enum_field_types type)
{
    return ((type == MYSQL_TYPE_VAR_STRING) || (type == MYSQL_TYPE_VARCHAR));
}

/** return Is type blob type
 */
inline bool isBlobType(enum_field_types type)
{
    return ((type == MYSQL_TYPE_TINY_BLOB) || (type == MYSQL_TYPE_BLOB) ||
            (type == MYSQL_TYPE_MEDIUM_BLOB) || (type == MYSQL_TYPE_LONG_BLOB));
}

inline uint blob_var_bytes(Field* fd)
{
    return fd->pack_length() - portable_sizeof_char_ptr;
}

/** sizeof blob body
 */
inline uint blob_len(const uchar* ptr, uint varlen)
{
    uint v = 0;
    memcpy(&v, ptr, varlen);
    return v;
}

inline uint blob_len(Field* fd)
{
    return blob_len(fd->ptr, blob_var_bytes(fd));
}

/** sizeof blob body ptr + size bytes. not include blob body length
 */
inline uint blob_total_len(Field* fd)
{
    return sizeof(char*) + blob_var_bytes(fd);
}

inline uint blob_total_len(uint varlen)
{
    return sizeof(char*) + varlen;
}

inline const uchar** blobBodyPtrRef(Field* fd)
{
    return (const uchar**)(fd->ptr + blob_var_bytes(fd));
}

inline const uchar* blobBodyPtr(Field* fd)
{
    return *(blobBodyPtrRef(fd));
}

/** return size bytes
 */
inline uint var_bytes_if(Field* fd)
{
    if (isVarType(fd->type()))
        return var_bytes(fd);
    else if (isBlobType(fd->type()))
        return blob_var_bytes(fd);
    return 0;
}

/** Is this field name NIS?
 */
inline bool isNisField(const char* name)
{
    return (name[0] && (name[0] == '$') && name[1] && (name[1] == 'n') &&
            name[2] && (name[2] == 'f'));
}

} // namespace mysql
} // namespace engine
} // namespace db
} // namespace bzs
#endif // BZS_DB_ENGINE_MYSQL_FIELDACCESS_H
