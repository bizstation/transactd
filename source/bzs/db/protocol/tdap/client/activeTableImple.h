#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_ACTIVETABLEIMPLE_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_ACTIVETABLEIMPLE_H
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
#include "trdboostapi.h"
#include "trdormapi.h"
#include "recordsetImple.h"
#include "memRecord.h"

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

class map_orm_fdi
{
    friend class map_orm;
    const table* m_tb;

public:
    void init(table* tb) { m_tb = tb; }
};

inline map_orm_fdi* createFdi(map_orm_fdi*)
{
    return new map_orm_fdi();
}
inline void destroyFdi(map_orm_fdi* p)
{
    delete p;
}
inline void initFdi(map_orm_fdi* fdi, table* tb)
{
    fdi->init(tb);
}

class map_orm
{
    const map_orm_fdi& m_fdi;
    short m_autoIncFiled;

    int comp(row& lm, row& rm, const _TCHAR* name, int index) const
    {
        return lm[index].comp(rm[index], 0);
    }

public:
    map_orm(const map_orm_fdi& fdi) : m_fdi(fdi), m_autoIncFiled(-2) {}

    bool compKeyValue(row& l, row& r, int keyNum) const
    {
        const tabledef* def = m_fdi.m_tb->tableDef();
        const keydef* kd = &def->keyDefs[keyNum];
        for (int i = 0; i < kd->segmentCount; ++i)
        {
            short n = kd->segments[i].fieldNum;
            const fielddef* fd = &def->fieldDefs[n];
            int ret = comp(l, r, fd->name(), n);
            if (ret)
                return (ret < 0);
        }
        return 0;
    }

    template <class T> void readMap(T& m, const fields& fds, int optipn)
    {
        // needlessness
    }

    typedef row mdl_typename;
    typedef map_orm_fdi fdi_typename;
    typedef mdlsHandler<map_orm, recordsetImple> collection_orm_typename;
};

#define FIXED_VALUE_BUF_SIZE 256
struct joinInfo
{
    unsigned char fixedValue[FIXED_VALUE_BUF_SIZE];
    ushort_td len;
    ushort_td type;
};

#define JOIN_KEYVALUE_TYPE_PTR 0
#define JOIN_KEYVALUE_TYPE_STR 1
#define MAX_JOIN_KEY_SIZE 8
class activeTableImple : public activeObject<map_orm>
{

    typedef recordsetImple Container;
    typedef writableRecord* record;
    typedef activeObject<map_orm> baseClass_type;
    typedef std::vector<std::vector<int> > joinmap_type;
    record m_record;
    int m_tmpIndex;

    // return can memcpy
    uchar_td convertFieldType(uchar_td v)
    {
        if (v == ft_autoinc)
            v = ft_integer;
        else if (v == ft_autoIncUnsigned)
            v = ft_integer;
        else if (v == ft_uinteger)
            v = ft_integer;
        else if (v == ft_logical)
            v = ft_integer;
        else if (v == ft_bit)
            v = ft_integer;
        return v;
    }

