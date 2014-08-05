#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_MEMRECORD_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_MEMRECORD_H
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
#include "fields.h"

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

class DLLLIB autoMemory
{

public:
	autoMemory();
	explicit autoMemory(unsigned char* p, size_t s, short* endIndex, bool own);
	autoMemory(const autoMemory& p);
	~autoMemory();
	autoMemory& operator=(const bzs::db::protocol::tdap::client::autoMemory& p);
	unsigned char* ptr;
	short* endFieldIndex;
	unsigned int size;
	bool owner;
};

/** @endcond */



#pragma warning(disable:4251)
#pragma warning(disable:4275)
#define ROW_MEM_BLOCK_RESERVE 4

class DLLLIB memoryRecord : public fieldsBase
{
	friend class multiRecordAlocatorImple;
	friend class recordsetImple;

	std::vector<autoMemory > m_memblock;
protected:
/** @cond INTERNAL */
	inline memoryRecord(fielddefs& fdinfo);
	memoryRecord(const memoryRecord& r);
	void copyToBuffer(table* tb, bool updateOnly=false) const;

	/* return memory block first address which not field ptr address */
	inline unsigned char* ptr(int index) const
	{
		for (int i=0;i<memBlockSize();++i)
			if (*(m_memblock[i].endFieldIndex) > index)
				return 	m_memblock[i].ptr;
		assert(0);
		return NULL;
	}

	inline const autoMemory& memBlockByField(int index) const
	{
		for (int i=0;i<memBlockSize();++i)
			if (*(m_memblock[i].endFieldIndex) > index)
				return m_memblock[i];
		assert(0);
		return *((autoMemory*)0);
	}

	inline const autoMemory& memBlock(int index) const
	{
		return m_memblock[index];
	}

	inline int memBlockSize() const
	{
		return (int) m_memblock.size();
	}

	inline void copyFromBuffer(const table* tb)
	{
		memcpy(ptr(0), tb->fieldPtr(0), m_fns.totalFieldLen());
	}
	void removeLastMemBlock(){m_memblock.pop_back();}

/** @endcond */
public:
	void clear(); //orverride
	void setRecordData(unsigned char* ptr, size_t size
			, short* endFieldIndex, bool owner = false);

	static memoryRecord* create(fielddefs& fdinfo);
	static void release(fieldsBase* p);
};

#pragma warning(default:4275)
#pragma warning(default:4251)

/* non copyable*/
class DLLLIB writableRecord : public memoryRecord
{
	fielddefs* m_fddefs;
	short m_endIndex;
	table* m_tb;

	writableRecord(table* tb, const aliasMap_type* alias);
	writableRecord(const writableRecord&);
	writableRecord& operator=(const writableRecord&);

	fielddefs* fddefs();

public:
	~writableRecord();
	bool read(bool KeysetAlrady=false);
	void insert();
	void del(bool KeysetAlrady=false);
	void update();
	void save();

	static writableRecord* create(table* tb, const aliasMap_type* alias);

};

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs

#endif //BZS_DB_PROTOCOL_TDAP_CLIENT_MEMRECORD_H

