#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_FIELDS_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_FIELDS_H
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
#include "field.h"
#include "table.h"
#include <boost/shared_ptr.hpp>


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


class fieldsBase
{

	virtual unsigned char* ptr(int index) const = 0;
protected:
	fielddefs& m_fns;
	bool m_invalidRecord;
public:

	explicit inline fieldsBase(fielddefs& fns):m_fns(fns),m_invalidRecord(false){}
	virtual ~fieldsBase(){};
	void setInvalidRecord(bool v){m_invalidRecord = v;}
	bool isInvalidRecord()const{return m_invalidRecord;}

	inline field getFieldInternal(short index) const
	{
		return field(ptr((short)index), m_fns[(short)index], &m_fns);
	}

	inline field operator[](short index) const
	{
		if (m_fns.checkIndex(index))
			return field(ptr((short)index), m_fns[(short)index], &m_fns);
		nstable::throwError(_T("Invalid field name or index"), STATUS_INVARID_FIELD_IDX);
		return field(NULL, dummyFd(), &m_fns);
	}

	inline field operator[](const _TCHAR* name) const
	{
		int index = m_fns.indexByName(name);
		return operator[](index);
	}

	inline field operator[](const std::_tstring& name) const
	{
		return operator[](name.c_str());
	}

	inline size_t size() const {return m_fns.size();}


	inline field fd(short index) const
	{
		return operator[](index);
	}

	inline field fd(const _TCHAR* name) const
	{
		int index = m_fns.indexByName(name);
		return operator[](index);
	}

	inline short indexByName(const _TCHAR* name) const
	{
		return m_fns.indexByName(name);
	}

	inline const fielddefs* fieldDefs() const
	{
		return &m_fns;
	}

	inline void setFielddefs(fielddefs& def)
	{
		m_fns = def;
	}

};

typedef boost::shared_ptr<database>database_ptr;
typedef boost::shared_ptr<table>table_ptr;


class fields : public fieldsBase
{
	table& m_tb;
	inline unsigned char* ptr(int index) const
	{
	    return (unsigned char*)m_tb.data();
	}

public:
	inline explicit fields()
			:fieldsBase(*((fielddefs*)0)),m_tb(*((table*)0)){}
	inline explicit fields(table& tb)
			:fieldsBase(*(tb.m_fddefs)),m_tb(tb){}

	inline explicit fields(table_ptr tb)
			:fieldsBase(*((*tb).m_fddefs)),m_tb(*tb){}

	inline void clearValues(){m_tb.clearBuffer();}
	inline table& tb() const {return m_tb;}
	inline short inproc_size() const{return m_tb.getCurProcFieldCount();}
	inline field inproc_fd(short index) const
	{
		return operator[]( m_tb.getCurProcFieldIndex(index));
	}

};


}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs




#endif //BZS_DB_PROTOCOL_TDAP_CLIENT_FIELDS_H