    template <class Container>
    inline bool makeJoinFieldInfo(Container& mdls, const fielddefs* fds,
                      const _TCHAR*  fns[], int fnsCount,
                      std::vector<typename Container::key_type>& fieldIndexes,
                      std::vector<joinInfo>& joinFields)
    {
        joinFields.resize(fnsCount);
        fieldIndexes.resize(fnsCount);
        const tabledef* td = table()->tableDef();
        const keydef* kd = &td->keyDefs[(int)table()->keyNum()];
        if (kd->segmentCount < fnsCount)
            THROW_BZS_ERROR_WITH_MSG(_T("Join key fields are too many.\n ")
                                     _T("Check index number and field count."));

        bool hasMany = ((kd->segmentCount > fnsCount) || kd->segments[0].flags.bit0);/* duplicate key*/

        for (int i = 0; i < fnsCount; ++i)
        {
            std::_tstring s = fns[i];
            if (s[0] == '[')
            {
                fieldIndexes[i] = -1;
                const fielddef& f = td->fieldDefs[kd->segments[i].fieldNum];
                if (f.len > FIXED_VALUE_BUF_SIZE)
                    THROW_BZS_ERROR_WITH_MSG(_T("Join fixed key field is too long")
                                     _T(" field.\n This field can not use join."));
                field fd(joinFields[i].fixedValue - f.pos, f, fds);
                size_t pos = s.find(_T(']'), 1);
                if (pos == std::_tstring::npos)
                    THROW_BZS_ERROR_WITH_MSG(_T("Join fixed key field is invalid format")
                                     _T(".\n This field can not use join."));
                fd = s.substr(1, pos - 1).c_str();
                joinFields[i].len = f.isStringType() ? 0xff : f.len;
                joinFields[i].type = JOIN_KEYVALUE_TYPE_PTR;

            }
            else
            {
                ushort_td index = resolvKeyValue(mdls, s, false);
                fieldIndexes[i] = index;
                const fielddef& fd = (*fds)[index];
                // Check the fieldType
                if (convertFieldType(fd.type) ==
                    convertFieldType(
                        td->fieldDefs[kd->segments[i].fieldNum].type))
                {
                    joinFields[i].len = fd.isStringType() ? 0xff : fd.len;
                    joinFields[i].type = JOIN_KEYVALUE_TYPE_PTR;
                }
                else
                {
                    joinFields[i].len = 0;
                    joinFields[i].type = JOIN_KEYVALUE_TYPE_STR;
                }
            }
        }
        return hasMany;
    }

    template <class Container>
    void makeJoinMap(Container& mdls, joinmap_type& joinRowMap,
                     std::vector<typename Container::key_type>& keyFields)
    {
        grouping_comp<Container> groupingComp(mdls, keyFields);
        std::vector<int> index;
        std::vector<int> tmp;
        for (int n = 0; n < (int)mdls.size(); ++n)
        {
            bool found = false;
            int i = binary_search(n, index, 0, (int)index.size(), groupingComp,
                                  found);
            if (!found)
            {
                index.insert(index.begin() + i, n);
                joinRowMap.insert(joinRowMap.begin() + i, tmp);
            }
            joinRowMap[i].push_back(n);
        }
    }


    template <class Container>
    inline void addSeekValues(row& mdl, pq_handle& q,
                  std::vector<typename Container::key_type>& fieldIndexes,
                  std::vector<joinInfo>& joinFields,
                  const fielddefs* fds)
    {
        const uchar_td* ptr[8];
        int len[8];
        uchar_td buf[MAX_KEYLEN];
        uchar_td* buf_ptr = buf;
        const tabledef* td = table()->tableDef();
        const keydef* kd = &td->keyDefs[(int)table()->keyNum()];

        for (int i = 0; i < (int)fieldIndexes.size(); ++i)
        {
            if (fieldIndexes[i] == -1)
            {
                ptr[i] = joinFields[i].fixedValue;
                len[i] = joinFields[i].len;
            }
            else if (joinFields[i].type == JOIN_KEYVALUE_TYPE_PTR)
            {
                ptr[i] = (const uchar_td*)mdl[fieldIndexes[i]].ptr();
                len[i] = joinFields[i].len;
            }
            else
            {
                // if target field type is different then we need convrt type
                const fielddef& f = td->fieldDefs[kd->segments[i].fieldNum];
                field fd(buf_ptr - f.pos, f, fds);
                fd = mdl[fieldIndexes[i]].c_str();
                ptr[i] = buf_ptr;
                len[i] = f.isStringType() ? 0xff : f.len;
                buf_ptr += f.len;
            }
        }
    
        if (!q->supplySeekValue(ptr, len, (int)fieldIndexes.size(), m_tmpIndex))
            THROW_BZS_ERROR_WITH_MSG(_T("Join key value(s) are invalid at supply values to prepared statement or query.\n ")
                                     _T("Check prepared statement or query."));
    }

    int makeJoinKeys(const _TCHAR* fns[MAX_JOIN_KEY_SIZE], int &fnsCount, const _TCHAR* name1,
                const _TCHAR* name2 = NULL, const _TCHAR* name3 = NULL,
                const _TCHAR* name4 = NULL, const _TCHAR* name5 = NULL,
                const _TCHAR* name6 = NULL, const _TCHAR* name7 = NULL,
                const _TCHAR* name8 = NULL)
    {
        int count = 0;
        fns[0] = name1; fns[1] = name2; fns[2] = name3; fns[3] = name4;
        fns[4] = name5; fns[5] = name6; fns[6] = name7; fns[7] = name8;
        
        for (count = 0; count < MAX_JOIN_KEY_SIZE; ++count)
            if ((fns[count] == NULL) || (fns[count][0] == 0x00))
                break;
        return count;
    }

