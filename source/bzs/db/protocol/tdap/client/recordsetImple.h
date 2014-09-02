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

struct sortDescription
{
	short index;
	bool asc;
};

class recordsetSorter
{
	const std::vector<sortDescription>& m_sortDesc;

public:
	recordsetSorter(std::vector<sortDescription>& sortDesc):m_sortDesc(sortDesc)
	{

	}
	bool operator()(const row_ptr& l, const row_ptr r) const
	{
		std::vector<sortDescription>::const_iterator it = m_sortDesc.begin();
		while (it != m_sortDesc.end())
		{
			int ret = (*l)[(*it).index].comp((*r)[(*it).index], 0);
			if (ret) return ((*it).asc) ? (ret < 0):(ret > 0);

			++it;
		}
		return false;
	}
};

class multiRecordAlocatorImple : public multiRecordAlocator
{
	class recordsetImple* m_rs;
	const std::vector< std::vector<int> >* m_joinRowMap;
	int m_rowOffset;
	int m_addType;
	int m_curFirstFiled;

public:
	inline multiRecordAlocatorImple(recordsetImple* rs);
	inline void init(size_t recordCount, size_t recordLen, int addType, const table* tb);
	inline unsigned char* ptr(size_t row, int stat);
	inline void setRowOffset(int v){m_rowOffset = v;}
	inline void setJoinType(int v){m_addType = v;}
	inline void setInvalidRecord(size_t row, bool v);
	inline void setCurFirstFiled(int v){m_curFirstFiled = v;}
	inline void setJoinRowMap(const std::vector< std::vector<int> >* v/*, size_t size*/){m_joinRowMap = v;/*m_joinMapSize = size;*/}
	inline const std::vector< std::vector<int> >* joinRowMap()const {return m_joinRowMap;}
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
	std::vector<boost::shared_ptr<fielddefs> > m_unionFds;

	/* for registerMemoryBlock temp data */
	size_t m_joinRows;

	/*
		for optimazing join.
		If the first reading is using by unique key , set that field count.
	*/
	short m_uniqueReadMaxField;
public:
	typedef std::vector<row_ptr >::iterator iterator;

private:

	void registerMemoryBlock(unsigned char* ptr, size_t size, size_t recordLen
					, int addtype, const table* tb=NULL)
	{
		autoMemory* am = new autoMemory(ptr, size, 0 , true);
		m_memblock.push_back(boost::shared_ptr<autoMemory>(am));
		unsigned char* p = am->ptr;
		//copy fileds
		if (addtype & mra_nextrows)
		{
			if (addtype == mra_nextrows)
				m_mra->setRowOffset((int)m_recordset.size()); //no join
			else
				m_mra->setRowOffset((int)m_joinRows); //Join
		}
		else 
		{
			//assert(tb);
			m_joinRows = 0;
			m_mra->setRowOffset(0);
			m_mra->setCurFirstFiled((int)m_fds->size());
			if (tb)
				m_fds->copyFrom(tb);
			if (tb && (addtype == mra_nojoin))
			{
				const keydef& kd = tb->tableDef()->keyDefs[tb->keyNum()];
				m_uniqueReadMaxField =  (kd.segments[0].flags.bit0 == false) ? (short)m_fds->size():0;
			}
		}
		
		*(am->endFieldIndex) = (short)m_fds->size();
		size_t rows = size/recordLen;

		// set record pointer to each record
		if ((addtype & mra_innerjoin)
						|| (addtype & mra_outerjoin))
		{
			//Join optimazing
			const std::vector< std::vector<int> >* jmap = m_mra->joinRowMap();
			
			if (jmap)
			{
				// At Join that if some base records reference to a joined record
				//		that the joined record pointer is shared by some base records.
				for (int i=0;i<(int)rows;++i)
				{
					const std::vector<int>& map = (*jmap)[i+m_joinRows];
					for (int j=0;j<(int)map.size();++j)
						m_recordset[map[j]]->setRecordData(p + recordLen*i , 0, am->endFieldIndex, false);
				}
			}
			else
			{
				for (int i=0;i<(int)rows;++i)
					m_recordset[i+m_joinRows]->setRecordData(p + recordLen*i , 0, am->endFieldIndex, false);
			}
			m_joinRows += rows;
			
		}
		else
		{	//create new record
			size_t reserveSize =  m_recordset.size() + rows;
			m_recordset.reserve(reserveSize);
			for (int i=0;i<(int)rows;++i)
			{
				row_ptr rec(memoryRecord::create(*m_fds), &memoryRecord::release);
				rec->setRecordData(p + recordLen*i, 0, am->endFieldIndex, false);
				m_recordset.push_back(rec);
			}
		}
	}

	void makeSortFields(const _TCHAR* name, std::vector<sortDescription>& sortDesc
				, bool asc = true)
	{
		sortDescription sd;
		sd.index =  m_fds->indexByName(name);
		if (sd.index ==-1)
			THROW_BZS_ERROR_WITH_MSG(_T("orderBy:Invalid field name"));
				sd.asc = asc;
		sortDesc.push_back(sd);
	}

	int getMemBlockIndex(unsigned char* ptr) const
	{
		for (int i=0;i<(int)m_memblock.size();++i)
		{
			const boost::shared_ptr<autoMemory>& am = m_memblock[i];
			if ((ptr >= am->ptr) && (ptr < am->ptr + am->size))
				return i;
		}
		assert(0);
		return -1;
	}

	//Duplicate row for hasManyJoin
	void duplicateRow(int row, int count)
	{
		row_ptr& r = m_recordset[row];
		memoryRecord* p = dynamic_cast<memoryRecord*>(r.get());

		m_recordset.reserve(m_recordset.size()+count);
		m_recordset.insert(m_recordset.begin() + row, count, row_ptr());
		for (int i=0;i<count;++i)
			m_recordset[i+row].reset(new memoryRecord(*p), &memoryRecord::release);
	}

public:
	inline recordsetImple():m_fds(fielddefs::create(), &fielddefs::destroy)
		, m_joinRows(0), m_uniqueReadMaxField(0)
	{
		m_mra.reset(new multiRecordAlocatorImple(this));
	}

	inline ~recordsetImple()
	{

	}

	inline void checkIndex(size_t index)
	{
		if (index >= m_recordset.size())
			THROW_BZS_ERROR_WITH_MSG(_T("Invalid row index of recordset."));
	}

	/* This clone is deep copy.
	   But text and blob field data memory are shared.
	*/
	inline recordsetImple* clone() const
	{
		recordsetImple* p = new recordsetImple();
		p->m_joinRows = m_joinRows;
		p->m_uniqueReadMaxField = m_uniqueReadMaxField;
		p->m_unionFds = m_unionFds;
		p->m_fds.reset(m_fds->clone(), &fielddefs::destroy);

		std::vector<__int64> offsets;
		for (int i=0;i<(int)m_memblock.size();++i)
		{
			autoMemory* am = new autoMemory(m_memblock[i]->ptr, m_memblock[i]->size, 0 , true);
			*am->endFieldIndex =  *m_memblock[i]->endFieldIndex;
			p->m_memblock.push_back(boost::shared_ptr<autoMemory>(am));
			offsets.push_back((__int64)(am->ptr - m_memblock[i]->ptr));
		}

		for (int i=0;i<(int)m_recordset.size();++i)
		{
			memoryRecord* row = dynamic_cast<memoryRecord*>(m_recordset[i].get());
			memoryRecord* mr = memoryRecord::create(*p->m_fds);
			row_ptr rec(mr, &memoryRecord::release);
			p->m_recordset.push_back(rec);

			for (int j=0;j<(int)row->memBlockSize();++j)
			{
				const autoMemory& mb =  row->memBlock(j);
				int index = getMemBlockIndex(mb.ptr);
				#pragma warn -8072
				unsigned char* ptr =  mb.ptr + offsets[index];
				#pragma warn .8072
				const boost::shared_ptr<autoMemory>& am =  p->m_memblock[index];
				mr->setRecordData(ptr, mb.size, am->endFieldIndex, mb.owner);
			}
		}
		return p;
	}

	inline short uniqueReadMaxField() const{return m_uniqueReadMaxField;}

