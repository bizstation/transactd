//---------------------------------------------------------------------------

#pragma hdrstop

#include "memRecord.h"
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


autoMemory::autoMemory(unsigned char* p, size_t s, short endIndex, bool own)
        :ptr(p), size(s),endFieldIndex(endIndex), owner(own)

{
    if (owner)
    {
        ptr = new char[s];
        if (p)
            memcpy(ptr, p, size);
        else
            memset(ptr, 0, size);
    }
}
autoMemory::~autoMemory()
{
    if (owner)
        delete [] ptr;
}

autoMemory::autoMemory(const autoMemory& p)
         :ptr(p.ptr), size(p.size)
        ,endFieldIndex(p.endFieldIndex), owner(p.owner)
{
    const_cast<autoMemory&>(p).owner = false;
    const_cast<autoMemory&>(p).ptr = NULL;
}

autoMemory& autoMemory::operator=(const autoMemory& p)
{
    ptr = p.ptr;
	size = p.size;
	endFieldIndex = p.endFieldIndex;
    owner = p.owner;
    const_cast<autoMemory&>(p).owner = false;
    const_cast<autoMemory&>(p).ptr = NULL;

}


//---------------------------------------------------------------------------
//    class memoryRecord
//---------------------------------------------------------------------------

memoryRecord::memoryRecord(fieldInfo& fdinfo): fieldsBase(fdinfo)
{
    m_memblock.reserve(ROW_MEM_BLOCK_RESERVE);
}

memoryRecord::~memoryRecord()
{

}

void memoryRecord::clear()
{
    for (int i=0;i<(int)m_memblock.size();++i)
        memset(m_memblock[i].ptr, 0, m_memblock[i].size);
}

void memoryRecord::setRecordData(unsigned char* ptr, size_t size
        , short endFieldIndex, bool owner)
{
    m_memblock.push_back(autoMemory(ptr, size, endFieldIndex, owner));
}

/* return memory block first address which not field ptr address */
/*unsigned char* memoryRecord::ptr(int index) const
{
    for (int i=0;i<(int)m_memblock.size();++i)
        if (m_memblock[i].endFieldIndex > index)
            return 	m_memblock[i].ptr;
    assert(0);
    return NULL;
}

const autoMemory&  memoryRecord::memBlock(int index) const
{
    for (int i=0;i<(int)m_memblock.size();++i)
        if (m_memblock[i].endFieldIndex > index)
            return m_memblock[i];
    assert(0);
}

int memoryRecord::memBlockSize() const
{
    return (int) m_memblock.size();
} */

memoryRecord* memoryRecord::create(fieldInfo& fdinfo)
{
    return new memoryRecord(fdinfo);
}

void memoryRecord::release(memoryRecord* p)
{
    delete p;

}

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs

