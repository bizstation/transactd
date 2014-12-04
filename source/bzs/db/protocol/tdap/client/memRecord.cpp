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
#pragma hdrstop

#include "memRecord.h"
#include <bzs/db/protocol/tdap/client/trdboostapi.h>
#include <new>

//---------------------------------------------------------------------------
#pragma package(smart_init)
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

autoMemory::autoMemory() : ptr(0), endFieldIndex(NULL), size(0), owner(false)
{
}

autoMemory::autoMemory(unsigned char* p, size_t s, short* endIndex, bool own)
    : ptr(p), endFieldIndex(NULL), size((unsigned int)s), owner(own)

{
    if (owner)
    {
        ptr = new unsigned char[s];
        if (p)
            memcpy(ptr, p, size);
        else
            memset(ptr, 0, size);
        endFieldIndex = new short;
        if (endIndex != NULL)
            *endFieldIndex = *endIndex;
    }
    else
        endFieldIndex = endIndex;
}

autoMemory::~autoMemory()
{
    if (owner)
    {
        delete[] ptr;
        delete endFieldIndex;
    }
}

autoMemory::autoMemory(const autoMemory& p)
    : ptr(p.ptr), endFieldIndex(p.endFieldIndex), size(p.size), owner(p.owner)
{
    const_cast<autoMemory&>(p).owner = false;
}

autoMemory& autoMemory::operator=(const autoMemory& p)
{
    ptr = p.ptr;
    size = p.size;
    endFieldIndex = p.endFieldIndex;
    owner = p.owner;
    const_cast<autoMemory&>(p).owner = false;
    return *this;
}

//---------------------------------------------------------------------------
//    class memoryRecord
//---------------------------------------------------------------------------
inline memoryRecord::memoryRecord() : fieldsBase(NULL)
{
    m_memblock.reserve(ROW_MEM_BLOCK_RESERVE);
}

inline memoryRecord::memoryRecord(fielddefs& fdinfo) : fieldsBase(&fdinfo)
{
    m_memblock.reserve(ROW_MEM_BLOCK_RESERVE);
}

memoryRecord::memoryRecord(const memoryRecord& r)
    : fieldsBase(r.m_fns), m_memblock(r.m_memblock)
{
}

memoryRecord& memoryRecord::operator=(const memoryRecord& r)
{
     if (this != &r)
     {
         m_fns = r.m_fns;
         m_memblock = r.m_memblock;
     }
     return *this;
}

void memoryRecord::clear()
{
    for (int i = 0; i < (int)m_memblock.size(); ++i)
        memset(m_memblock[i].ptr, 0, m_memblock[i].size);

    m_fns->resetUpdateIndicator();
}

void memoryRecord::setRecordData(unsigned char* ptr, size_t size,
                                 short* endFieldIndex, bool owner)
{
    if ((size == 0) && owner)
    {
        size = m_fns->totalFieldLen();
        *endFieldIndex = (short)m_fns->size();
    }
    autoMemory am(ptr, size, endFieldIndex, owner);
    m_memblock.push_back(am);
}

void memoryRecord::copyToBuffer(table* tb, bool updateOnly) const
{
    if (!updateOnly)
        memcpy(tb->fieldPtr(0), ptr(0), m_fns->totalFieldLen());
    else
    {
        for (int i = 0; i < (int)m_fns->size(); ++i)
        {
            const fielddef& fd = (*m_fns)[i];
            // ptr() return memory block first address
            if (fd.enableFlags.bitE)
                memcpy(tb->fieldPtr(i), ptr(i) + fd.pos, fd.len);
        }
    }
}

memoryRecord* memoryRecord::create(fielddefs& fdinfo)
{
    memoryRecord* p = new memoryRecord(fdinfo);
    p->setAllocTypeThis(MEM_ALLOC_TYPE_ONE);
#ifdef DEBUG_TRACE_FIELDBASE_REFCOUNT
    _TCHAR tmp[50];
    wsprintf(tmp, _T("memoryRecord create one %p\n"), p);
    OutputDebugString(tmp);
#endif
    return p;
}

