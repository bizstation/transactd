#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_REQUSET_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_REQUSET_H
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

#include <bzs/db/protocol/tdap/tdapRequest.h>



namespace bzs
{
	namespace netsvc{namespace client{class connection;}}


namespace db
{
	namespace engine{namespace mysql{class table;}}
	
namespace protocol
{
namespace tdap
{
namespace client
{

class request : public bzs::db::protocol::tdap::request
{
public:
	clientID*	cid;

	request():bzs::db::protocol::tdap::request()
			,cid(NULL){};

	inline void parse(const char* p)
	{
		p += sizeof(unsigned int);
		paramMask = *((uchar_td*)p);
		p += sizeof(uchar_td);
		op = *((ushort_td*)p);
		p += sizeof(ushort_td);
	
		if (P_MASK_POSBLK & paramMask)
		{
			memcpy(pbk, p, POSBLK_SIZE);
			p += POSBLK_SIZE;
		}
		if (P_MASK_DATALEN & paramMask)
		{
			uint_td tmp = *((uint_td*)p);
			memset(data, 0, *datalen);
			if (*datalen < tmp)
				result = STATUS_BUFFERTOOSMALL;
			else
				*datalen = tmp;
			p += sizeof(uint_td);
		}
		if (P_MASK_EX_SENDLEN & paramMask)
		{
			memcpy(data, p, (*(ushort_td*)p));
			p += (*(ushort_td*)p);
	
		}
		else if (P_MASK_DATA & paramMask)
		{
			memcpy(data, p, *datalen);
			p += *datalen;
		}
		if (P_MASK_KEYBUF & paramMask)
		{
			if (keylen < *((keylen_td*)p))
			{
				result = STATUS_KEYBUFFERTOOSMALL;
				return ;
			}
			memset(keybuf, 0, keylen);
			keylen = *((keylen_td*)p);
			p += sizeof(keylen_td);
			memcpy(keybuf, p, keylen);
            p += keylen;
		}
		if (P_MASK_KEYNUM & paramMask)
		{
			keyNum =  *((char_td*)p);
			p += sizeof(char_td);
		}
		if (paramMask & P_MASK_BLOBBODY)
		{
			blobHeader = (const bzs::db::blobHeader*)p;
			if (blobHeader->rows)
				blobHeader->resetCur();
		}
		else
			blobHeader = NULL;
	
	}
	inline unsigned int serialize(char* buf)
	{
		char* p = buf;
		unsigned int totallen = sizeof(unsigned int) + sizeof(uchar_td)
				+  sizeof(ushort_td) +  CLIENTID_SIZE;
		if (P_MASK_POSBLK & paramMask)
			totallen += POSBLK_SIZE;
	
		if (P_MASK_EX_SENDLEN & paramMask)
			totallen += *((ushort_td*)data);
		else if (P_MASK_DATA & paramMask)
			totallen += *datalen;
	
		if (P_MASK_DATALEN & paramMask)
			totallen += sizeof(uint_td);
		if (P_MASK_KEYBUF & paramMask)
			totallen += keylen+sizeof(keylen_td) + 1;//+1 is null terminate byte
		if (P_MASK_KEYNUM & paramMask)
			totallen += sizeof(char_td);
		memcpy(p, &totallen, sizeof(unsigned int));
		p += sizeof(unsigned int);
	
		memcpy(p, &paramMask,  sizeof(uchar_td));
		p += sizeof(uchar_td);
	
		memcpy(p, &op,  sizeof(ushort_td));
		p += sizeof(ushort_td);
	
		if (P_MASK_POSBLK & paramMask)
		{
			memcpy(p, pbk,  POSBLK_SIZE);
			p += POSBLK_SIZE;
		}
	
		if (P_MASK_DATALEN & paramMask)
		{
			memcpy(p, datalen,  sizeof(uint_td));
			p += sizeof(uint_td);
		}
	
		if (P_MASK_EX_SENDLEN & paramMask)
		{
			unsigned int v = *((unsigned int*)data);
			v &= 0xFFFFFFF; //28bit
			memcpy(p, data,  v);
			p += v;
		}
		else if (P_MASK_DATA & paramMask)
		{
			memcpy(p, data, *datalen);
			p += *datalen;
		}
	
		if (P_MASK_KEYBUF & paramMask)
		{
			memcpy(p, &keylen, sizeof(keylen_td));
			p += sizeof(keylen_td);
	
			memcpy(p, keybuf, keylen);
			p += keylen;
			*p = 0x00;
			++p;
		}
		if (P_MASK_KEYNUM & paramMask)
		{
			memcpy(p, &keyNum, sizeof(char_td));
			p += sizeof(char_td);
		}
	
		memcpy(p, &cid->id, CLIENTID_SIZE);
		p += CLIENTID_SIZE;

		return (unsigned int)(p - buf);
	}





};

}//namespace client
}//namespace protocol
}//namespace tdap
}//namespace db
}//namespace bzs


#endif //BZS_DB_PROTOCOL_TDAP_CLIENT_REQUSET_H
