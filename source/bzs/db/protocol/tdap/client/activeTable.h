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
#include "trdormapi.h"
#include "memRecordset.h"
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


class map_orm_fdi
{
	friend class map_orm;
	const table* m_tb;
public:
	void init(table* tb) {m_tb = tb;}
};

inline map_orm_fdi* createFdi(map_orm_fdi * ){return new map_orm_fdi();}
inline void destroyFdi(map_orm_fdi * p){delete p;}
inline void initFdi(map_orm_fdi * fdi, table* tb){fdi->init(tb);}

class map_orm
{
	const map_orm_fdi& m_fdi;
	short m_autoIncFiled;

	int comp(row& lm, row& rm, const _TCHAR* name, int index) const
	{
		return lm[index].comp(rm[index], 0);
	}


public:
	map_orm(const map_orm_fdi& fdi):m_fdi(fdi),m_autoIncFiled(-2){}

	bool compKeyValue(row& l, row& r, int keyNum) const
	{
		const tabledef* def = m_fdi.m_tb->tableDef();
		const keydef* kd = &def->keyDefs[keyNum];
		for (int i=0;i<kd->segmentCount;++i)
		{
			short n =  kd->segments[i].fieldNum;
			const fielddef* fd = &def->fieldDefs[n];
			int ret = comp(l, r, fd->name(), n);
			if (ret)return (ret < 0);

		}
		return 0;
	}

	template <class T>
	void readMap(T& m, const fields& fds, int optipn)
	{
		//needlessness
	}

	typedef row         mdl_typename;
	typedef map_orm_fdi fdi_typename;
	typedef mdlsHandler< map_orm, recordset> collection_orm_typename;


};

/** @endcond */


class AGRPACK activeTable : protected activeObject<map_orm>
{
	typedef activeObject<map_orm> baseClass_type;
	typedef std::vector<std::vector<int> > joinmap_type;
	typedef recordset Container;
	typedef boost::shared_ptr<writableRecord> record;

	record m_record;

	void makeJoinMap(Container& mdls, joinmap_type& joinRowMap
				, std::vector<Container::key_type>& keyFields);

	void doJoin(bool innner, Container& mdls, queryBase& q, const _TCHAR* name1
					, const _TCHAR* name2=NULL, const _TCHAR* name3=NULL
					, const _TCHAR* name4=NULL, const _TCHAR* name5=NULL
					, const _TCHAR* name6=NULL, const _TCHAR* name7=NULL
					, const _TCHAR* name8=NULL, const _TCHAR* name9=NULL
					, const _TCHAR* name10=NULL, const _TCHAR* name11=NULL);


public:

	explicit activeTable(idatabaseManager& mgr, const _TCHAR* tableName)
						:baseClass_type(mgr, tableName){}
	explicit activeTable(database_ptr& db, const _TCHAR* tableName)
						:baseClass_type(db, tableName){}
	explicit activeTable(database* db, const _TCHAR* tableName)
						:baseClass_type(db, tableName){}


	inline activeTable& alias(const _TCHAR* src, const _TCHAR* dst)
	{
		m_alias.set(src, dst);
		return *this;
	}

	inline activeTable& resetAlias()
	{
		m_alias.clear();
		return *this;
	}


	inline writableRecord& getWritableRecord()
	{
		m_record.reset(writableRecord::create(m_tb.get(), &m_alias.map()), &writableRecord::release);
		return *m_record.get();
	}

	inline activeTable& join(Container& mdls, queryBase& q, const _TCHAR* name1
					, const _TCHAR* name2=NULL, const _TCHAR* name3=NULL
					, const _TCHAR* name4=NULL, const _TCHAR* name5=NULL
					, const _TCHAR* name6=NULL, const _TCHAR* name7=NULL
					, const _TCHAR* name8=NULL, const _TCHAR* name9=NULL
					, const _TCHAR* name10=NULL, const _TCHAR* name11=NULL)
	{
		doJoin(true, mdls, q, name1, name2, name3, name4, name5
						, name6, name7, name8, name9, name10, name11);
		return *this;
	}

	inline activeTable& outerJoin(Container& mdls, queryBase& q, const _TCHAR* name1
					, const _TCHAR* name2=NULL, const _TCHAR* name3=NULL
					, const _TCHAR* name4=NULL, const _TCHAR* name5=NULL
					, const _TCHAR* name6=NULL, const _TCHAR* name7=NULL
					, const _TCHAR* name8=NULL, const _TCHAR* name9=NULL
					, const _TCHAR* name10=NULL, const _TCHAR* name11=NULL)
	{
		doJoin(false, mdls, q, name1, name2, name3, name4, name5
						, name6, name7, name8, name9, name10, name11);
		return *this;

	}

	inline activeTable& index(int v)
	{
		baseClass_type::index(v);
		return *this;
	}

	template <class T0>
	inline activeTable& keyValue(const T0 kv0)
	{
		baseClass_type::keyValue(kv0);
		return *this;
	}

	template <class T0, class T1>
	inline activeTable& keyValue(const T0 kv0, const T1 kv1)
	{
		baseClass_type::keyValue( kv0, kv1);
		return *this;
	}

	template <class T0, class T1 , class T2>
	inline activeTable& keyValue(const T0 kv0, const T1 kv1, const T2 kv2)
	{
		baseClass_type::keyValue( kv0, kv1, kv2);
		return *this;
	}

	template <class T0, class T1 , class T2, class T3>
	inline activeTable& keyValue(const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3)
	{
		baseClass_type::keyValue( kv0, kv1, kv2, kv3);
		return *this;
	}

	template <class T0, class T1 , class T2, class T3, class T4>
	inline activeTable& keyValue(const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3
							,const T4 kv4)
	{
		baseClass_type::keyValue( kv0, kv1, kv2, kv3, kv4);
		return *this;
	}

	template <class T0, class T1 , class T2, class T3, class T4, class T5 >
	inline activeTable& keyValue(const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3
							,const T4 kv4, const T5 kv5)
	{
		baseClass_type::keyValue( kv0, kv1, kv2, kv3, kv4, kv5);
		return *this;
	}

	template <class T0, class T1 , class T2, class T3, class T4, class T5 , class T6>
	inline activeTable& keyValue(const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3
							,const T4 kv4, const T5 kv5, const T6 kv6)
	{
		baseClass_type::keyValue( kv0, kv1, kv2, kv3, kv4, kv5, kv6);
		return *this;
	}

	template <class T0, class T1 , class T2, class T3
				,class T4, class T5 , class T6 , class T7>
	inline activeTable& keyValue(const T0 kv0, const T1 kv1, const T2 kv2, const T3 kv3
							,const T4 kv4, const T5 kv5, const T6 kv6, const T7 kv7)
	{
		baseClass_type::keyValue( kv0, kv1, kv2, kv3, kv4, kv5, kv6, kv7);
		return *this;
	}

	inline activeTable& option(int v)
	{
		baseClass_type::option(v);
		return *this;
	}

	activeTable& read(Container& mdls, queryBase& q)
	{
		map_orm::collection_orm_typename map(mdls);
		baseClass_type::readMap(map, q);
		return *this;
	}

	activeTable& read(Container& mdls, queryBase& q, validationFunc func)
	{
		map_orm::collection_orm_typename map(mdls);
		baseClass_type::readMap(map, q, func);
		return *this;
	}

	using baseClass_type::table;

};

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs

#endif //BZS_DB_PROTOCOL_TDAP_CLIENT_ACTIVETABLE_H