	inline void clearRecords()
	{
		m_recordset.clear();
		m_uniqueReadMaxField = 0;
	}

	inline const fielddefs* fieldDefs() const {return m_fds.get();}

	inline void clear()
	{
		clearRecords();
		m_fds->clear();
		m_unionFds.clear();
		m_memblock.clear();
	}

	inline row_ptr& getRow(size_t index)
	{
		return m_recordset[index];
	}

	inline row& operator[](size_t index)const
	{
		return *m_recordset[index].get();
	}

	inline row& first() const
	{
		if (m_recordset.size() == 0)
			THROW_BZS_ERROR_WITH_MSG(_T("Invalid index of recordset."));
		return *m_recordset[0].get();
	}

	inline row& last() const
	{
		if (m_recordset.size() == 0)
			THROW_BZS_ERROR_WITH_MSG(_T("Invalid index of recordset."));
		return *m_recordset[m_recordset.size() - 1].get();
	}

	inline recordsetImple& top(recordsetImple& c, int n) const
	{
		c = *this;
		c.clearRecords();
		n = std::min(n, (int)m_recordset.size());
		for (int i=0;i<n;++i)
			c.push_back(m_recordset[i]);
		return c;
	}

	inline iterator begin(){return m_recordset.begin();}

	inline iterator end(){return m_recordset.end();}

	inline iterator erase(size_t index)
	{
		return m_recordset.erase(m_recordset.begin()+index);
	}

	inline iterator erase(const iterator& it){return m_recordset.erase(it);}

	inline void push_back(row_ptr r){m_recordset.push_back(r);};

	inline size_t size()const {return m_recordset.size();}

	inline size_t count()const {return m_recordset.size();}

	void readBefore(const table_ptr tb, const aliasMap_type* alias)
	{
		 tb->setMra(m_mra.get());
		 m_fds->setAliases(alias);
	}

	typedef fielddefs header_type;
	typedef int key_type;
	typedef row_ptr row_type;
	typedef row row_pure_type;

	key_type resolvKeyValue(const std::_tstring& name, bool noexception=false)
	{
		int index = m_fds->indexByName(name);
		if (index != -1)
			return index;
		if (!noexception)
			THROW_BZS_ERROR_WITH_MSG(_T("groupQuery:Invalid key name"));

		return (key_type)m_fds->size();
	}