memoryRecord* memoryRecord::create(fielddefs& fdinfo, int n)
{
    assert(n);
    memoryRecord* p =  new memoryRecord[n];
    p->setFielddefs(&fdinfo);
    p->setAllocTypeThis(MEM_ALLOC_TYPE_ARRAY);

    for (int i = 1; i < n ; ++i)
    {
        memoryRecord* pp = p + i;
        pp->setFielddefs(&fdinfo);
        pp->setAllocParent(p);
    }
#ifdef DEBUG_TRACE_FIELDBASE_REFCOUNT
    _TCHAR tmp[50];
    wsprintf(tmp, _T("memoryRecord create n %p\n"), p);
    OutputDebugString(tmp);
#endif
    return p;
}

//copy constractor
memoryRecord* memoryRecord::create(const memoryRecord& m, int n)
{
    assert(n);
    memoryRecord* p =  new memoryRecord[n];
    *p = m;
    p->setAllocTypeThis(MEM_ALLOC_TYPE_ARRAY);
    for (int i = 1; i < n ; ++i)
    {
        memoryRecord* pp = p + i;
        *pp = m;
        pp->setAllocParent(p);
    }
#ifdef DEBUG_TRACE_FIELDBASE_REFCOUNT
    _TCHAR tmp[50];
    wsprintf(tmp, _T("memoryRecord create copy n %p\n"), p);
    OutputDebugString(tmp);
#endif
    return p;
}

void memoryRecord::releaseMemory()
{
    if (allocType() == MEM_ALLOC_TYPE_ONE)
        delete this;
    else
        delete [] this;
}

//---------------------------------------------------------------------------
//    class writableRecord
//---------------------------------------------------------------------------
writableRecord::writableRecord(table* tb, const aliasMap_type* alias)
    : memoryRecord(*fddefs()), m_tb(tb)
{
    m_tb->setFilter(NULL, 0, 0);
    m_tb->clearBuffer();
    m_fddefs->clear();
    m_fddefs->setAliases(alias);
    m_fddefs->copyFrom(m_tb);
    setRecordData(0, 0, &m_endIndex, true);
}

fielddefs* writableRecord::fddefs()
{
    m_fddefs = fielddefs::create();
    return m_fddefs;
}

writableRecord::~writableRecord()
{
    m_fddefs->release();
}

bool writableRecord::read(bool KeysetAlrady)
{
    if (!KeysetAlrady)
        copyToBuffer(m_tb);
    m_tb->seek();
    if (m_tb->stat())
        return false;
    copyFromBuffer(m_tb);
    return true;
}

void writableRecord::insert()
{
    copyToBuffer(m_tb);
    insertRecord(m_tb);
    copyFromBuffer(m_tb);
}

void writableRecord::del(bool KeysetAlrady)
{
    if (!KeysetAlrady)
        copyToBuffer(m_tb);
    m_tb->seek();
    if (m_tb->stat())
        nstable::throwError(_T("activeTable delete "), m_tb->stat());
    deleteRecord(m_tb);
}

void writableRecord::update()
{
    copyToBuffer(m_tb);
    m_tb->seek();
    if (m_tb->stat())
        nstable::throwError(_T("activeTable update "), m_tb->stat());
    copyToBuffer(m_tb, true /*only changed*/);
    updateRecord(m_tb);
}

void writableRecord::save()
{
    copyToBuffer(m_tb);
    m_tb->seek();
    if (m_tb->stat() == STATUS_NOT_FOUND_TI)
        insertRecord(m_tb);
    else
    {
        copyToBuffer(m_tb);
        updateRecord(m_tb);
    }
}

writableRecord* writableRecord::create(table* tb, const aliasMap_type* alias)
{
    writableRecord* p = new writableRecord(tb, alias);
    p->setAllocTypeThis(MEM_ALLOC_TYPE_ONE);
    
#ifdef DEBUG_TRACE_FIELDBASE_REFCOUNT
    _TCHAR tmp[50];
    wsprintf(tmp, _T("writableRecord create one %p\n"), p);
    OutputDebugString(tmp);
#endif
    return p;
}

void writableRecord::releaseMemory()
{
    if (allocType() == MEM_ALLOC_TYPE_ONE)
        delete this;
    else
        delete [] this;
}


} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs
