#ifndef BZS_DB_PROTOCOL_TDAP_TDAPREQUSET_H
#define BZS_DB_PROTOCOL_TDAP_TDAPREQUSET_H
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

#include <bzs/db/protocol/tdap/tdapcapi.h>
#include <bzs/db/blobBuffer.h>
#include <iostream>
#include <vector>
#include <string.h>
#include <boost/asio/buffer.hpp>
#include <bzs/env/compiler.h>


namespace bzs
{
	namespace netsvc{namespace client{class connection;}}
namespace db
{
namespace protocol
{
namespace tdap
{

#define P_MASK_OP			0
#define P_MASK_POSBLK		1
#define P_MASK_DATA			2
#define P_MASK_DATALEN		4
#define P_MASK_KEYBUF		8
#define P_MASK_KEYNUM		16
#define P_MASK_EX_SENDLEN	32  //< The data length which transmits to a client is described at 2 bytes of the data buffer head.
#define P_MASK_BLOBBODY		64
#define P_MASK_FINALRET	    128 //server sent final result
#define P_MASK_FINALDATALEN	256 //server sent final result





#define P_MASK_ALL			P_MASK_POSBLK|P_MASK_DATA|P_MASK_DATALEN|P_MASK_KEYBUF|P_MASK_KEYNUM
#define P_MASK_KEYONLY 		P_MASK_KEYBUF|P_MASK_KEYNUM
#define P_MASK_NOKEYBUF		P_MASK_POSBLK|P_MASK_DATA|P_MASK_DATALEN|P_MASK_KEYNUM
#define P_MASK_POS_LEN_KEY	P_MASK_POSBLK|P_MASK_DATALEN|P_MASK_KEYNUM
#define P_MASK_KEYNAVI		P_MASK_POSBLK|P_MASK_DATALEN|P_MASK_KEYBUF|P_MASK_KEYNUM
#define P_MASK_NODATA		P_MASK_POSBLK|P_MASK_DATALEN|P_MASK_KEYBUF

//server side
#define P_MASK_READRESULT	P_MASK_POSBLK|P_MASK_DATA|P_MASK_DATALEN
#define P_MASK_STAT			P_MASK_DATA|P_MASK_DATALEN
#define P_MASK_READ_EXT		P_MASK_STAT|P_MASK_KEYBUF
#define P_MASK_MOVPOS		P_MASK_POSBLK|P_MASK_DATA|P_MASK_DATALEN|P_MASK_KEYBUF
#define P_MASK_INS_AUTOINC	P_MASK_MOVPOS

struct req1
{
	ushort_td	op;
	void_td*	keybuf;
	uchar_td	keylen;
	short_td	keyNum;
	void write(std::ostream& ost);
};

#pragma option -a-
pragma_pack1


#define POSBLK_SIZE 4
#define CLIENTID_SIZE 2

/** tdap version info struct
 */
struct version
{
    ushort_td majorVersion;
    ushort_td minorVersion;
    uchar_td Type;           
};

struct posblk
{
	posblk()
	{
		memset(this, 0, sizeof(posblk));
	}
	int handle;

};
struct clientID
{

#ifdef __x86_64__
	bzs::netsvc::client::connection* con;
	char_td reserved[4];	
#else
	bzs::netsvc::client::connection* con;
	char_td reserved[8];
#endif
	char_td		aid[2];
	ushort_td 	id;
};


#pragma option -a
pragma_pop


class request
{
private:
	ushort_td		varLenBytesPos;	/*Ooffset of last var field*/
	ushort_td		varLenBytes;	/*Bytes of last var field length */

public:
	ushort_td		paramMask;
	union
	{
		short_td    result;
		short_td	op;
	};

	posblk*			pbk;
	uint_td*		datalen;
	void_td*		data;
	keylen_td 		keylen;
	void_td*		keybuf;
	char_td			keyNum;
	uint_td			resultLen;		
	
	request()
	{
    	memset(this, 0, sizeof(request));
	}
	
	virtual ~request(){}

	void reset()
	{
		paramMask = 0;
		result = 0;
		varLenBytesPos = 0;
		varLenBytes = 0;
		resultLen = 0;
	}

	const struct bzs::db::blobHeader* blobHeader;

	inline unsigned int serializeBlobBody(blobBuffer* blob, char* buf, unsigned int max_size
				, std::vector<boost::asio::const_buffer>* mbuffer)
	{
		unsigned int totallen = *((unsigned int*)(buf));
		unsigned int size = totallen;
		char* blobbuf = buf + totallen;
		//write blob body
		int stat=0;
		if (mbuffer)
			totallen += blob->makeMultiBuffer(*mbuffer);
		else
			totallen += blob->writeBuffer((unsigned char*)blobbuf, max_size - totallen - 200, stat);
		//write total
		memcpy(buf, &totallen, sizeof(unsigned int));

		//write result
		if (stat)
		{
			short_td* retPtr = (short_td*)(buf + 4 + 1);
			*retPtr = stat;
		}
		//return this buffer size;
		if (mbuffer)
			return size;
		return totallen;
	}
};


}//namespace tdap
}//namespace protocol
}//namespace db
}//namespace bzs


#endif //BZS_DB_PROTOCOL_TDAP_TDAPREQUSET_H
