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
#include "trdormapi.h"

namespace tdc =  bzs::db::protocol::tdap::client;
namespace td =  bzs::db::protocol::tdap;
class map_orm;
typedef tdc::memoryRecord row;
typedef boost::shared_ptr<tdc::memoryRecord> row_ptr;
typedef int key_type;



class recordsetSorter
{
	const std::vector<int>& m_fieldNums;
public:
	recordsetSorter(const std::vector<int>& fieldNums):m_fieldNums(fieldNums)
	{

	}
	bool operator()(const row_ptr& l, const row_ptr r) const
	{
		std::vector<int>::const_iterator it = m_fieldNums.begin();
		while (it != m_fieldNums.end())
		{
			int ret = (*l)[*it].comp((*r)[*it], 0);
			if (ret) return (ret < 0);
			++it;
		}
		return false;
	}
};

#ifndef SWIG
class multiRecordAlocatorImple : public tdc::multiRecordAlocator
{
	class recordset* m_rs;
	int m_rowOffset;
	int m_addType;
	int m_curFirstFiled;
	const std::vector< std::vector<int> >* m_joinRowMap;
	
public:
	inline multiRecordAlocatorImple(recordset* rs);
	inline void init(size_t recordCount, size_t recordLen, int addType, const tdc::table* tb);
	inline unsigned char* ptr(size_t row, int stat);
	inline void setRowOffset(int v){m_rowOffset = v;}
	inline void setJoinType(int v){m_addType = v;}
	inline void setInvalidRecord(size_t row, bool v);
	inline void setCurFirstFiled(int v){m_curFirstFiled = v;}
	inline void setJoinRowMap(const std::vector< std::vector<int> >* v/*, size_t size*/){m_joinRowMap = v;/*m_joinMapSize = size;*/}
	inline const std::vector< std::vector<int> >* joinRowMap()const {return m_joinRowMap;}
};
#endif



template <class T>
void setValue(row& row, key_type key, const T& value)
{
	row[key] = value;
}


class recordset
{
	friend class multiRecordAlocatorImple;
	boost::shared_ptr<tdc::fielddefs> m_fds;
	boost::shared_ptr<multiRecordAlocatorImple> m_mra;
	std::vector<row_ptr> m_recordset;
	std::vector<boost::shared_ptr<tdc::autoMemory> > m_memblock;

	/* for registerMemoryBlock temp data */
	size_t m_joinRows;
	
	/* 
		for optimazing join.
		If the first reading is using by unique key , set that field count.
	*/
	short m_uniqueReadMaxField;
public:
	typedef std::vector<boost::shared_ptr<tdc::memoryRecord> >::iterator iterator;

private:

	void registerMemoryBlock(unsigned char* ptr, size_t size, size_t recordLen
					, int addtype, const tdc::table* tb=NULL)
	{
		tdc::autoMemory* am = new tdc::autoMemory(ptr, size, 0 , true);
		m_memblock.push_back(boost::shared_ptr<tdc::autoMemory>(am));
		unsigned char* p = am->ptr;
		//copy fileds
		if (addtype & tdc::multiRecordAlocator::mra_nextrows)
		{
			if (addtype == tdc::multiRecordAlocator::mra_nextrows)
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
			if (tb && (addtype == tdc::multiRecordAlocator::mra_nojoin))
			{
				const td::keydef& kd = tb->tableDef()->keyDefs[tb->keyNum()];
				m_uniqueReadMaxField =  (kd.segments[0].flags.bit0 == false) ? (short)m_fds->size():0;
			}
		}
		
		*(am->endFieldIndex) = (short)m_fds->size();
		size_t rows = size/recordLen;

		// set record pointer to each record
		if ((addtype & tdc::multiRecordAlocator::mra_innerjoin)
						|| (addtype & tdc::multiRecordAlocator::mra_outerjoin))
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
				boost::shared_ptr<tdc::memoryRecord> rec(tdc::memoryRecord::create(*m_fds), &tdc::memoryRecord::release);
 				rec->setRecordData(p + recordLen*i, 0, am->endFieldIndex, false);
				m_recordset.push_back(rec);
			}
		}
	}

	void makeSortFields(const _TCHAR* name, std::vector<int>& fieldNums)
	{
		int index =  m_fds->indexByName(name);
		if (index ==-1)
			THROW_BZS_ERROR_WITH_MSG(_T("oorderBy:Invalid field name"));
		fieldNums.push_back(index);
	}

