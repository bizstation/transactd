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
#include "nsdatabase.h"
#include "table.h"
#include "stringConverter.h"
#include <bzs/rtl/stringBuffers.h>
#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <map>

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

class fieldShare
{
    friend class field;
    friend class table;
    friend class recordCache;

private:
    struct
    {
    unsigned char myDateTimeValueByBtrv: 1;
    unsigned char trimPadChar: 1;
    unsigned char usePadChar: 1;
    unsigned char logicalToString: 1;
    };

    stringConverter* cv;
    std::vector<boost::shared_array<char> >blobs;
    bzs::rtl::stringBuffer strBufs;

public:
    fieldShare() : strBufs(4096),myDateTimeValueByBtrv(true), trimPadChar(true),
        usePadChar(true), logicalToString(false)
    {
        cv = new stringConverter(nsdatabase::execCodePage(), nsdatabase::execCodePage());
    }

    virtual ~fieldShare()
    {
        delete cv;
        cv = NULL;
    }
};


class fieldInfo : private fieldShare
{

    std::vector<fielddef> m_fields;
    boost::unordered_map<std::_tstring, int> m_map;


public:
    short m_stat;

    fieldInfo():fieldShare(),m_stat(0){};

    void addAllFileds(tabledef* def)
    {
        for (int i=0;i<def->fieldCount;++i)
            push_back(&def->fieldDefs[i]);
    }

    void push_back(const fielddef* p)
    {
        m_fields.push_back(*p);
        m_map[p->name()] = m_fields.size() - 1;
    }

    void remove(index)
    {
        m_fields.erase(m_fields.begin() + index);
        m_map.erase(m_fields[index].name());
    }
    void reserve(size_t size){m_fields.reserve(size);}

    void clear()
    {
        m_fields.clear();
        m_map.clear();
    }
    inline int indexByName(const std::_tstring& name)const
    {
        if(m_map.count(name)==0) return -1;

        return m_map.at(name);
    }
    inline const fielddef& operator[] (int index) const
    {
        assert(index >= 0  && index < m_fields.size());
        return m_fields[index];
    }
    inline const fielddef& operator[] (const _TCHAR* name) const {return m_fields[indexByName(std::_tstring(name))];}
    inline const fielddef& operator[] (const std::_tstring& name)const{return m_fields[indexByName(name)];}
    inline size_t size() const {return m_fields.size();}

};

class fieldsBase
{
    fieldInfo& m_fns;

    virtual unsigned char* ptr(int index) const = 0;

public:
    virtual ~fieldsBase(){};

    inline explicit fieldsBase(fieldInfo& fns):m_fns(fns){}

    inline field operator[](size_t index) const
    {
        return field(ptr((short)index), m_fns[(short)index], &m_fns);
    }

    inline field operator[](const _TCHAR* name) const
    {
		int index = m_fns.indexByName(name);
		return field(ptr(index), m_fns[index], &m_fns);
	}

    inline field operator[](const std::_tstring& name) const
    {
        return operator[](name.c_str());
    }

    inline size_t size() const {return m_fns.size();}


    inline field fd(size_t index) const
    {
        return field(ptr((short)index), m_fns[(short)index], &m_fns);
    }

    inline field fd(const _TCHAR* name) const
    {
        int index = m_fns.indexByName(name);
		return field(ptr(index), m_fns[index], &m_fns);
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
            :fieldsBase(*((fieldInfo*)0)),m_tb(*((table*)0)){}
    inline explicit fields(table& tb)
            :fieldsBase(*(tb.m_fdinfo)),m_tb(tb){}

    inline explicit fields(table_ptr tb)
            :fieldsBase(*((*tb).m_fdinfo)),m_tb(*tb){}

    inline void clearValues(){m_tb.clearBuffer();}
    inline table& tb() const {return m_tb;}
    short inproc_size() const{return m_tb.getCurProcFieldCount();}
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

