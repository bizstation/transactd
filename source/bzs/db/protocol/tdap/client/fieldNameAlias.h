#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_FIELDNAMEALIAS_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_FIELDNAMEALIAS_H
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
#include "table.h"

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

class DLLLIB fdNmaeAlias
{
    struct fdnImple* m_imple;

public:
    fdNmaeAlias();
    fdNmaeAlias(const fdNmaeAlias& r);
    fdNmaeAlias& operator=(const fdNmaeAlias& r);
    ~fdNmaeAlias();
    void set(const _TCHAR* src, const _TCHAR* dst);
    const _TCHAR* get(const _TCHAR* src) const;
    const _TCHAR* resolv(const _TCHAR* dst) const;
    void clear();
    void reverseAliasNamesQuery(queryBase& q) const;
};

typedef fdNmaeAlias aliasMap_type;

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#endif // BZS_DB_PROTOCOL_TDAP_CLIENT_FIELDNAMEALIAS_H