public:
	recordset():m_fds(tdc::fielddefs::create(), &tdc::fielddefs::destroy)
		,m_mra(new ::multiRecordAlocatorImple(this)),m_uniqueReadMaxField(0)
	{

	};

	~recordset()
	{

	}
	inline short uniqueReadMaxField() const{return m_uniqueReadMaxField;}
	inline void clearRecords()
	{
		m_recordset.clear();
		m_uniqueReadMaxField = 0;
	}

	const tdc::fielddefs* fieldDefs() const {return m_fds.get();}

	inline void clear(){clearRecords();m_fds->clear();m_memblock.clear();}

	inline row_ptr& operator[](size_t index){return m_recordset[index];}

	inline row_ptr& first() {return m_recordset[0];}

	inline row_ptr& last() {return m_recordset[m_recordset.size() - 1];}

	inline recordset top(recordset& c, int n)
	{
		c = *this;
		c.clearRecords();
		for (int i=0;i<n;++i)
			c.push_back(m_recordset[i]);
		return c;
	}

	inline iterator begin(){return m_recordset.begin();}

	inline iterator end(){return m_recordset.end();}

	inline iterator erase(size_t index){return m_recordset.erase(m_recordset.begin()+index);}

	inline iterator erase(const iterator& it){return m_recordset.erase(it);}

	inline void push_back(row_ptr r){m_recordset.push_back(r);};

	inline size_t size()const {return m_recordset.size();}

	inline size_t count()const {return m_recordset.size();}

	void readBefore(const tdc::table_ptr tb, const tdc::aliasMap_type* alias)
	{
		 tb->setMra(m_mra.get());
		 m_fds->setAliases(alias);
	}

	typedef tdc::fielddefs header_type;
	typedef ::key_type key_type;
	typedef row_ptr row_type;

	key_type resolvKeyValue(const std::_tstring& name, bool noexception=false)
	{
		int index = m_fds->indexByName(name);
		if (index != -1)
			return index;
		if (!noexception)
			THROW_BZS_ERROR_WITH_MSG(_T("groupQuery:Invalid key name"));

		return (key_type)m_fds->size();
	}

	void removeField(int index)
	{
		m_fds->remove(index);
		for(int i=0;i<(int)m_memblock.size();++i)
		{
			if (*(m_memblock[i]->endFieldIndex) > index)
			{
				short v = *(m_memblock[i]->endFieldIndex) -1;
				*(m_memblock[i]->endFieldIndex) = v;
			}
		}
	}

	template <class FUNC>
	recordset& groupBy(tdc::groupQuery& gq, FUNC func)
	{
		gq.grouping(*this, func);
		return *this;
	}

	recordset& orderBy(const _TCHAR* name1 , const _TCHAR* name2=NULL,
					 const _TCHAR* name3=NULL,const _TCHAR* name4=NULL,
					 const _TCHAR* name5=NULL, const _TCHAR* name6=NULL,
					 const _TCHAR* name7=NULL, const _TCHAR* name8=NULL)
	{
		std::vector<int> fieldNums;
		makeSortFields(name1, fieldNums);
		if (name2) makeSortFields(name2, fieldNums);
		if (name3) makeSortFields(name3, fieldNums);
		if (name4) makeSortFields(name4, fieldNums);
		if (name5) makeSortFields(name5, fieldNums);
		if (name6) makeSortFields(name6, fieldNums);
		if (name7) makeSortFields(name7, fieldNums);
		if (name8) makeSortFields(name8, fieldNums);
		std::sort(begin(), end(), recordsetSorter(fieldNums));
		return *this;
	}

	inline recordset& reverse()
	{
		std::reverse(begin(), end());
		return *this;
	}

	void appendCol(const _TCHAR* name, int type, short len)
	{
		assert(m_fds->size());
		td::fielddef fd((*m_fds)[0]);
		fd.len = len;
		fd.pos = 0;
		fd.type = type;
		fd.setName(name);

		m_fds->push_back(&fd);
		registerMemoryBlock(NULL, fd.len*size(), fd.len
					, tdc::multiRecordAlocator::mra_outerjoin);

	}

	recordset& operator+=(const recordset& r)
	{
		if (!m_fds->canUnion(*r.m_fds))
			THROW_BZS_ERROR_WITH_MSG(_T("Recordsets are different format"));

		m_recordset.reserve(m_recordset.size()+r.size());
		for (size_t i=0;i<r.size();++i)
		{
			m_recordset.push_back(r.m_recordset[i]);
			m_recordset[m_recordset.size()-1]->setFielddefs(*m_fds);
		}
		for (size_t i=0;i<r.m_memblock.size();++i)
			m_memblock.push_back(r.m_memblock[i]);
		return *this;
	}

};

inline multiRecordAlocatorImple::multiRecordAlocatorImple(recordset* rs)
	:m_rs(rs),m_rowOffset(0),m_addType(0),m_curFirstFiled(0),m_joinRowMap(NULL)
{

}

inline void multiRecordAlocatorImple::init(size_t recordCount, size_t recordLen
		, int addType, const tdc::table* tb)
{
	 m_rs->registerMemoryBlock(NULL, recordCount * recordLen
			, recordLen, addType|m_addType, tb);
}

inline unsigned char* multiRecordAlocatorImple::ptr(size_t row, int stat)
{
	int col = (stat == tdc::mra::mra_current_block) ? m_curFirstFiled : 0;
 	size_t rowNum  = m_joinRowMap ? (*m_joinRowMap)[row+m_rowOffset][0] : row+m_rowOffset;
	return (*m_rs)[rowNum]->ptr(col);
}

inline void multiRecordAlocatorImple::setInvalidRecord(size_t row, bool v)
{
	(*m_rs)[row+m_rowOffset]->setInvalidRecord(v);
}

namespace bzs{namespace db{namespace protocol{namespace tdap{namespace client
{

inline recordset::iterator begin(recordset& m){return m.begin();}
inline recordset::iterator end(recordset& m){return m.end();}

inline void push_back(recordset& m, row_ptr c)
{
	//m.push_back(c);
}

/* for groupby */
inline void clear(recordset& m)
{
	return m.clearRecords();
}

/* for groupby */
inline recordset::key_type resolvKeyValue(recordset& m, const std::_tstring& name
	, bool noexception=false)
{
	return m.resolvKeyValue(name, noexception);
}

}}}}}


inline row* create(recordset& m, int)
{
	return NULL;
}



class map_orm_fdi
{
	friend class map_orm;
	const tdc::table* m_tb;
public:
	void init(tdc::table* tb) {m_tb = tb;}
};

inline map_orm_fdi* createFdi(map_orm_fdi * ){return new map_orm_fdi();}
inline void destroyFdi(map_orm_fdi * p){delete p;}
inline void initFdi(map_orm_fdi * fdi, tdc::table* tb){fdi->init(tb);}



class map_orm
{
	const map_orm_fdi& m_fdi;
	short m_autoIncFiled;

	bool comp(row& lm, row& rm, const _TCHAR* name, int index) const
	{
		return lm[index].comp(rm[index], 0);
	}


public:
	map_orm(const map_orm_fdi& fdi):m_fdi(fdi),m_autoIncFiled(-2){}

	bool compKeyValue(row& l, row& r, int keyNum) const
	{
		const td::tabledef* def = m_fdi.m_tb->tableDef();
		const td::keydef* kd = &def->keyDefs[keyNum];
		for (int i=0;i<kd->segmentCount;++i)
		{
			short n =  kd->segments[i].fieldNum;
			const td::fielddef* fd = &def->fieldDefs[n];
			int ret = comp(l, r, fd->name(), n);
			if (ret)return (ret < 0);

		}
		return 0;
	}

