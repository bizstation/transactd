#ifndef BZS_DB_TRANSACTD_CONNECTIONRECORD_H
#define BZS_DB_TRANSACTD_CONNECTIONRECORD_H
/*=================================================================
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
=================================================================*/
#include <bzs/env/compiler.h>
#include <bzs/env/crosscompile.h>

#pragma pack(push, 1)
pragma_pack1;

namespace bzs
{
namespace db
{
namespace transactd
{
namespace connection
{
struct record
{
    record() : conId(0), cid(0), dbid(0), status(0), readCount(0), 
        updCount(0), delCount(0), insCount(0), trnType(0)
    {
        name[0] = 0x00;
    }
    __int64 conId;
    unsigned int cid;
    unsigned short dbid;
    char name[64];
    union
    {
        char status;
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
    unsigned int readCount;
    unsigned int updCount;
    unsigned int delCount;
    unsigned int insCount;
    short trnType;

};

} // connection
} // transactd
} // db
} // bzs

#pragma pack(pop)
pragma_pop;

#endif // BZS_DB_TRANSACTD_CONNECTIONRECORD_H