	inline void removeField(int index)
	{
		m_fds->remove(index);
		for(int i=0;i<(int)m_unionFds.size();++i)
			m_unionFds[i]->remove(index);

		for(int i=0;i<(int)m_memblock.size();++i)
		{
			if (*(m_memblock[i]->endFieldIndex) > index)
			{
				short v = *(m_memblock[i]->endFieldIndex) -1;
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
			for (int i=(int)m_recordset.size()-1;i>=0;--i)
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

	inline recordsetImple& orderBy(const _TCHAR* name1 , const _TCHAR* name2=NULL,
					 const _TCHAR* name3=NULL,const _TCHAR* name4=NULL,
					 const _TCHAR* name5=NULL, const _TCHAR* name6=NULL,
					 const _TCHAR* name7=NULL, const _TCHAR* name8=NULL)
	{
		if (m_recordset.size())
		{
			std::vector<sortDescription> sds;
			makeSortFields(name1, sds, true);
			if (name2) makeSortFields(name2,  sds, true);
			if (name3) makeSortFields(name3,  sds, true);
			if (name4) makeSortFields(name4,  sds, true);
			if (name5) makeSortFields(name5,  sds, true);
			if (name6) makeSortFields(name6,  sds, true);
			if (name7) makeSortFields(name7,  sds, true);
			if (name8) makeSortFields(name8,  sds, true);
			std::sort(begin(), end(), recordsetSorter(sds));
		}
		return *this;
	}



	inline recordsetImple& orderBy(const sortFields& orders)
	{
		if (m_recordset.size())
		{
			std::vector<sortDescription> sds;

			for (int i=0;i<(int)orders.size();++i)
				makeSortFields(orders[i].name.c_str(), sds, orders[i].asc);

			std::sort(begin(), end(), recordsetSorter(sds));
		}
		return *this;
	}

	inline recordsetImple& reverse()
	{
		std::reverse(begin(), end());
		return *this;
	}

	inline void appendCol(const _TCHAR* name, int type, short len)
	{
		assert(m_fds->size());
		fielddef fd((*m_fds)[0]);
		fd.len = len;
		fd.pos = 0;
		fd.type = type;
		fd.setName(name);
		m_fds->push_back(&fd);
		for(int i=0;i<(int)m_unionFds.size();++i)
			m_unionFds[i]->push_back(&fd);
		registerMemoryBlock(NULL, fd.len*size(), fd.len
					, mra_outerjoin);

	}

	inline recordsetImple& operator+=(const recordsetImple& r)
	{
		if (!m_fds->canUnion(*r.m_fds))
			THROW_BZS_ERROR_WITH_MSG(_T("Recordsets are different format"));

		m_recordset.reserve(m_recordset.size()+r.size());
		m_unionFds.push_back(r.m_fds);
		for (size_t i=0;i<r.size();++i)
			m_recordset.push_back(r.m_recordset[i]);
		for (size_t i=0;i<r.m_memblock.size();++i)
			m_memblock.push_back(r.m_memblock[i]);
		return *this;
	}

#ifdef _DEBUG
	void dump()
	{
		const fielddefs& fields = *fieldDefs();
		for (int j=0;j<(int)fields.size();++j)
			std::tcout << fields[j].name()  << _T("\t");
		std::tcout << _T("\n");

		for (int i=0;i<(int)size();++i)
		{
			row& m = (operator[](i));
			for (int j=0;j<(int)m.size();++j)
			{
				std::tcout << m[(short)j].c_str()  << _T("\t");
				if (j == (int)m.size() -1)
				   std::tcout << _T("\n");
			}
		}
	}
#endif

};


inline multiRecordAlocatorImple::multiRecordAlocatorImple(recordsetImple* rs)
	:m_rs(rs),m_rowOffset(0),m_addType(0),m_curFirstFiled(0),m_joinRowMap(NULL)
{

}

inline void multiRecordAlocatorImple::init(size_t recordCount, size_t recordLen
		, int addType, const table* tb)
{
	 m_rs->registerMemoryBlock(NULL, recordCount * recordLen
			, recordLen, addType|m_addType, tb);
}

inline unsigned char* multiRecordAlocatorImple::ptr(size_t row, int stat)
{
	int col = (stat == mra_current_block) ? m_curFirstFiled : 0;
	size_t rowNum  = m_joinRowMap ? (*m_joinRowMap)[row+m_rowOffset][0] : row+m_rowOffset;
	return (*m_rs)[rowNum].ptr(col);
}

inline void multiRecordAlocatorImple::setInvalidRecord(size_t row, bool v)
{
	if (m_joinRowMap)
	{
		const std::vector<int>& map = (*m_joinRowMap)[row+m_rowOffset];
		for (int j=0;j<(int)map.size();++j)
			(*m_rs)[map[j]].setInvalidRecord(v);
	}else
		(*m_rs)[row+m_rowOffset].setInvalidRecord(v);


}

inline void multiRecordAlocatorImple::duplicateRow(int row, int count)
{
	m_rs->duplicateRow(row, count);
}

inline void multiRecordAlocatorImple::removeLastMemBlock(int row)
{
	(*m_rs)[row].removeLastMemBlock();
}

template<> inline recordsetImple::iterator begin(recordsetImple& m){return m.begin();}
template<> inline recordsetImple::iterator end(recordsetImple& m){return m.end();}
template<> inline void push_back(recordsetImple& m, row_ptr c){}

/* for groupby */
template<> inline void clear(recordsetImple& m){return m.clearRecords();}

/* for groupby */
template<> inline recordsetImple::key_type resolvKeyValue(recordsetImple& m
				, const std::_tstring& name, bool noexception)
{
	return m.resolvKeyValue(name, noexception);
}

inline row* create(recordsetImple& m, int)
{
	return NULL;
}

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs

#endif //BZS_DB_PROTOCOL_TDAP_CLIENT_MEMRECORDSETIMPLE_H
