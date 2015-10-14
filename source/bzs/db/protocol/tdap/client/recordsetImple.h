#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_MEMRECORDSETIMPLE_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_MEMRECORDSETIMPLE_H
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
#include "trdormapi.h"
#include "groupQuery.h"
#ifdef _DEBUG
#include <iostream>
#include <iomanip>
#endif

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

#ifdef _DEBUG

/*
 Print record set whole row like a mysql command line tool.
*/
template <class RS>
class dumpRecordset
{
    std::vector<size_t> m_widths;
    std::vector<std::ios::fmtflags> m_ailgns;

    void cacheWidthAndAlign(RS& rs)
    {
        const fielddefs& fds = *rs.fieldDefs();
        m_widths.clear();
        m_widths.resize(fds.size());
        m_ailgns.clear();
        m_ailgns.resize(fds.size());

        for (size_t col = 0; col < fds.size(); ++col)
        {
            m_widths[col] = std::max(_tcslen(fds[col].name()), m_widths[col]);
            m_ailgns[col] = fds[col].isStringType() ? std::ios::left : std::ios::right;
        }
        for(size_t i = 0; i < rs.size(); ++i)
        {
            row& rec = rs[i];
            for (size_t col = 0; col < fds.size(); ++col)
                m_widths[col] = std::max(_tcslen(rec[col].c_str()), m_widths[col]);
        }
    }

    std::_tstring makeLine()
    {
        std::_tstring s = _T("+");
        for (size_t i = 0;i < m_widths.size(); ++i)
        {
            s += std::_tstring(m_widths[i] + 2, _T('-'));
            s += _T('+');
        }
        s += _T("\n");
        return s;
    }

    void printValue(size_t width, std::ios::fmtflags f, const _TCHAR* value)
    {
        std::tcout.setf(f, std::ios::adjustfield);
        std::tcout << _T(" ")  << std::setw(width) << value << _T(" |");
    }

    const _TCHAR* value(const fielddef& fd) {return fd.name();}
    const _TCHAR* value(const field& fd) {return fd.c_str();}

    template <class T>
    void printRecord(const T& coll)
    {
        std::tcout  << _T("|");
        for (size_t col = 0; col < m_widths.size(); ++col)
            printValue(m_widths[col], m_ailgns[col], value(coll[col]));
        std::tcout << std::endl;
    }
public:
    void operator()(RS& rs)
    {
        cacheWidthAndAlign(rs);
        std::_tstring line = makeLine();
        //header
        std::tcout << line;
        printRecord(*rs.fieldDefs());
        std::tcout << line;

        //field value
        for(size_t i = 0; i < rs.size(); ++i)
            printRecord(rs[i]);
        std::tcout << line;
    }
};
#endif


struct sortDescription
{
    short index;
    bool asc;
};

class recordsetSorter
{
    const sortDescription* m_begin;
    const sortDescription* m_end;
public:

    recordsetSorter(sortDescription* begin, sortDescription* end)
        : m_begin(begin), m_end(end)
    {
    }

    bool operator()(const row_ptr& l, const row_ptr r) const
    {
        const sortDescription* it = m_begin;
        while (it != m_end)
        {
            int ret = (*l)[it->index].comp((*r)[it->index], 0);
            if (ret)
                return it->asc ? (ret < 0) : (ret > 0);
            ++it;
        }
        return false;
    }
};

