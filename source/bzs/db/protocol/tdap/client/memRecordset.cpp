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

#include "memRecordset.h"

#ifdef _DEBUG
#include <iostream>
#endif
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

// ---------------------------------------------------------------------------
// class multiRecordAlocatorImple
// ---------------------------------------------------------------------------
class multiRecordAlocatorImple : public multiRecordAlocator
{
    class recordset* m_rs;
    int m_rowOffset;
    int m_addType;
    int m_curFirstFiled;
    const std::vector<std::vector<int> >* m_joinRowMap;

public:
    inline multiRecordAlocatorImple(recordset* rs)
        : m_rs(rs), m_rowOffset(0), m_addType(0), m_curFirstFiled(0),
          m_joinRowMap(NULL)
    {
    }

    inline void init(size_t recordCount, size_t recordLen, int addType,
                     const table* tb)
    {
        m_rs->registerMemoryBlock(NULL, recordCount * recordLen, recordLen,
                                  addType | m_addType, tb);
    }

    inline unsigned char* ptr(size_t row, int stat)
    {
        int col = (stat == mra_current_block) ? m_curFirstFiled : 0;
        size_t rowNum = m_joinRowMap ? (*m_joinRowMap)[row + m_rowOffset][0]
                                     : row + m_rowOffset;
        return (*m_rs)[rowNum].ptr(col);
    }

    inline void setRowOffset(int v) { m_rowOffset = v; }

    inline void setJoinType(int v) { m_addType = v; }

    inline void setInvalidRecord(size_t row, bool v)
    {
        (*m_rs)[row + m_rowOffset].setInvalidRecord(v);
    }

    inline void setCurFirstFiled(int v) { m_curFirstFiled = v; }

    inline void setJoinRowMap(const std::vector<std::vector<int> >* v)
    {
        m_joinRowMap = v; /*m_joinMapSize = size;*/
    }

    inline const std::vector<std::vector<int> >* joinRowMap() const
    {
        return m_joinRowMap;
    }
};

// ---------------------------------------------------------------------------
// class recordsetSorter
// ---------------------------------------------------------------------------
class recordsetSorter
{
    const std::vector<int>& m_fieldNums;

public:
    recordsetSorter(const std::vector<int>& fieldNums) : m_fieldNums(fieldNums)
    {
    }
    bool operator()(const row_ptr& l, const row_ptr r) const
    {
        std::vector<int>::const_iterator it = m_fieldNums.begin();
        while (it != m_fieldNums.end())
        {
            int ret = (*l)[*it].comp((*r)[*it], 0);
            if (ret)
                return (ret < 0);
            ++it;
        }
        return false;
    }
};

// ---------------------------------------------------------------------------
// class recordset
// ---------------------------------------------------------------------------
recordset::recordset()
    : m_fds(fielddefs::create(), &fielddefs::destroy), m_joinRows(0),
      m_uniqueReadMaxField(0)
{
    m_mra.reset(new multiRecordAlocatorImple(this));
}

recordset::~recordset()
{
}

void recordset::registerMemoryBlock(unsigned char* ptr, size_t size,
                                    size_t recordLen, int addtype,
                                    const table* tb)
{
    autoMemory* am = new autoMemory(ptr, size, 0, true);
    m_memblock.push_back(boost::shared_ptr<autoMemory>(am));
    unsigned char* p = am->ptr;
    // copy fileds
    if (addtype & mra_nextrows)
    {
        if (addtype == mra_nextrows)
            m_mra->setRowOffset((int)m_recordset.size()); // no join
        else
            m_mra->setRowOffset((int)m_joinRows); // Join
    }
    else
    {
        // assert(tb);
        m_joinRows = 0;
        m_mra->setRowOffset(0);
        m_mra->setCurFirstFiled((int)m_fds->size());
        if (tb)
            m_fds->copyFrom(tb);
        if (tb && (addtype == mra_nojoin))
        {
            const keydef& kd = tb->tableDef()->keyDefs[tb->keyNum()];
            m_uniqueReadMaxField =
                (kd.segments[0].flags.bit0 == false) ? (short)m_fds->size() : 0;
        }
    }

    *(am->endFieldIndex) = (short)m_fds->size();
    size_t rows = size / recordLen;

    // set record pointer to each record
    if ((addtype & mra_innerjoin) || (addtype & mra_outerjoin))
    {
        // Join optimazing
        const std::vector<std::vector<int> >* jmap = m_mra->joinRowMap();

        if (jmap)
        {
            // At Join that if some base records reference to a joined record
            //		that the joined record pointer is shared by some base
            // records.
            for (int i = 0; i < (int)rows; ++i)
            {
                const std::vector<int>& map = (*jmap)[i + m_joinRows];
                for (int j = 0; j < (int)map.size(); ++j)
                    m_recordset[map[j]]->setRecordData(
                        p + recordLen * i, 0, am->endFieldIndex, false);
            }
        }
        else
        {
            for (int i = 0; i < (int)rows; ++i)
                m_recordset[i + m_joinRows]->setRecordData(
                    p + recordLen * i, 0, am->endFieldIndex, false);
        }
        m_joinRows += rows;
    }
    else
    { // create new record
        size_t reserveSize = m_recordset.size() + rows;
        m_recordset.reserve(reserveSize);
        for (int i = 0; i < (int)rows; ++i)
        {
            row_ptr rec(memoryRecord::create(*m_fds), &memoryRecord::release);
            rec->setRecordData(p + recordLen * i, 0, am->endFieldIndex, false);
            m_recordset.push_back(rec);
        }
    }
}

void recordset::makeSortFields(const _TCHAR* name, std::vector<int>& fieldNums)
{
    int index = m_fds->indexByName(name);
    if (index == -1)
        THROW_BZS_ERROR_WITH_MSG(_T("oorderBy:Invalid field name"));
    fieldNums.push_back(index);
}

int recordset::getMemBlockIndex(unsigned char* ptr) const
{
    for (int i = 0; i < (int)m_memblock.size(); ++i)
    {
        const boost::shared_ptr<autoMemory>& am = m_memblock[i];
        if ((ptr >= am->ptr) && (ptr < am->ptr + am->size))
            return i;
    }
    assert(0);
    return -1;
}

/* This clone is deep copy.
   But text and blob field data memory are shared.
*/
recordset* recordset::clone() const
{
    recordset* p = new recordset();
    p->m_joinRows = m_joinRows;
    p->m_uniqueReadMaxField = m_uniqueReadMaxField;
    p->m_unionFds = m_unionFds;
    p->m_fds.reset(m_fds->clone(), &fielddefs::destroy);

    std::vector<__int64> offsets;
    for (int i = 0; i < (int)m_memblock.size(); ++i)
    {
        autoMemory* am =
            new autoMemory(m_memblock[i]->ptr, m_memblock[i]->size, 0, true);
        *am->endFieldIndex = *m_memblock[i]->endFieldIndex;
        p->m_memblock.push_back(boost::shared_ptr<autoMemory>(am));
        offsets.push_back((__int64)(am->ptr - m_memblock[i]->ptr));
    }

    for (int i = 0; i < (int)m_recordset.size(); ++i)
    {
        memoryRecord* row = dynamic_cast<memoryRecord*>(m_recordset[i].get());
        memoryRecord* mr = memoryRecord::create(*p->m_fds);
        row_ptr rec(mr, &memoryRecord::release);
        p->m_recordset.push_back(rec);

        for (int j = 0; j < (int)row->memBlockSize(); ++j)
        {
            const autoMemory& mb = row->memBlock(j);
            int index = getMemBlockIndex(mb.ptr);
            unsigned char* ptr = mb.ptr + offsets[index];
            const boost::shared_ptr<autoMemory>& am = p->m_memblock[index];
            mr->setRecordData(ptr, mb.size, am->endFieldIndex, mb.owner);
        }
    }
    return p;
}

void recordset::clearRecords()
{
    m_recordset.clear();
    m_uniqueReadMaxField = 0;
}

void recordset::clear()
{
    clearRecords();
    m_fds->clear();
    m_unionFds.clear();
    m_memblock.clear();
}

recordset& recordset::top(recordset& c, int n)
{
    c = *this;
    c.clearRecords();
    for (int i = 0; i < n; ++i)
        c.push_back(m_recordset[i]);
    return c;
}

recordset::iterator recordset::begin()
{
    return m_recordset.begin();
}

recordset::iterator recordset::end()
{
    return m_recordset.end();
}

recordset::iterator recordset::erase(size_t index)
{
    return m_recordset.erase(m_recordset.begin() + index);
}