    inline void reserveSeekSize(pq_handle& q,  size_t size, int keySize)
    {
        q->beginSupplySeekValues(size, keySize);
        m_tmpIndex = 0;
    }

    template <class Container>
    void doJoin(bool innner, Container& mdls, pq_handle& stmt, const _TCHAR* fns[8], int fnsCount)
    {
        if (mdls.size() == 0) return;
        stmt->clearSeeks();
        mraResetter mras(m_tb);
        typename Container::iterator it = mdls.begin(), ite = mdls.end();

        joinmap_type joinRowMap;

        std::vector<typename Container::key_type> fieldIndexes;
        std::vector<joinInfo> joinFields;

        const fielddefs* fds = mdls.fieldDefs();
        bool hasMany = makeJoinFieldInfo<Container>(mdls, fds, fns, fnsCount, fieldIndexes, joinFields);
        if (!hasMany)
            hasMany = (stmt->cachedOptimaize() & queryBase::joinHasOneOrHasMany);

        // optimizing join
        // if base recordsetImple is made by unique key and join by uniqe field,
        // that can not opitimize.
        //
        if (!hasMany)
        {
            makeJoinMap(mdls, joinRowMap, fieldIndexes);
            reserveSeekSize(stmt, joinRowMap.size() * fieldIndexes.size(), fnsCount);
            std::vector<std::vector<int> >::iterator it1 = joinRowMap.begin(),
                                                     ite1 = joinRowMap.end();
            while (it1 != ite1)
            {
                row& mdl = *(mdls.getRow((*it1)[0]));
                addSeekValues<Container>(mdl, stmt, fieldIndexes, joinFields, fds);
                ++it1;
            }
        }
        else
        {
            reserveSeekSize(stmt, mdls.size() * fieldIndexes.size(), fnsCount);
            while (it != ite)
            {
                row& mdl = *(*it);
                addSeekValues<Container>(mdl, stmt, fieldIndexes, joinFields, fds);
                ++it;
            }
        }

        if (m_tb->stat() != 0)
            nstable::throwError(_T("activeObject Join Query"), &(*m_tb));

        typename map_orm::collection_orm_typename map(mdls);

        // std::vector<typename Container::iterator> ignores;
        it = mdls.begin();
        map.init(m_option, m_fdi, m_map, m_tb, &m_alias);
        if (m_tb->mra())
        {
            m_tb->mra()->setJoinType(innner ? mra_innerjoin : mra_outerjoin);
            if (!hasMany)
                m_tb->mra()->setJoinRowMap(&joinRowMap);
        }
        m_tb->find();
        while (1)
        {
            if (m_tb->stat())
            {
                if ((m_tb->stat() == STATUS_EOF) ||
                    ((m_tb->stat() != STATUS_SUCCESS) &&
                     (m_tb->stat() != STATUS_NOT_FOUND_TI)))
                    break;
            }
            ++it;
            m_tb->findNext(); // mra copy value to memrecord
        }

        readStatusCheck(*m_tb, _T("join"));
        m_tb->mra()->setJoinRowMap(NULL);

        // remove record see ignore list for inner join
        if (innner)
        {
            for (int i = (int)mdls.size() - 1; i >= 0; --i)
            {
                if (mdls[i].isInvalidRecord())
                    mdls.erase(i);
            }
        }
    }

    inline void checkPreparedQuery(pq_handle& q)
    {
        if (!q)
            THROW_BZS_ERROR_WITH_MSG(_T("Invalid query or prepqredQuery.\n "));
    }

public:
    explicit activeTableImple(idatabaseManager* mgr, const _TCHAR* tableName)
        : baseClass_type(mgr, tableName), m_record(NULL){};

    explicit activeTableImple(database_ptr& db, const _TCHAR* tableName, short mode)
        : baseClass_type(db, tableName, mode), m_record(NULL){};

    explicit activeTableImple(database* db, const _TCHAR* tableName, short mode)
        : baseClass_type(db, tableName, mode), m_record(NULL){};

    explicit activeTableImple(database_ptr& db, short tableIndex, short mode)
        : baseClass_type(db, tableIndex, mode), m_record(NULL){};

    explicit activeTableImple(database* db, short tableIndex, short mode)
        : baseClass_type(db, tableIndex, mode), m_record(NULL){};

    ~activeTableImple()
    {
        if (m_record)
            m_record->release();
    }

    inline writableRecord& getWritableRecord()
    {
        if (m_record == NULL)
        {
            m_record = writableRecord::create(m_tb.get(), &m_alias);
            m_record->addref();
        }
        return *m_record;
    }

    inline void join(Container& mdls, queryBase& q, const _TCHAR* name1,
                     const _TCHAR* name2 = NULL, const _TCHAR* name3 = NULL,
                     const _TCHAR* name4 = NULL, const _TCHAR* name5 = NULL,
                     const _TCHAR* name6 = NULL, const _TCHAR* name7 = NULL,
                     const _TCHAR* name8 = NULL)
    {
        const _TCHAR* fns[MAX_JOIN_KEY_SIZE];
        m_alias.reverseAliasNamesQuery(q);
        int fnsCount = makeJoinKeys(fns, fnsCount, name1, name2, name3, name4,
                                        name5, name6, name7, name8);
        q.joinKeySize(fnsCount);
        pq_handle pq = setQuery(m_tb, q);
        checkPreparedQuery(pq);
        doJoin(true, mdls, pq, fns, fnsCount);
    }

    inline void
    outerJoin(Container& mdls, queryBase& q, const _TCHAR* name1,
              const _TCHAR* name2 = NULL, const _TCHAR* name3 = NULL,
              const _TCHAR* name4 = NULL, const _TCHAR* name5 = NULL,
              const _TCHAR* name6 = NULL, const _TCHAR* name7 = NULL,
              const _TCHAR* name8 = NULL)
    {
        const _TCHAR* fns[MAX_JOIN_KEY_SIZE];
        m_alias.reverseAliasNamesQuery(q);
        int fnsCount = makeJoinKeys(fns, fnsCount, name1, name2, name3, name4,
                                        name5, name6, name7, name8);
        q.joinKeySize(fnsCount);
        pq_handle pq = setQuery(m_tb, q);
        checkPreparedQuery(pq);
        doJoin(false, mdls, pq, fns, fnsCount);

    }

    inline void join(Container& mdls, pq_handle& q, const _TCHAR* name1,
                     const _TCHAR* name2 = NULL, const _TCHAR* name3 = NULL,
                     const _TCHAR* name4 = NULL, const _TCHAR* name5 = NULL,
                     const _TCHAR* name6 = NULL, const _TCHAR* name7 = NULL,
                     const _TCHAR* name8 = NULL)
    {
        const _TCHAR* fns[MAX_JOIN_KEY_SIZE];
        int fnsCount = makeJoinKeys(fns, fnsCount, name1, name2, name3, name4,
                                        name5, name6, name7, name8);
        pq_handle pq = setQuery(m_tb, q);
        checkPreparedQuery(pq);
        doJoin(true, mdls, pq, fns, fnsCount);    
    }

    inline void
    outerJoin(Container& mdls, pq_handle& q, const _TCHAR* name1,
              const _TCHAR* name2 = NULL, const _TCHAR* name3 = NULL,
              const _TCHAR* name4 = NULL, const _TCHAR* name5 = NULL,
              const _TCHAR* name6 = NULL, const _TCHAR* name7 = NULL,
              const _TCHAR* name8 = NULL)
    {
        const _TCHAR* fns[MAX_JOIN_KEY_SIZE];
        int fnsCount = makeJoinKeys(fns, fnsCount, name1, name2, name3, name4,
                                        name5, name6, name7, name8);
        pq_handle pq = setQuery(m_tb, q);
        checkPreparedQuery(pq);
        doJoin(false, mdls, pq, fns, fnsCount);
    }

    void releaseTable() { m_tb.reset(); }
};

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#endif // BZS_DB_PROTOCOL_TDAP_CLIENT_ACTIVETABLEIMPLE_H
