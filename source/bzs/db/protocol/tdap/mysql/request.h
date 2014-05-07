#ifndef BZS_DB_PROTOCOL_TDAP_MYSQL_REQUSET_H
#define BZS_DB_PROTOCOL_TDAP_MYSQL_REQUSET_H
/*=================================================================
   Copyright (C) 2013 BizStation Corp All rights reserved.

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
#include <bzs/db/engine/mysql/database.h>
#include <bzs/db/protocol/tdap/tdapRequest.h>
#include <bzs/rtl/exception.h>
#include <bzs/db/blobBuffer.h>
#ifdef USE_DATA_COMPRESS
#include <bzs/rtl/lzss.h>
#endif
/** readRecords reserved buffer size
 *
 */
#define RETBUF_EXT_RESERVE_SIZE 11



namespace bzs
{
namespace db
{	
namespace protocol
{
namespace tdap
{
namespace mysql
{

class request : public bzs::db::protocol::tdap::request
{
public:
	
	ushort_td cid;
	char* decodeBuffer;
	char* m_tmp;
	request():bzs::db::protocol::tdap::request(),cid(0)
					,decodeBuffer(0),m_tmp(new char[0XFFFF])
	{
	
	}

	~request()
	{
		delete [] decodeBuffer;
		delete [] m_tmp;
	}
	
	/**
	 *  @param size  allready copied size.
	 */
	inline unsigned int serializeForExt(engine::mysql::table* tb, char* buf, unsigned int size)
	{
		paramMask = (engine::mysql::table::noKeybufResult==false) ? 
						P_MASK_READ_EXT : P_MASK_DATA|P_MASK_DATALEN;
		//paramMask = P_MASK_READ_EXT;
		if (tb->blobFields()) paramMask |=P_MASK_BLOBBODY;
		resultLen = (size - RETBUF_EXT_RESERVE_SIZE);// 4+1+2+4 = 11
	
		int pos = sizeof(unsigned int);									//4
		memcpy(buf + pos, (const char*)(&paramMask), sizeof(uchar_td));	//1
		pos += sizeof(uchar_td);
		memcpy(buf + pos, (const char*)(&result), sizeof(short_td));		//2
		pos += sizeof(short_td);
		memcpy(buf + pos, (const char*)&resultLen, sizeof(uint_td));		//4
		
		/* The result contents is copied allready.*/
	
		//buf + size
		if (paramMask & P_MASK_KEYBUF)
		{
			keylen = tb->keyPackCopy((uchar*)buf + size + sizeof(keylen_td));
			memcpy(buf + size, (const char*)&keylen, sizeof(keylen_td));
			size += sizeof(keylen_td);
			size += keylen;
		}
		memcpy(buf, (const char*)(&size), sizeof(unsigned int));
		return size;
	}

	inline unsigned int serialize(engine::mysql::table* tb, char* buf)
	{
		char* p = buf;
		char* datalenPtr = NULL;
	
		p += sizeof(unsigned int);//space of totalLen
#ifdef USE_DATA_COMPRESS
		//add lzss infomation
		if ((P_MASK_DATA & paramMask) && (resultLen > 1600))
			paramMask |= P_MASK_USELZSS;
#endif
	
		memcpy(p, (const char*)(&paramMask), sizeof(uchar_td));
		p += sizeof(uchar_td);
		
		memcpy(p, (const char*)(&result), sizeof(short_td));
		p += sizeof(short_td);
	
		if (P_MASK_POSBLK & paramMask)
		{
			memcpy(p, (const char*)pbk, POSBLK_SIZE);
			p += POSBLK_SIZE;
		}
		if (P_MASK_DATALEN & paramMask)
		{
			datalenPtr = p;
			p += sizeof(uint_td);
		}
#ifdef USE_DATA_COMPRESS
		if (P_MASK_USELZSS & paramMask)
		{
			if (tb && (data == tb->record()))
			{
				resultLen = tb->recordPackCopy(m_tmp, 0);
				data = m_tmp;
			}
			uint_td compSize = bzs::rtl::lzssEncode(data, resultLen, p + sizeof(uint_td));
			memcpy(p, &compSize, sizeof(uint_td));
			p += compSize + sizeof(uint_td);
		}
		else
#endif
		if (P_MASK_DATA & paramMask)
		{
			if (tb && (data == tb->record()))
				resultLen = tb->recordPackCopy(p, 0);
			else
				memcpy(p, (const char*)data, resultLen);
			p += resultLen;
		}

		if (P_MASK_DATALEN & paramMask)
			memcpy(datalenPtr, (const char*)&resultLen, sizeof(uint_td));
	
		if (tb && (P_MASK_KEYBUF & paramMask))
		{
			keylen = tb->keyPackCopy((uchar*)p + sizeof(keylen_td));
			memcpy(p, (const char*)&keylen, sizeof(keylen_td));
			p += sizeof(keylen_td);
			p += keylen;
		}
		unsigned int totallen = (unsigned int)(p - buf);
		memcpy(buf, &totallen, sizeof(unsigned int));
		return totallen;
	}

	inline void parse(const char* p)
	{
		p += sizeof(unsigned int);
		paramMask = *((uchar_td*)p);
		p += sizeof(uchar_td);
		op = *((ushort_td*)p);
		p += sizeof(ushort_td);
	
		if (P_MASK_POSBLK & paramMask)
		{
			pbk = (posblk*)p;
			p += POSBLK_SIZE;
		}
		if (P_MASK_DATALEN & paramMask)
		{
			datalen = (uint_td*)p;
			p += sizeof(uint_td);
		}
#ifdef USE_DATA_COMPRESS
		if (P_MASK_USELZSS & paramMask)
		{
			uint_td compSize = *(uint_td*)p;
			p += sizeof(uint_td);

			if (decodeBuffer)
				delete [] decodeBuffer;
			uint_td dataSize = bzs::rtl::lzssDecode(p, NULL);
			decodeBuffer = new char[dataSize];
			bzs::rtl::lzssDecode(p, decodeBuffer);
			data = decodeBuffer;
			p += compSize;
		}
		else
#endif
		if (P_MASK_EX_SENDLEN & paramMask)
		{
			data = (void_td*)p;
			int v = *((int*)p);
			v &= 0xFFFFFFF; //28bit
			p += v;
		}
		else if (P_MASK_DATA & paramMask)
		{
			data = (void_td*)p;
			p += *datalen;
		}
		if (P_MASK_KEYBUF & paramMask)
		{
			keylen = *((keylen_td*)p);
			p += sizeof(keylen_td);
			keybuf = (void_td*)p;
			p += keylen + 1;//null terminate for server
		}
		if (P_MASK_KEYNUM & paramMask)
		{
			keyNum =  *((char_td*)p);
			p += sizeof(char_td);
		}
		cid = *((ushort_td*)p);
		p += sizeof(ushort_td);

		if (paramMask & P_MASK_BLOBBODY)
		{
			blobHeader = (const bzs::db::blobHeader*)p;
			if (blobHeader->rows)
				blobHeader->resetCur();
		}
		else
			blobHeader = NULL;
	
	}
};



}//namespace mysql
}//namespace protocol
}//namespace db
}//namespace tdap
}//namespace bzs


#endif //BZS_DB_PROTOCOL_TDAP_MYSQL_REQUSET_H