recordset::iterator recordset::erase(const iterator& it)
{
    return m_recordset.erase(it);
}

void recordset::push_back(row_ptr r)
{
    m_recordset.push_back(r);
};

void recordset::readBefore(const table_ptr tb, const aliasMap_type* alias)
{
    tb->setMra(m_mra.get());
    m_fds->setAliases(alias);
}

recordset::key_type recordset::resolvKeyValue(const std::_tstring& name,
                                              bool noexception)
{
    int index = m_fds->indexByName(name);
    if (index != -1)
        return index;
    if (!noexception)
        THROW_BZS_ERROR_WITH_MSG(_T("groupQuery:Invalid key name"));

    return (key_type)m_fds->size();
}

void recordset::removeField(int index)
{
    m_fds->remove(index);
    for (int i = 0; i < (int)m_unionFds.size(); ++i)
        m_unionFds[i]->remove(index);

    for (int i = 0; i < (int)m_memblock.size(); ++i)
    {
        if (*(m_memblock[i]->endFieldIndex) > index)
        {
            short v = *(m_memblock[i]->endFieldIndex) - 1;
            *(m_memblock[i]->endFieldIndex) = v;
        }
    }
}

recordset& recordset::matchBy(recordsetQuery& rq)
{
    rq.init(fieldDefs());
    for (int i = (int)m_recordset.size() - 1; i >= 0; --i)
        if (!rq.match(m_recordset[i]))
            erase(i);
    return *this;
}

recordset& recordset::groupBy(groupQuery& gq)
{
    gq.grouping(*this);
    return *this;
}

recordset& recordset::orderBy(const _TCHAR* name1, const _TCHAR* name2,
                              const _TCHAR* name3, const _TCHAR* name4,
                              const _TCHAR* name5, const _TCHAR* name6,
                              const _TCHAR* name7, const _TCHAR* name8)
{
    std::vector<int> fieldNums;
    makeSortFields(name1, fieldNums);
    if (name2)
        makeSortFields(name2, fieldNums);
    if (name3)
        makeSortFields(name3, fieldNums);
    if (name4)
        makeSortFields(name4, fieldNums);
    if (name5)
        makeSortFields(name5, fieldNums);
    if (name6)
        makeSortFields(name6, fieldNums);
    if (name7)
        makeSortFields(name7, fieldNums);
    if (name8)
        makeSortFields(name8, fieldNums);
    std::sort(begin(), end(), recordsetSorter(fieldNums));
    return *this;
}

inline recordset& recordset::reverse()
{
    std::reverse(begin(), end());
    return *this;
}

void recordset::appendCol(const _TCHAR* name, int type, short len)
{
    assert(m_fds->size());
    fielddef fd((*m_fds)[0]);
    fd.len = len;
    fd.pos = 0;
    fd.type = type;
    fd.setName(name);
    m_fds->push_back(&fd);
    for (int i = 0; i < (int)m_unionFds.size(); ++i)
        m_unionFds[i]->push_back(&fd);
    registerMemoryBlock(NULL, fd.len * size(), fd.len, mra_outerjoin);
}

recordset& recordset::operator+=(const recordset& r)
{
    if (!m_fds->canUnion(*r.m_fds))
        THROW_BZS_ERROR_WITH_MSG(_T("Recordsets are different format"));

    m_recordset.reserve(m_recordset.size() + r.size());
    m_unionFds.push_back(r.m_fds);
    for (size_t i = 0; i < r.size(); ++i)
        m_recordset.push_back(r.m_recordset[i]);
    for (size_t i = 0; i < r.m_memblock.size(); ++i)
        m_memblock.push_back(r.m_memblock[i]);
    return *this;
}

#ifdef _DEBUG
void recordset::dump()
{
    const fielddefs& fields = *fieldDefs();
    for (int j = 0; j < (int)fields.size(); ++j)
        std::tcout << fields[j].name() << _T("\t");
    std::tcout << _T("\n");

    for (int i = 0; i < (int)size(); ++i)
    {
        row& m = (operator[](i));
        for (int j = 0; j < (int)m.size(); ++j)
        {
            std::tcout << m[(short)j].c_str() << _T("\t");
            if (j == (int)m.size() - 1)
                std::tcout << _T("\n");
        }
    }
}
#endif

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs
