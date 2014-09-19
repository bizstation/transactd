#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_FILEDDF_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_FILEDDF_H
/* =================================================================
 Copyright (C) 2000-2013 BizStation Corp All rights reserved.

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
 ================================================================= */
#include "table.h"

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

#pragma option -a1
pragma_pack1

struct SFILEDDF
{
	ushort_td id;
	char tablename[20];
	char filename[64];
	ushort_td flag;
	char filler[9];
};

#pragma option -a
pragma_pop



class fileDDF : public nstable
{
private:
	_TCHAR m_DataDir[256];
	SFILEDDF datbuf;
	void doOpen(const _TCHAR* Dir, char_td mode, const _TCHAR* OwnerName);
	keylen_td writeKeyData();
	void writeRecordData();
	void onReadAfter();

protected:
	~fileDDF();

public:
	ushort_td id;
	char tablename[21];
	char filename[65];
	ushort_td flag;
	char filler[9];
	char keybuf[128];

	explicit fileDDF(nsdatabase *pbe);
	void createTable(const _TCHAR* fullpath);
	using nstable::open;


};

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs
#endif//BZS_DB_PROTOCOL_TDAP_CLIENT_FILEDDF_H
