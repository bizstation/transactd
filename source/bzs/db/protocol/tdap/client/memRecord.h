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

/** @cond INTERNAL */

class autoMemory : public refarymem
{
    void releaseMemory();

    autoMemory();
    autoMemory(const autoMemory& p);
    ~autoMemory();
public:
    void setParams(unsigned char* p, size_t s, short* endIndex, bool own);

    autoMemory& operator=(const bzs::db::protocol::tdap::client::autoMemory& p);
    void assignEndFieldIndex(short* p);
    unsigned char* ptr;
    short* endFieldIndex;
    unsigned int size;
    bool owner ;
    static autoMemory* create(int n);
    static autoMemory* create();
};

/** @endcond */

#pragma warning(disable : 4251)
#pragma warning(disable : 4275)
#define ROW_MEM_BLOCK_RESERVE 4

#ifndef JOINLIMIT_PER_RECORD
#define JOINLIMIT_PER_RECORD 14
#endif
#if (JOINLIMIT_PER_RECORD < 1)
#define JOIN_UNLIMIT
#endif

class DLLLIB memoryRecord : public fieldsBase
{
    friend class multiRecordAlocatorImple;
    friend class recordsetImple;
    friend class recordsetQuery;

#ifdef JOIN_UNLIMIT
    std::vector<autoMemory*> m_memblock;
#else
    autoMemory* m_memblock[JOINLIMIT_PER_RECORD];
    int m_memblockSize;
#endif
    mutable int m_blockIndexCache;
    static memoryRecord* create(fielddefs& fdinfo, int n);
    static memoryRecord* create(const memoryRecord& m, int n);


    /** @cond INTERNAL */

    /* return memory block first address which not field ptr address */
    inline unsigned char* ptr(int index) const
    {
        return nullPtr(index) + (*m_fns)[index].nullbytes();
    }

    inline unsigned char* nullPtr(int index) const
    {
        return m_memblock[memoryBlockIndex(index)]->ptr;
    }

    inline int memoryBlockIndex(int index) const 
    { 
        for (int i = 0; i < memBlockSize(); ++i)
            if (*(m_memblock[i]->endFieldIndex) > index)
                return m_blockIndexCache = i;
        assert(0);
        return 0;
    }

    int memoryBlockIndexCache() const { return m_blockIndexCache;}

    inline const autoMemory& memBlockByField(int index) const
    {
        return *m_memblock[memoryBlockIndex(index)];
    }

    inline const autoMemory& memBlock(int index) const
    {
        return *m_memblock[index];
    }
#ifdef JOIN_UNLIMIT
    inline int memBlockSize() const { return (int)m_memblock.size(); }
#else
    inline int memBlockSize() const { return m_memblockSize; }
#endif

    void removeLastMemBlock() 
    { 
        if (memBlockSize())
        {
            m_memblock[memBlockSize()-1]->release();
#ifdef JOIN_UNLIMIT
            m_memblock.pop_back(); 
#else
            --m_memblockSize; 
#endif
        }
    }

    inline void setInvalidMemblockLast() 
    { 
        int num = memBlockSize() -1;
        m_InvalidFlags |= ((2L << num) | 1L); 
    }

    void releaseMemory();

protected:
    inline memoryRecord();
    inline memoryRecord(fielddefs& fdinfo);
    memoryRecord(const memoryRecord& r);
    ~memoryRecord();
    void copyToBuffer(table* tb, bool updateOnly = false) const;
    inline void copyFromBuffer(const table* tb)
    {
        memcpy(nullPtr(0), tb->data(), m_fns->totalFieldLen());
    }
    void setRecordData(autoMemory* am, unsigned char* ptr, size_t size,
                       short* endFieldIndex, bool owner = false);

    /** @endcond */
public:
    void clear(); // orverride
    memoryRecord& operator=(const memoryRecord& r); // For SWIG
    static memoryRecord* create(fielddefs& fdinfo); // For SWIG
};

#pragma warning(default : 4275)
#pragma warning(default : 4251)

/* non copyable*/
class DLLLIB writableRecord : public memoryRecord
{
    friend class activeTableImple;

    fielddefs* m_fddefs;
    table* m_tb;
    short m_endIndex;

    static writableRecord* create(table* tb, const aliasMap_type* alias);
    writableRecord(table* tb, const aliasMap_type* alias);
    writableRecord(const writableRecord&);
    writableRecord& operator=(const writableRecord&);

    fielddefs* fddefs();
    void releaseMemory();
    ~writableRecord();

public:

    bool read(bool KeysetAlrady = false);
    bool read(bookmark_td& bm);
    void insert();
    void del(bool KeysetAlrady = false, bool noSeek = false);
    void update(bool KeysetAlrady = false, bool noSeek = false);
    void save();
    void clear();// orverride

};

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#endif // BZS_DB_PROTOCOL_TDAP_CLIENT_MEMRECORD_H