class multiRecordAlocatorImple : public multiRecordAlocator
{
    class recordsetImple* m_rs;
    const std::vector<std::vector<int> >* m_joinRowMap;
    int m_rowOffset;
    int m_addType;
    int m_curFirstField;

public:
    inline multiRecordAlocatorImple(recordsetImple* rs);
    inline void init(size_t recordCount, size_t recordLen,
                     int addType, const table* tb);
    inline unsigned char* allocBlobBlock(size_t size);
    inline unsigned char* ptr(size_t row, int stat);
    inline void setRowOffset(int v) { m_rowOffset = v; }
    inline void setJoinType(int v) { m_addType = v; }
    inline void setInvalidRecord(size_t row, bool v);
    inline void setCurFirstField(int v) { m_curFirstField = v; }
    inline void setJoinRowMap(const std::vector<std::vector<int> >* v)
    {
        m_joinRowMap = v;
    }
    inline const std::vector<std::vector<int> >* joinRowMap() const
    {
        return m_joinRowMap;
    }
    inline void duplicateRow(int row, int count);
    inline void removeLastMemBlock(int row);
};

class recordsetImple
{
    friend class multiRecordAlocatorImple;
    boost::shared_ptr<fielddefs> m_fds;
    boost::shared_ptr<multiRecordAlocatorImple> m_mra;
    std::vector<row_ptr> m_recordset;
    std::vector<boost::shared_ptr<autoMemory> > m_memblock;

    /* for registerMemoryBlock temp data */
    size_t m_joinRows;

    /*
            for optimazing join.
            If the first reading is using by unique key , set that field count.
    */
    short m_uniqueReadMaxField;

public:
    typedef std::vector<row_ptr>::iterator iterator;
    typedef row_ptr item_type;
private:
    unsigned char* allocBlobBlock(size_t size)
    {
        autoMemory* am = autoMemory::create();
        am->addref();
        am->setParams(NULL, size, 0, true);
        m_memblock.push_back(boost::shared_ptr<autoMemory>(am, boost::bind(&autoMemory::release, _1)));
        return am->ptr;
    }

    void registerMemoryBlock(unsigned char* ptr, size_t size, size_t recordLen,
                             int addtype, const table* tb = NULL)
    {
        autoMemory* am = autoMemory::create();
        am->addref();
        am->setParams(ptr, size, 0, true);
        m_memblock.push_back(boost::shared_ptr<autoMemory>(am, boost::bind(&autoMemory::release, _1)));
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
            m_joinRows = 0;
            m_mra->setRowOffset(0);
            m_mra->setCurFirstField((int)m_fds->size());
            if (tb)
                m_fds->copyFrom(tb);
            if (tb && (addtype == mra_nojoin))
            {
                const keydef& kd = tb->tableDef()->keyDefs[(int)tb->keyNum()];
                m_uniqueReadMaxField = (kd.segments[0].flags.bit0 == false)
                                           ? (short)m_fds->size()
                                           : 0;
            }
        }

        *(am->endFieldIndex) = (short)m_fds->size();
        size_t rows = size / recordLen;

        // set record pointer to each record
        if ((addtype & mra_innerjoin) || (addtype & mra_outerjoin))
        {
            // Join optimazing
            const std::vector<std::vector<int> >* jmap = m_mra->joinRowMap();
            autoMemory* ama = autoMemory::create((int)rows);
            if (jmap)
            {
                // At Join that if some base records reference to a joined
                // record
                //      that the joined record pointer is shared by some
                // base records.
                for (int i = 0; i < (int)rows; ++i)
                {
                    const std::vector<int>& map = (*jmap)[i + m_joinRows];
                    for (int j = 0; j < (int)map.size(); ++j)
                    {
                        autoMemory* a = ama + i;
                        m_recordset[map[j]]->setRecordData(a,
                            p + recordLen * i, 0, am->endFieldIndex, false);
                    }
                }
            }
            else
            {
                for (int i = 0; i < (int)rows; ++i)
                {
                    autoMemory* a = ama + i;
                    m_recordset[i + m_joinRows]->setRecordData(a,
                        p + recordLen * i, 0, am->endFieldIndex, false);
                }
            }
            m_joinRows += rows;
        }
        else
        { // create new record
            size_t reserveSize = m_recordset.size() + rows;
            m_recordset.reserve(reserveSize);
            memoryRecord* rec = memoryRecord::create(*m_fds, (int)rows);
            autoMemory* ama = autoMemory::create((int)rows);
            for (int i = 0; i < (int)rows; ++i)
            {
                autoMemory* a = ama + i;
                rec[i].setRecordData(a, p + recordLen * i, 0, am->endFieldIndex,
                                   false);
                push_back(&rec[i]);
            }
        }
    }

    void makeSortFields(const _TCHAR* name, sortDescription* sd, bool asc = true)
    {
        sd->index = m_fds->indexByName(name);
        if (sd->index == -1)
            THROW_BZS_ERROR_WITH_MSG(_T("orderBy:Invalid field name"));
        sd->asc = asc;
    }

    int getMemBlockIndex(unsigned char* ptr) const
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

    // Duplicate row for hasManyJoin
    void duplicateRow(int row, int count)
    {
        row_ptr& r = m_recordset[row];
        memoryRecord* p = dynamic_cast<memoryRecord*>(r);
        m_recordset.reserve(m_recordset.size() + count);
        memoryRecord* rec = memoryRecord::create(*p, count);
        for (int i = 0; i < count; ++i)
        {
            rec[i].addref();
            m_recordset.insert(m_recordset.begin() + row + i, &rec[i]);
        }
    }

