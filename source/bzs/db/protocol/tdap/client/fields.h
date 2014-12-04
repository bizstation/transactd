#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_FIELDS_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_FIELDS_H
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
#include "field.h"
#include "table.h"
#include <boost/shared_ptr.hpp>
#include <stdio.h>
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
#define MEM_ALLOC_TYPE_NONE   0
#define MEM_ALLOC_TYPE_ONE    1
#define MEM_ALLOC_TYPE_ARRAY  2

class refarymem
{
    union
    {
        refarymem* m_parent;
        int m_refCount;
    };

    bool m_child;
    unsigned char m_allocType;

    virtual void releaseMemory() = 0;

protected:

    inline int allocType() {return m_allocType;}

public:

    refarymem():m_parent(NULL), m_child(0), m_allocType(MEM_ALLOC_TYPE_NONE){}
    
    inline void setAllocParent(refarymem* v) 
    { 
        m_parent = v;
        m_child = (v != NULL);
    }
    
    void setAllocTypeThis(int v) { m_allocType = ( unsigned char)v; }
    
    void addref()
    {
        if (m_child)
            m_parent->addref();
        else if (m_allocType)
            ++m_refCount;
    }

    void release()
    {
        if (m_child)
            m_parent->release();
        else
        {
            --m_refCount;
#ifdef DEBUG_TRACE_FIELDBASE_REFCOUNT
            if (m_allocType != MEM_ALLOC_TYPE_ONE)
            {    
                _TCHAR tmp[100];
                wsprintf(tmp, _T("refarymem::m_refCount %ld \t%p\n"), m_refCount, this);
                OutputDebugString(tmp);
            }
#endif
            if (m_refCount == 0)
            {
#ifdef DEBUG_TRACE_FIELDBASE_REFCOUNT
                _TCHAR tmp[50];
                if (m_allocType == MEM_ALLOC_TYPE_ONE)
                    wsprintf(tmp, _T("refarymem release one %p\n"), this);
                else
                    wsprintf(tmp, _T("refarymem release n %p\n"), this);
                OutputDebugString(tmp);
#endif
                releaseMemory();
            }
        }
    }
};
/** @endcond */

class autoMemory;

/* copyable */
class fieldsBase : public refarymem
{
    friend class multiRecordAlocatorImple;
    friend class recordsetImple;
    friend class recordsetQuery;

    virtual unsigned char* ptr(int index) const = 0;

protected:
    /** @cond INTERNAL */
    fielddefs* m_fns;
    bool m_invalidRecord;
    virtual table* tbptr() const { return NULL; }

    void throwIndexError(short index) const
    {
        if (tbptr())
        {
            tbptr()->setStat(STATUS_INVARID_FIELD_IDX);
            nstable::throwError(_T("field access"), tbptr());
        }
        else
        {
            _TCHAR tmp[100];
            _stprintf_s(tmp, 100, _T("field access index %d of %s"), index,
                        tbptr() ? tbptr()->tableDef()->tableName() : _T(""));
            nstable::throwError(tmp, STATUS_INVARID_FIELD_IDX);
        }
    }
    
    explicit inline fieldsBase(fielddefs* fns)
        : refarymem(), m_fns(fns), m_invalidRecord(false)
    {
    }

    inline void setFielddefs(fielddefs* def) { m_fns = def; }

    virtual void removeLastMemBlock(){};

    virtual void setRecordData(autoMemory* am, unsigned char* ptr, size_t size,
                               short* endFieldIndex, bool owner = false){};
    /** @endcond */
public:

    virtual ~fieldsBase(){};

    inline void setInvalidRecord(bool v) { m_invalidRecord = v; }

    inline bool isInvalidRecord() const { return m_invalidRecord; }

    inline field getFieldNoCheck(short index) const
    {
        return field(ptr((short)index), (*m_fns)[(short)index], m_fns);
    }

    inline field operator[](short index) const
    {
        if (m_fns->checkIndex(index))
            return field(ptr((short)index), (*m_fns)[(short)index], m_fns);

        throwIndexError(index);
        return field(NULL, dummyFd(), m_fns);
    }

    inline field operator[](const _TCHAR* name) const
    {
        int index = m_fns->indexByName(name);
        return operator[](index);
    }

    inline field operator[](const std::_tstring& name) const
    {
        return operator[](name.c_str());
    }

    inline size_t size() const { return m_fns->size(); }

    inline field fd(short index) const { return operator[](index); }

    inline field fd(const _TCHAR* name) const
    {
        int index = m_fns->indexByName(name);
        return operator[](index);
    }

    inline short indexByName(const _TCHAR* name) const
    {
        return m_fns->indexByName(name);
    }

    inline const fielddefs* fieldDefs() const { return m_fns; }

    virtual void clear() = 0;
};

typedef boost::shared_ptr<database> database_ptr;
typedef boost::shared_ptr<table> table_ptr;
typedef fieldsBase row;
typedef row* row_ptr;

/* copyable*/
class fields : public fieldsBase
{
    table& m_tb;

    void releaseMemory(){}

    inline unsigned char* ptr(int index) const
    {
        return (unsigned char*)m_tb.data();
    }

    table* tbptr() const { return &m_tb; }

    inline explicit fields() : fieldsBase(NULL), m_tb(*((table*)0))
    {
    }

public:
    inline explicit fields(table& tb) : fieldsBase(tb.m_fddefs), m_tb(tb) {}

    inline explicit fields(table_ptr tb)
        : fieldsBase((*tb).m_fddefs), m_tb(*tb)
    {
    }

    inline void clear() { m_tb.clearBuffer(); }

    inline table& tb() const { return m_tb; }

    inline short inproc_size() const { return m_tb.getCurProcFieldCount(); }

    inline field inproc_fd(short index) const
    {
        return operator[](m_tb.getCurProcFieldIndex(index));
    }
};

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#endif // BZS_DB_PROTOCOL_TDAP_CLIENT_FIELDS_H
