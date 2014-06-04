#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_FIELDNAME_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_FIELDNAME_H
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
#include <bzs/db/protocol/tdap/client/trdboostapi.h>

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

class fieldNames
{

protected:
	std::vector<std::_tstring> m_keyFields;

public:
	virtual fieldNames& reset()
	{
		m_keyFields.clear();
		return *this;
	}

	fieldNames& keyField(const TCHAR* name, const TCHAR* name1=NULL, const TCHAR* name2=NULL, const TCHAR* name3=NULL
				,const TCHAR* name4=NULL, const TCHAR* name5=NULL, const TCHAR* name6=NULL, const TCHAR* name7=NULL
				,const TCHAR* name8=NULL, const TCHAR* name9=NULL, const TCHAR* name10=NULL)
	{
		m_keyFields.clear();
		m_keyFields.push_back(name);
		if (name1) m_keyFields.push_back(name1);
		if (name2) m_keyFields.push_back(name2);
		if (name3) m_keyFields.push_back(name3);
		if (name4) m_keyFields.push_back(name4);
		if (name5) m_keyFields.push_back(name5);
		if (name6) m_keyFields.push_back(name6);
		if (name7) m_keyFields.push_back(name7);
		if (name8) m_keyFields.push_back(name8);
		if (name9) m_keyFields.push_back(name9);
		if (name10) m_keyFields.push_back(name10);
		return *this;
	}

	const std::vector<std::_tstring>& getKeyFields()const {return m_keyFields;}

};

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs

#endif //BZS_DB_PROTOCOL_TDAP_CLIENT_FIELDNAME_H