public:
    inline recordsetImple()
        : m_fds(fielddefs::create(), boost::bind(&fielddefs::release, _1)),
          m_joinRows(0), m_uniqueReadMaxField(0)
    {
        m_mra.reset(new multiRecordAlocatorImple(this));
    }

    inline recordsetImple(const recordsetImple& r)
        : m_fds(r.m_fds),m_mra(r.m_mra), m_recordset(r.m_recordset),
          m_memblock(r.m_memblock), m_joinRows(r.m_joinRows),
          m_uniqueReadMaxField(r.m_uniqueReadMaxField)
    {
        for (size_t i = 0; i < m_recordset.size(); ++i)
            m_recordset[i]->addref();
    }

    inline ~recordsetImple() { clearRecords(); }

    inline void checkIndex(size_t index)
    {
        if (index >= m_recordset.size())
            THROW_BZS_ERROR_WITH_MSG(_T("Invalid row index of recordset."));
    }

    inline recordsetImple& operator=(const recordsetImple& r)
    {
        if (this != &r)
        {
            m_fds = r.m_fds;
            m_mra = r.m_mra;
            m_recordset = r.m_recordset;
            m_memblock = r.m_memblock;
            m_joinRows = r.m_joinRows;
            m_uniqueReadMaxField = r.m_uniqueReadMaxField;
            for (size_t i = 0; i < m_recordset.size(); ++i)
                m_recordset[i]->addref();
        }
        return *this;
    }

    /* This clone is deep copy.
       But text and blob field data memory are shared.
    */
    inline recordsetImple* clone() const
    {
        recordsetImple* p = new recordsetImple();
        p->m_joinRows = m_joinRows;
        p->m_uniqueReadMaxField = m_uniqueReadMaxField;
        p->m_fds.reset(m_fds->clone(), boost::bind(&fielddefs::release, _1));

        std::vector<size_t> offsets;
        if (m_memblock.size())
        {
            autoMemory* ama = autoMemory::create((int)m_memblock.size());
            for (int i = 0; i < (int)m_memblock.size(); ++i)
            {
                autoMemory* am = ama + i;
                am->addref();
                am->setParams(m_memblock[i]->ptr, m_memblock[i]->size, 0, true);
                *am->endFieldIndex = *m_memblock[i]->endFieldIndex;
                p->m_memblock.push_back(boost::shared_ptr<autoMemory>(am,  boost::bind(&autoMemory::release, _1)));
                offsets.push_back((am->ptr - m_memblock[i]->ptr));
            }
        }
        if (m_recordset.size())
        {
            p->m_recordset.reserve(m_recordset.size());
            memoryRecord* recs = memoryRecord::create(*p->m_fds, (int)m_recordset.size());
            int amindex = 0;
            for (int i = 0; i < (int)m_recordset.size(); ++i)
            {
                memoryRecord* row = dynamic_cast<memoryRecord*>(m_recordset[i]);
                amindex += (int)row->memBlockSize();
            }
            autoMemory* amar = autoMemory::create(amindex);
            amindex = 0;
            std::vector<short> blobs;
            std::vector<short> offsetIndex;
            for (int j = 0; j < (int)m_fds->size(); ++j)
            {
                if (m_fds->operator[](j).blobLenBytes())
                {
                    blobs.push_back((short)j);
                    unsigned char* p = (unsigned char*)(*m_recordset[0])[j].ptr()
                                        + m_fds->operator[](j).blobLenBytes();
                    short index = (short)getMemBlockIndex(p);
                    offsetIndex.push_back(index);
                }
            }
            for (int i = 0; i < (int)m_recordset.size(); ++i)
            {
                memoryRecord* row =
                    dynamic_cast<memoryRecord*>(m_recordset[i]);
                memoryRecord* mr = recs + i;
                p->push_back(mr);
                mr->m_invalidRecord = row->m_invalidRecord;
                for (int j = 0; j < (int)row->memBlockSize(); ++j)
                {
                    const autoMemory& mb = row->memBlock(j);
                    int index = getMemBlockIndex(mb.ptr);
#pragma warn -8072
                    unsigned char* ptr = mb.ptr + offsets[index];
#pragma warn .8072
                    autoMemory* a = amar + amindex;
                    const boost::shared_ptr<autoMemory>& am = p->m_memblock[index];
                    mr->setRecordData(a, ptr, mb.size, am->endFieldIndex, mb.owner);
                    ++amindex;
                }

                for (int j = 0; j < (int)blobs.size(); ++j)
                    row->getFieldNoCheck(blobs[j])
                        .offsetBlobPtr(offsets[offsetIndex[j]]);
            }
        }
        return p;
    }

    inline short uniqueReadMaxField() const { return m_uniqueReadMaxField; }

    inline void clearRecords()
    {
        if (m_recordset.size())
        {
            if (!m_recordset[0]->tryFastRelease((int)m_recordset.size()))
            {
                for (int i = (int)m_recordset.size() - 1; i >= 0; --i)
                    m_recordset[i]->release();
            }
        } 
        
        m_recordset.clear();
        m_uniqueReadMaxField = 0;
    }

    inline const fielddefs* fieldDefs() const { return m_fds.get(); }

    inline void clear()
    {
        if (m_memblock.size())
        {
            clearRecords();
            m_fds->clear();
            m_memblock.clear();
        }
    }

    inline row_ptr& getRow(size_t index) { return m_recordset[index]; }

    inline row& operator[](size_t index) const
    {
        return *m_recordset[index];
    }

    inline row& first() const
    {
        if (m_recordset.size() == 0)
            THROW_BZS_ERROR_WITH_MSG(_T("Invalid index of recordset."));
        return *m_recordset[0];
    }

    inline row& last() const
    {
        if (m_recordset.size() == 0)
            THROW_BZS_ERROR_WITH_MSG(_T("Invalid index of recordset."));
        return *m_recordset[m_recordset.size() - 1];
    }

    inline recordsetImple& top(recordsetImple& c, int n) const
    {
        c = *this;
        c.clearRecords();
        n = std::min(n, (int)m_recordset.size());
        for (int i = 0; i < n; ++i)
            c.push_back(m_recordset[i]);
        return c;
    }

    inline iterator begin() { return m_recordset.begin(); }

    inline iterator end() { return m_recordset.end(); }

    inline iterator erase(size_t index)
    {
        return erase(m_recordset.begin() + index);
    }

    inline iterator erase(const iterator& it) 
    { 
        m_recordset[it - m_recordset.begin()]->release();
        return m_recordset.erase(it); 
    }

    inline void push_back(row_ptr r)
    {
        r->addref();
        m_recordset.push_back(r);
    }

    inline size_t size() const { return m_recordset.size(); }

    inline size_t count() const { return m_recordset.size(); }

    void readBefore(const table_ptr tb, const aliasMap_type* alias)
    {
        tb->setMra(m_mra.get());
        m_fds->setAliases(alias);
    }

    typedef fielddefs header_type;
    typedef int key_type;
    typedef row_ptr row_type;
    typedef row row_pure_type;

    key_type resolvKeyValue(const std::_tstring& name, bool noexception = false)
    {
        int index = m_fds->indexByName(name);
        if (index != -1)
            return index;
        if (!noexception)
        {
            _TCHAR tmp[256];
            _stprintf_s(tmp, 256, _T("groupQuery:Invalid key name '%s'."), name.c_str());
            THROW_BZS_ERROR_WITH_MSG(tmp);
        }
        return (key_type)m_fds->size();
    }

    inline void removeField(int index)
    {
        m_fds->remove(index);

        for (int i = 0; i < (int)m_memblock.size(); ++i)
        {
            if (*(m_memblock[i]->endFieldIndex) > index)
            {
                short v = *(m_memblock[i]->endFieldIndex) - 1;
                *(m_memblock[i]->endFieldIndex) = v;
            }
        }
    }

    inline recordsetImple& matchBy(recordsetQuery& rq)
    {
        m_fds->setAliases(NULL);
        if (m_recordset.size())
        {
            rq.init(fieldDefs());
            for (int i = (int)m_recordset.size() - 1; i >= 0; --i)
                if (!rq.match(m_recordset[i]))
                    erase(i);
        }
        return *this;
    }

    inline recordsetImple& groupBy(groupQuery& gq)
    {
        m_fds->setAliases(NULL);
        if (m_recordset.size())
            gq.grouping(*this);
        return *this;
    }

    inline recordsetImple&
    orderBy(const _TCHAR* name1, const _TCHAR* name2 = NULL,
            const _TCHAR* name3 = NULL, const _TCHAR* name4 = NULL,
            const _TCHAR* name5 = NULL, const _TCHAR* name6 = NULL,
            const _TCHAR* name7 = NULL, const _TCHAR* name8 = NULL)
    {
        if (m_recordset.size() > 1)
        {
            sortDescription sds[9];
            sortDescription* sd = &sds[0];
            makeSortFields(name1, sd, true);
            if (name2)
                makeSortFields(name2, ++sd, true);
            if (name3)
                makeSortFields(name3, ++sd, true);
            if (name4)
                makeSortFields(name4, ++sd, true);
            if (name5)
                makeSortFields(name5, ++sd, true);
            if (name6)
                makeSortFields(name6, ++sd, true);
            if (name7)
                makeSortFields(name7, ++sd, true);
            if (name8)
                makeSortFields(name8, ++sd, true);
            std::sort(begin(), end(), recordsetSorter(&sds[0], ++sd));
        }
        return *this;
    }

    inline recordsetImple& orderBy(const sortFields& orders)
    {
        if (m_recordset.size() > 1)
        {
            if (orders.size() > 8)
                THROW_BZS_ERROR_WITH_MSG(_T("orderBy:Too many keys"));
            sortDescription sds[9];
            for (int i = 0; i < (int)orders.size(); ++i)
                makeSortFields(orders[i].name.c_str(), &sds[i], orders[i].asc);
            std::sort(begin(), end(), recordsetSorter(&sds[0], &sds[orders.size()]));
        }
        return *this;
    }


    inline recordsetImple& reverse()
    {
        std::reverse(begin(), end());
        return *this;
    }

    inline void appendField(const _TCHAR* name, int type, short len)
    {
        assert(m_fds->size());
        
        fielddef fd((*m_fds)[0]);
        memset(&fd, 0, sizeof(fielddef));
        fd.len = len;
        fd.pos = 0;
        fd.type = type;
        fd.setName(name);
        if (fd.blobLenBytes())
            THROW_BZS_ERROR_WITH_MSG(_T("Can not append Blob or Text field."));
        m_fds->push_back(&fd);
        if (size())
            registerMemoryBlock(NULL, fd.len * size(), fd.len, mra_outerjoin);
    }

    inline recordsetImple& operator+=(const recordsetImple& r)
    {
        if (!m_fds->canUnion(*r.m_fds))
            THROW_BZS_ERROR_WITH_MSG(_T("Recordsets are different format"));

        m_recordset.reserve(m_recordset.size() + r.size());
        for (size_t i = 0; i < r.size(); ++i)
        {
            push_back(r.m_recordset[i]);
            r.m_recordset[i]->setFielddefs(m_fds.get());
        }
        for (size_t i = 0; i < r.m_memblock.size(); ++i)
            m_memblock.push_back(r.m_memblock[i]);
        return *this;
    }

