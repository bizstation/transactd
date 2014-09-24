#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_MEMRECORDSET_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_MEMRECORDSET_H
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
#include "groupQuery.h"

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

class AGRPACK recordset
{
    friend class multiRecordAlocatorImple;
    boost::shared_ptr<fielddefs> m_fds;
    boost::shared_ptr<class multiRecordAlocatorImple> m_mra;
    std::vector<row_ptr> m_recordset;
    std::vector<boost::shared_ptr<autoMemory> > m_memblock;
    std::vector<boost::shared_ptr<fielddefs> > m_unionFds;

    /* for registerMemoryBlock temp data */
    size_t m_joinRows;

    /*
            for optimazing join.
            If the first reading is using by unique key , set that field count.
    */
    short m_uniqueReadMaxField;

public:
    typedef std::vector<row_ptr>::iterator iterator;

private:
    void registerMemoryBlock(unsigned char* ptr, size_t size, size_t recordLen,
                             int addtype, const table* tb = NULL);
    void makeSortFields(const _TCHAR* name, std::vector<int>& fieldNums);
    int getMemBlockIndex(unsigned char* ptr) const;

public:
    typedef fielddefs header_type;
    typedef int key_type;
    typedef row_ptr row_type;
    typedef row row_pure_type;

    recordset();
    ~recordset();
    /* This clone is deep copy.
       But text and blob field data memory are shared.
    */
    recordset* clone() const;

    inline short uniqueReadMaxField() const { return m_uniqueReadMaxField; }

    inline row_ptr& getRow(size_t index) { return m_recordset[index]; }

    inline row& operator[](size_t index) { return *m_recordset[index].get(); }

    inline row& first() { return *m_recordset[0].get(); }

    inline row& last() { return *m_recordset[m_recordset.size() - 1].get(); }

    inline size_t size() const { return m_recordset.size(); }

    inline size_t count() const { return m_recordset.size(); }

    void clearRecords();
    const fielddefs* fieldDefs() const { return m_fds.get(); }
    void clear();
    recordset& top(recordset& c, int n);
    iterator begin();
    iterator end();
    iterator erase(size_t index);
    iterator erase(const iterator& it);
    void push_back(row_ptr r);
    void readBefore(const table_ptr tb, const aliasMap_type* alias);

    key_type resolvKeyValue(const std::_tstring& name,
                            bool noexception = false);
    void removeField(int index);
    recordset& matchBy(recordsetQuery& rq);
    recordset& groupBy(groupQuery& gq);
    recordset& orderBy(const _TCHAR* name1, const _TCHAR* name2 = NULL,
                       const _TCHAR* name3 = NULL, const _TCHAR* name4 = NULL,
                       const _TCHAR* name5 = NULL, const _TCHAR* name6 = NULL,
                       const _TCHAR* name7 = NULL, const _TCHAR* name8 = NULL);
    recordset& reverse();
    void appendCol(const _TCHAR* name, int type, short len);
    recordset& operator+=(const recordset& r);

#ifdef _DEBUG
    void dump();

#endif
};

/** @cond INTERNAL */

inline recordset::iterator begin(recordset& m)
{
    return m.begin();
}
inline recordset::iterator end(recordset& m)
{
    return m.end();
}
inline void push_back(recordset& m, row_ptr c)
{
}

/* for groupby */
inline void clear(recordset& m)
{
    return m.clearRecords();
}

/* for groupby */
inline recordset::key_type resolvKeyValue(recordset& m,
                                          const std::_tstring& name,
                                          bool noexception = false)
{
    return m.resolvKeyValue(name, noexception);
}

inline row* create(recordset& m, int)
{
    return NULL;
}

/** @endcond */

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#endif // BZS_DB_PROTOCOL_TDAP_CLIENT_MEMRECORDSET_H
