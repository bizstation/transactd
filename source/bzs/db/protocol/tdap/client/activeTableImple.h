//---------------------------------------------------------------------------
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
	typedef mdlsHandler< map_orm, recordsetImple> collection_orm_typename;


};



class  activeTableImple : public activeObject<map_orm>
{

	typedef recordsetImple Container;
	typedef boost::shared_ptr<writableRecord> record;
	typedef activeObject<map_orm> baseClass_type;
	typedef std::vector<std::vector<int> > joinmap_type;
	record m_record;

	
	template <class Container>
	void makeJoinMap(Container& mdls, joinmap_type& joinRowMap
				, std::vector<typename Container::key_type>& keyFields)
	{

		grouping_comp<Container> groupingComp(mdls, keyFields);
		std::vector<int> index;
		std::vector<int> tmp;
		for (int n=0;n<(int)mdls.size();++n)
		{
			bool found = false;
			int i = binary_search(n, index, 0, (int)index.size(), groupingComp, found);
			if (!found)
			{
				index.insert(index.begin() + i, n);
				joinRowMap.insert(joinRowMap.begin() + i, tmp);
			}
			joinRowMap[i].push_back(n);
		}
	}
	
	
	template <class Container>
	void doJoin(bool innner, Container& mdls, queryBase& q, const _TCHAR* name1
					, const _TCHAR* name2=NULL, const _TCHAR* name3=NULL
					, const _TCHAR* name4=NULL, const _TCHAR* name5=NULL
					, const _TCHAR* name6=NULL, const _TCHAR* name7=NULL
					, const _TCHAR* name8=NULL)
	{
		if (mdls.size()==0) return;
		m_alias.reverseAliasNamesQuery(q);
		q.clearSeekKeyValues();

		mraResetter mras(m_tb);
		typename Container::iterator it = mdls.begin(),ite = mdls.end();

		bool optimize = !(q.getOptimize() & queryBase::joinHasOneOrHasMany);
		joinmap_type joinRowMap;
		std::vector<typename Container::key_type> fieldIndexes;
		std::vector<ushort_td> fieldLens;

		fieldNames fns;
		fns.keyField(name1, name2, name3, name4, name5, name6, name7, name8);
		const fielddefs* fds = mdls.fieldDefs();
		for (int i=0;i<fns.count();++i)
		{
			std::_tstring s = fns.getValue(i);
			ushort_td index = resolvKeyValue(mdls, s, false);
			fieldIndexes.push_back(index);
			fieldLens.push_back((*fds)[index].len);
		}
		// optimizing join
		// if base recordsetImple is made by unique key and join by uniqe field, that can not opitimize.
		//
		q.joinKeySize(fns.count());
		if (optimize)
		{
			makeJoinMap(mdls, joinRowMap, fieldIndexes);
			q.reserveSeekKeyValueSize(joinRowMap.size());
			std::vector<std::vector<int> >::iterator it1 = joinRowMap.begin(),ite1 = joinRowMap.end();
			while(it1 != ite1)
			{
				row& mdl = *(mdls.getRow((*it1)[0]));
				for (int i=0;i<(int)fieldIndexes.size();++i)
					q.addSeekKeyValuePtr(mdl[fieldIndexes[i]].ptr(), fieldLens[i]);
				++it1;
			}
		}
		else
		{
			while(it != ite)
			{
				row& mdl = *(*it);
				for (int i=0;i<(int)fieldIndexes.size();++i)
					q.addSeekKeyValuePtr(mdl[fieldIndexes[i]].ptr(), fieldLens[i]);
				++it;
			}
		}

		m_tb->setQuery(&q);
		if (m_tb->stat() != 0)
			nstable::throwError(_T("activeObject Join Query"), &(*m_tb));

		typename map_orm::collection_orm_typename map(mdls);

		//std::vector<typename Container::iterator> ignores;
		it = mdls.begin();
		map.init(m_option, m_fdi, m_map, m_tb, &m_alias);
		if (m_tb->mra())
		{
			m_tb->mra()->setJoinType(innner ? mra_innerjoin : mra_outerjoin);
			if (optimize)
				m_tb->mra()->setJoinRowMap(&joinRowMap);
		}
		m_tb->find();
		while(1)
		{
			if (m_tb->stat())
			{
				if ((m_tb->stat() == STATUS_EOF) ||
					((m_tb->stat() != STATUS_SUCCESS) && (m_tb->stat() != STATUS_NOT_FOUND_TI)))
					break;
			}
			++it;
			m_tb->findNext(); //mra copy value to memrecord
		}

		readStatusCheck(*m_tb, _T("join"));
		m_tb->mra()->setJoinRowMap(NULL);

		// remove record see ignore list for inner join
		if (innner)
		{
			for (int i=(int)mdls.size()-1;i>=0;--i)
			{
				if(mdls[i].isInvalidRecord())
					mdls.erase(i);
			}
		}
	}
public:

	explicit activeTableImple(idatabaseManager* mgr, const _TCHAR* tableName)
		:baseClass_type(mgr, tableName){};

	explicit activeTableImple(database_ptr& db, const _TCHAR* tableName)
		:baseClass_type(db, tableName){};

	explicit activeTableImple(database* db, const _TCHAR* tableName)
		:baseClass_type(db, tableName){};


	inline writableRecord& getWritableRecord()
	{
		m_record.reset(writableRecord::create(m_tb.get(), &m_alias), &writableRecord::release);
		return *m_record.get();
	}

	inline void join(Container& mdls, queryBase& q, const _TCHAR* name1
					, const _TCHAR* name2=NULL, const _TCHAR* name3=NULL
					, const _TCHAR* name4=NULL, const _TCHAR* name5=NULL
					, const _TCHAR* name6=NULL, const _TCHAR* name7=NULL
					, const _TCHAR* name8=NULL)
	{
		doJoin(true, mdls, q, name1, name2, name3, name4, name5
						, name6, name7, name8);
	}

	inline void outerJoin(Container& mdls, queryBase& q, const _TCHAR* name1
					, const _TCHAR* name2=NULL, const _TCHAR* name3=NULL
					, const _TCHAR* name4=NULL, const _TCHAR* name5=NULL
					, const _TCHAR* name6=NULL, const _TCHAR* name7=NULL
					, const _TCHAR* name8=NULL)
	{
		doJoin(false, mdls, q, name1, name2, name3, name4, name5
						, name6, name7, name8);

	}
};

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs

#endif //BZS_DB_PROTOCOL_TDAP_CLIENT_ACTIVETABLEIMPLE_H