#ifdef _DEBUG
    void dump()
    {
        dumpRecordset<recordsetImple> dumpRs;
        dumpRs(*this);
    }
#endif
};

inline multiRecordAlocatorImple::multiRecordAlocatorImple(recordsetImple* rs)
    : m_rs(rs), m_joinRowMap(NULL), m_rowOffset(0), m_addType(0),
      m_curFirstField(0)
{
}

inline void multiRecordAlocatorImple::init(size_t recordCount, size_t recordLen,
                                           int addType, const table* tb)
{
    m_rs->registerMemoryBlock(NULL, recordCount * recordLen, recordLen,
                              addType | m_addType, tb);
}

unsigned char* multiRecordAlocatorImple::allocBlobBlock(size_t size)
{
    return m_rs->allocBlobBlock(size);
}

inline unsigned char* multiRecordAlocatorImple::ptr(size_t row, int stat)
{
    int col = (stat == mra_current_block) ? m_curFirstField : 0;
    size_t rowNum = m_joinRowMap ? (*m_joinRowMap)[row + m_rowOffset][0]
                                 : row + m_rowOffset;
    return (*m_rs)[rowNum].ptr(col);
}

inline void multiRecordAlocatorImple::setInvalidRecord(size_t row, bool v)
{
    if (m_joinRowMap)
    {
        const std::vector<int>& map = (*m_joinRowMap)[row + m_rowOffset];
        for (int j = 0; j < (int)map.size(); ++j)
            (*m_rs)[map[j]].setInvalidRecord(v);
    }
    else
        (*m_rs)[row + m_rowOffset].setInvalidRecord(v);
}

inline void multiRecordAlocatorImple::duplicateRow(int row, int count)
{
    m_rs->duplicateRow(row, count);
}

inline void multiRecordAlocatorImple::removeLastMemBlock(int row)
{
    (*m_rs)[row].removeLastMemBlock();
}

/*template <> */inline recordsetImple::iterator begin(recordsetImple& m)
{
    return m.begin();
}
/*template <> */inline recordsetImple::iterator end(recordsetImple& m)
{
    return m.end();
}

/* Called from trdormapi.h : mdlsHandler::addContainer() */
inline void push_back(recordsetImple& m, row_ptr c)
{
}

/* for groupby */
inline void clear(recordsetImple& m)
{
    return m.clearRecords();
}

/* for groupby */
template <>
inline recordsetImple::key_type
resolvKeyValue(recordsetImple& m, const std::_tstring& name, bool noexception)
{
    return m.resolvKeyValue(name, noexception);
}

/* Called from trdormapi.h : mdlsHandler::operator() */
inline row* create(recordsetImple& m, int)
{
    return NULL;
}

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#endif // BZS_DB_PROTOCOL_TDAP_CLIENT_MEMRECORDSETIMPLE_H
