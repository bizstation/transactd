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
#include "fieldNameAlias.h"

#ifdef BCB_32
#	pragma option push
#		pragma option -O1
#		include <boost/unordered_map.hpp>
#	pragma option pop
#else
#	include <boost/unordered_map.hpp>
#endif

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

typedef boost::unordered_map<std::_tstring, std::_tstring> map_type;

struct fdnImple
{
	map_type map;
};


fdNmaeAlias::fdNmaeAlias():m_imple(new fdnImple){}

fdNmaeAlias::fdNmaeAlias(const fdNmaeAlias& r)
			:m_imple(new fdnImple(*r.m_imple)){}

fdNmaeAlias& fdNmaeAlias::operator=(const fdNmaeAlias& r)
{
	if (this != &r)
		*this->m_imple =  *r.m_imple;
	return *this;
}

fdNmaeAlias::~fdNmaeAlias()
{
	delete m_imple;
}


void fdNmaeAlias::set(const _TCHAR* src, const _TCHAR* dst)
{
	m_imple->map[src] = dst;
}

const _TCHAR* fdNmaeAlias::get(const _TCHAR* src) const
{
	try
	{
		if (m_imple->map.count(src))
			return m_imple->map.at(src).c_str();
		return _T("");
	}
	catch(...)
	{

	}
	return _T("");

}

const _TCHAR* fdNmaeAlias::resolv(const _TCHAR* dst) const
{
	map_type::const_iterator it = m_imple->map.begin();
	while(it != m_imple->map.end())
	{
		if ((*it).second == dst)
			return (*it).first.c_str();
		++it;
	}
	return _T("");

}

void fdNmaeAlias::clear(){m_imple->map.clear();}

void fdNmaeAlias::reverseAliasNamesQuery(queryBase& q) const
{
	map_type::const_iterator it = m_imple->map.begin();
	while(it != m_imple->map.end())
	{
		q.reverseAliasName((*it).second.c_str(), (*it).first.c_str());
		++it;
	}
}	

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs


