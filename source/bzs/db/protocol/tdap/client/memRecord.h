#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_MEMRECORD_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_MEMRECORD_H
/*=================================================================
   Copyright (C) 2014 BizStation Corp All rights reserved.

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
#include "fields.h"

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


class autoMemory
{

public:
	explicit autoMemory(unsigned char* p, size_t s, short endIndex, bool own);
    explicit autoMemory(const autoMemory& p);
	~autoMemory();
    autoMemory& operator=(const autoMemory& p);
	unsigned char* ptr;
	size_t size;
	short endFieldIndex;
	bool owner;
};


#define ROW_MEM_BLOCK_RESERVE 4

class memoryRecord : public fieldsBase
{
    friend class multiRecordAlocatorImple;
	std::vector<autoMemory > m_memblock;
    memoryRecord(fieldInfo& fdinfo);

public:
    void clear();
    ~memoryRecord();
    void setRecordData(unsigned char* ptr, size_t size
            , short endFieldIndex, bool owner = false);
    /* return memory block first address which not field ptr address */
    inline unsigned char* memoryRecord::ptr(int index) const
    {
        for (int i=0;i<(int)m_memblock.size();++i)
            if (m_memblock[i].endFieldIndex > index)
                return 	m_memblock[i].ptr;
        assert(0);
        return NULL;
    }

    inline const autoMemory&  memoryRecord::memBlock(int index) const
    {
        for (int i=0;i<(int)m_memblock.size();++i)
            if (m_memblock[i].endFieldIndex > index)
                return m_memblock[i];
        assert(0);
    }

    inline int memoryRecord::memBlockSize() const
    {
        return (int) m_memblock.size();
    }


    static memoryRecord* create(fieldInfo& fdinfo);
    static void release(memoryRecord* p);
};




}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs




#endif //BZS_DB_PROTOCOL_TDAP_CLIENT_MEMRECORD_H

