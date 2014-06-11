//---------------------------------------------------------------------------
#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_ACTIVETABLE_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_ACTIVETABLE_H
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
#include "recordset.h"
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

class DLLLIB activeTable
{
	class activeTableImple* m_imple;

public:

	explicit activeTable(idatabaseManager& mgr, const _TCHAR* tableName);
	explicit activeTable(database_ptr& db, const _TCHAR* tableName);
	explicit activeTable(database* db, const _TCHAR* tableName);

	~activeTable();

	activeTable& alias(const _TCHAR* src, const _TCHAR* dst);

	activeTable& resetAlias();

	writableRecord& getWritableRecord();

	activeTable& join(recordset& mdls, queryBase& q, const _TCHAR* name1
					, const _TCHAR* name2=NULL, const _TCHAR* name3=NULL
					, const _TCHAR* name4=NULL, const _TCHAR* name5=NULL
					, const _TCHAR* name6=NULL, const _TCHAR* name7=NULL
					, const _TCHAR* name8=NULL, const _TCHAR* name9=NULL
					, const _TCHAR* name10=NULL, const _TCHAR* name11=NULL);

	activeTable& outerJoin(recordset& mdls, queryBase& q, const _TCHAR* name1
					, const _TCHAR* name2=NULL, const _TCHAR* name3=NULL
					, const _TCHAR* name4=NULL, const _TCHAR* name5=NULL
					, const _TCHAR* name6=NULL, const _TCHAR* name7=NULL
					, const _TCHAR* name8=NULL, const _TCHAR* name9=NULL
					, const _TCHAR* name10=NULL, const _TCHAR* name11=NULL);

	activeTable& index(int v);
	table_ptr table() const;
	activeTable& option(int v);
	activeTable& read(recordset& mdls, queryBase& q);
	activeTable& read(recordset& mdls, queryBase& q, validationFunc func);

	template <class T0>
	activeTable& keyValue(const T0 kv0)
	{
		keyValueSetter<T0>::set(table(), table()->keyNum(), kv0);
		return *this;
	}

	template <class T0, class T1>
	activeTable& keyValue(const T0 kv0, const T1 kv1)
	{
		keyValueSetter<T0, T1>::set(table(), table()->keyNum(), kv0, kv1);
		return *this;
	}

	template <class T0, class T1 , class T2>
	activeTable& keyValue(const T0 kv0, const T1 kv1, const T2 kv2)
	{
		keyValueSetter<T0, T1, T2>::set(table(), table()->keyNum(), kv0, kv1, kv2);
		return *this;
	}

	template <class T0, class T1 , class T2, class T3>
	activeTable& keyValue(const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3)
	{
		keyValueSetter<T0, T1, T2, T3>::set(table(), table()->keyNum(), kv0, kv1, kv2, kv3);
		return *this;
	}

	template <class T0, class T1 , class T2, class T3, class T4>
	activeTable& keyValue(const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3
							,const T4 kv4)
	{
		keyValueSetter<T0, T1, T2, T3, T4>
				::set(table(), table()->keyNum(), kv0, kv1, kv2, kv3, kv4);
		return *this;
	}

	template <class T0, class T1 , class T2, class T3, class T4, class T5 >
	activeTable& keyValue(const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3
							,const T4 kv4, const T5 kv5)
	{
		keyValueSetter<T0, T1, T2, T3, T4, T5>
				::set(table(), table()->keyNum(), kv0, kv1, kv2, kv3, kv4, kv5);
		return *this;
	}

	template <class T0, class T1 , class T2, class T3, class T4, class T5 , class T6>
	activeTable& keyValue(const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3
							,const T4 kv4, const T5 kv5, const T6 kv6)
	{
		keyValueSetter<T0, T1, T2, T3, T4, T5, T6>
				::set(table(), table()->keyNum(), kv0, kv1, kv2, kv3, kv4, kv5, kv6);
		return *this;
	}

	template <class T0, class T1 , class T2, class T3
				,class T4, class T5 , class T6 , class T7>
	activeTable& keyValue(const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3
							,const T4 kv4, const T5 kv5, const T6 kv6, const T7 kv7)
	{
		keyValueSetter<T0, T1, T2, T3, T4, T5, T6, T7>
				::set(table(), table()->keyNum(), kv0, kv1, kv2, kv3, kv4, kv5, kv6, kv7);
		return *this;
	}
};

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs

#endif //BZS_DB_PROTOCOL_TDAP_CLIENT_ACTIVETABLE_H