	void setKeyValues(row& m, const tdc::fields& fds, int keyNum)
	{
		const tdc::autoMemory& mb =  m.memBlock(0);
		memcpy(fds.tb().fieldPtr(0), mb.ptr, mb.size);

	}

	void writeMap(row& m, const tdc::fields& fds, int optipn)
	{
		const tdc::autoMemory& mb =  m.memBlock(0);
		memcpy(fds.tb().fieldPtr(0), mb.ptr, mb.size);
	}

	template <class T>
	void readMap(T& m, const tdc::fields& fds, int optipn)
	{
		//needlessness
	}

	void readAuntoincValue(row& m, const tdc::fields& fds, int optipn)
	{
		//needlessness
	}

	typedef row         mdl_typename;
	typedef map_orm_fdi fdi_typename;
	typedef tdc::mdlsHandler< map_orm, recordset> collection_orm_typename;


};

template <class row_type=row_ptr, class key_type=int, class value_type_=__int64>
class sum
{
	value_type_ m_value;
	key_type m_resultKey;
public:
	sum():m_value(0){}

	void setResultKey(key_type key)
	{
		m_resultKey = key;
	}

	void operator()(const row_type& row)
	{
        value_type_ tmp;
		m_value += (*row)[m_resultKey].value(tmp);
	}
	value_type_ result()const{return m_value;}

	void reset(){m_value = 0;}
	typedef typename value_type_ value_type;
};

template <class row_type=row_ptr, class key_type=int, class value_type_=__int64>
class avg
{
	value_type_ m_value;
	value_type_ m_count;
	key_type m_resultKey;
public:
	avg():m_value(0),m_count(0){}

	void setResultKey(key_type key)
	{
		m_resultKey = key;
	}

	void operator()(const row_type& row)
	{
		++m_count;
		value_type_ tmp;
		m_value += (*row)[m_resultKey].value(tmp);
	}

	value_type_ result()const{return m_value/m_count;}

	void reset(){m_value = 0;m_count=0;}
	typedef typename value_type_ value_type;
};

template <class row_type=row_ptr, class key_type=int, class value_type_=__int64>
class count
{
	value_type_ m_count;
	key_type m_resultKey;
public:
	count():m_count(0){}

	void setResultKey(key_type key)
	{
		m_resultKey = key;
	}

	void operator()(const row_type& row)
	{
		++m_count;
	}

	value_type_ result()const{return m_count;}

	void reset(){m_count=0;}
	typedef typename value_type_ value_type;
};

#undef min
template <class row_type=row_ptr, class key_type=int, class value_type_=__int64 >
class min
{
	value_type_ m_value;
	key_type m_resultKey;
	bool m_flag ;
public:
	min():m_value(0),m_flag(true){}

	void setResultKey(key_type key)
	{
		m_resultKey = key;
	}

	void operator()(const row_type& row)
	{
		value_type_ tmp = (*row)[m_resultKey].value(tmp);
		if (m_flag || m_value > tmp)
		{
			m_flag = false;
			m_value = tmp;
		}
	}

	value_type_ result()const{return m_value;}

	void reset(){m_value = 0;m_flag = true;}
	typedef typename value_type_ value_type;
};

#undef max
template <class row_type=row_ptr, class key_type=int, class value_type_=__int64>
class max
{
	value_type_ m_value;
	key_type m_resultKey;
	bool m_flag ;
public:
	max():m_value(0),m_flag(true){}

	void setResultKey(key_type key)
	{
		m_resultKey = key;
	}

	void operator()(const row_type& row)
	{
		value_type_ tmp = (*row)[m_resultKey].value(tmp);
		if (m_flag || m_value < tmp)
		{
			m_flag = false;
			m_value = tmp;
		}
	}

	value_type_ result()const{return m_value;}

	void reset(){m_value = 0;m_flag = true;}
	typedef typename value_type_ value_type;
};


typedef sum<row_ptr, int, double> group_sum;

typedef count<row_ptr, int, int> group_count;

typedef tdc::activeTable<map_orm> queryTable;

#endif //BZS_DB_PROTOCOL_TDAP_CLIENT_MEMRECORDSET_H
