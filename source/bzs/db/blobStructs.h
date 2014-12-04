#ifndef BZS_DB_BLOBSTRUCTS_H
#define BZS_DB_BLOBSTRUCTS_H
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

namespace bzs
{
namespace db
{

#pragma pack(push, 1)
pragma_pack1;

/** bolob buffer field
 *  size byte is allways 4byte.
 *  2byte null
 */
struct blobField
{
    unsigned int size;
    unsigned short fieldNum;
    inline const char* data() const { return (const char*)(this + 1); }
    inline const blobField* next() const
    {
        return (const blobField*)(data() + size + 2);
    }
};

struct blob
{
    inline blob(unsigned int size, unsigned short fnum,
                const unsigned char* dataptr)
        : ptr(dataptr)
    {
        bf.fieldNum = fnum;
        bf.size = size;
    }
    blobField bf;
    const unsigned char* ptr;
};

/** blob buffer header
 *
 */
struct blobHeader
{
    unsigned int dataSize;  // Not include this and blobField::fieldNum
                            // blobField::size, Only data(s).
    unsigned short rows;
    unsigned short fieldCount;
    mutable unsigned short curRow;
    mutable blobField* nextField;
#ifndef __x86_64__
    blobField* dymmySpace32; // 8byte size space for 32bit
#endif

    inline void resetCur() const
    {
        curRow = 0;
        nextField = (blobField*)(this + 1);
    }
};

#pragma pack(pop)
pragma_pop;

} // namespace db
} // namespace bzs

#endif // BZS_DB_BLOBSTRUCTS_H
