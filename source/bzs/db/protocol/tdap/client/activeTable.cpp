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
#include "activeTable.h"
#include "groupComp.h"

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

void activeTable::makeJoinMap(Container& mdls, joinmap_type& joinRowMap
			, std::vector<Container::key_type>& keyFields)
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

void activeTable::doJoin(bool innner, Container& mdls, queryBase& q, const _TCHAR* name1
				, const _TCHAR* name2, const _TCHAR* name3
				, const _TCHAR* name4, const _TCHAR* name5
				, const _TCHAR* name6, const _TCHAR* name7
				, const _TCHAR* name8, const _TCHAR* name9
				, const _TCHAR* name10, const _TCHAR* name11)
{
	if (mdls.size()==0) return;
	reverseAliasNamesQuery(q);
	q.clearSeekKeyValues();
	fields fds(m_tb);
	mraResetter mras(m_tb);
	Container::iterator it = mdls.begin(),ite = mdls.end();

	bool optimize = !(q.getOptimize() & queryBase::joinKeyValuesUnique);
	joinmap_type joinRowMap;
	std::vector<Container::key_type> fieldIndexes;

	fieldNames fns;
	fns.keyField(name1, name2, name3, name4, name5, name6, name7, name8, name9, name10, name11);
	const std::vector<std::_tstring>& names = fns.getKeyFields();

	for (int i=0;i<(int)names.size();++i)
		fieldIndexes.push_back(resolvKeyValue(mdls, names[i], false));

	/* optimizing join
		If base recordset is made by unique key and join by uniqe field, that can not opitimize.
	*/
	if (optimize)
	{
		makeJoinMap(mdls, joinRowMap, fieldIndexes);
		q.reserveSeekKeyValueSize(joinRowMap.size());
		std::vector<std::vector<int> >::iterator it1 = joinRowMap.begin(),ite1 = joinRowMap.end();
		while(it1 != ite1)
		{
			map_orm::mdl_typename& mdl = (mdls[(*it1)[0]]);
			for (int i=0;i<(int)fieldIndexes.size();++i)
				q.addSeekKeyValuePtr(mdl[fieldIndexes[i]].ptr());
			++it1;
		}
	}
	else
	{
		while(it != ite)
		{
			map_orm::mdl_typename& mdl = *(*it);
			for (int i=0;i<(int)fieldIndexes.size();++i)
				q.addSeekKeyValuePtr(mdl[fieldIndexes[i]].ptr());
			++it;
		}
	}

	m_tb->setQuery(&q);
	if (m_tb->stat() != 0)
		nstable::throwError(_T("activeTable Join Query"), &(*m_tb));

	map_orm::collection_orm_typename map(mdls);

	/* ignore list for inner join */
	std::vector<Container::iterator> ignores;
	it = mdls.begin();
	map.init(m_option, m_fdi, m_map, m_tb, &m_alias.map());
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
			else if (innner)
				ignores.push_back(it);
		}
		++it;
		m_tb->findNext(); //mra copy value to memrecord
	}

	readStatusCheck(*m_tb, _T("join"));
	m_tb->mra()->setJoinRowMap(NULL);

	/* remove record see ignore list for inner join */
	if (innner)
	{
		if (m_tb->isUseTransactd())
		{
			for (int i=(int)ignores.size()-1;i>=0;--i)
				mdls.erase(ignores[i]);
		}else
		{
			for (int i=(int)mdls.size()-1;i>=0;--i)
			{
				if(mdls[i].isInvalidRecord())
					mdls.erase(i);
			}
		}
	}
}


}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs





