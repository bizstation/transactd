#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_BULKINSERT_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_BULKINSERT_H
/*=================================================================
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
=================================================================*/
#include <bzs/db/protocol/tdap/tdapcapi.h>
#include <limits.h>
#include "nsTable.h"

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

class bulkInsert
{
	char* m_buf;
	char* m_ptr;
	int m_count;
	int m_buflen;
	int m_maxBuflen;

public:
	bulkInsert(int max) : m_maxBuflen(max)
	{

		m_buf = new char[BULKBUFSIZE];
		m_ptr = m_buf + sizeof(ushort_td);
		m_count = 0;
		m_buflen = BULKBUFSIZE;

	}

	~bulkInsert()
	{
		delete [] m_buf;
	}

	char* reallocBuffer(char* buf, int oldsize, int newSize)
	{
		char* p = new char[newSize];
		memcpy(p, buf, oldsize);
		delete [] buf;

		return p;

	}

	ushort_td insert(const char* p, ushort_td size, nstable* tb)
	{
		ushort_td ins_rows = 0;
		if (m_count == (int)(USHRT_MAX - 3))
		{
			ins_rows = tb->commitBulkInsert(true /* auto */);
			m_ptr = m_buf +sizeof(ushort_td);
			m_count = 0;
		}

		// check over run. current size + need size
		if ((m_ptr - m_buf )+ size + sizeof(ushort_td) > (uint_td)m_buflen)
		{
			if ((int)(m_buflen + BULKBUFSIZE) > m_maxBuflen)
			{
				ins_rows = tb->commitBulkInsert(true /* auto */);
				m_ptr = m_buf +sizeof(ushort_td);
				m_count = 0;
			}
			else
			{
				size_t pos = m_ptr - m_buf;
				m_buf = reallocBuffer(m_buf, m_buflen, m_buflen + BULKBUFSIZE);
				m_buflen += BULKBUFSIZE;
				m_ptr = m_buf + pos;
			}
		}
		memcpy(m_ptr, &size, sizeof(ushort_td));
		m_ptr += sizeof(ushort_td);
		memcpy(m_ptr, p, size);
		m_ptr += size;
		m_count++;
		return ins_rows;

	}

	void* data()
	{
		(*(ushort_td*)m_buf) = (ushort_td)m_count;
		return m_buf;
	}

	uint_td dataLen()
	{
		return (uint_td)(m_ptr - m_buf);
	}

	int count() {return m_count;}

};

}//namespace client
}//namespace tdap
}//namespace protocol
}//namespace db
}//namespace bzs

#endif//BZS_DB_PROTOCOL_TDAP_CLIENT_BULKINSERT_H
