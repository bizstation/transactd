#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_RECORDSET_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_RECORDSET_H
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

class DLLLIB recordset
{

    friend class activeTable;
    class recordsetImple* m_imple;

public:
    typedef std::vector<row_ptr>::iterator iterator;

    recordset();
    recordset(const recordset& r);
    recordset& operator=(const recordset& r);
    ~recordset();
    recordset* clone() const;
    row& operator[](size_t index) const;
    row& first() const;
    row& last() const;
    size_t size() const;
    size_t count() const;
    void clearRecords();
    const fielddefs* fieldDefs() const;
    void clear();
    recordset& top(recordset& c, int n) const;
    iterator begin();
    iterator end();
    iterator erase(size_t index);
    iterator erase(const iterator& it);
    void removeField(int index);
    recordset& matchBy(recordsetQuery& rq);
    recordset& groupBy(groupQuery& gq);
    recordset& orderBy(const _TCHAR* name1, const _TCHAR* name2 = NULL,
                       const _TCHAR* name3 = NULL, const _TCHAR* name4 = NULL,
                       const _TCHAR* name5 = NULL, const _TCHAR* name6 = NULL,
                       const _TCHAR* name7 = NULL, const _TCHAR* name8 = NULL);
    recordset& orderBy(const sortFields& orders);
    recordset& reverse();
    void appendField(const _TCHAR* name, int type, short len);
    recordset& operator+=(const recordset& r);
    void release();
    static recordset* create();

#ifdef _DEBUG
    void dump();
#endif
};

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#endif // BZS_DB_PROTOCOL_TDAP_CLIENT_RECORDSET_H
