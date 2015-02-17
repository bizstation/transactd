#ifndef BZS_DB_PROTOCOL_TDAP_TDAPREQUSET_H
#define BZS_DB_PROTOCOL_TDAP_TDAPREQUSET_H
/* =================================================================
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
 ================================================================= */

#include <bzs/db/protocol/tdap/tdapcapi.h>
#include <bzs/db/blobBuffer.h>
#include <iostream>
#include <vector>
#include <string.h>
#include <boost/asio/buffer.hpp>
#include <bzs/env/compiler.h>

namespace bzs
{
namespace netsvc
{
namespace client
{
class connection;
}
}

namespace db
{
namespace protocol
{
namespace tdap
{
namespace client {class table;}

#define P_MASK_OP 0
#define P_MASK_POSBLK 1
#define P_MASK_DATA 2
#define P_MASK_DATALEN 4
#define P_MASK_KEYBUF 8
#define P_MASK_KEYNUM 16

/** The data length which transmits to a client 
 *  is described at 2 bytes of the data buffer head.
 */
#define P_MASK_EX_SENDLEN 32    
#define P_MASK_BLOBBODY 64
#define P_MASK_FINALRET 128     // server sent final result
#define P_MASK_FINALDATALEN 256 // server sent final result
#define P_MASK_PB_BOOKMARK 512  // posblk bookmark writen
#define P_MASK_PB_ERASE_BM 1024 // posblk bookmark erase command
#define P_MASK_PB_LOCKED   2048 // posblk row locked


#define P_MASK_ALL                                                             \
    P_MASK_POSBLK | P_MASK_DATA | P_MASK_DATALEN | P_MASK_KEYBUF | P_MASK_KEYNUM
#define P_MASK_KEYONLY P_MASK_KEYBUF | P_MASK_KEYNUM
#define P_MASK_NOKEYBUF                                                        \
    P_MASK_POSBLK | P_MASK_DATA | P_MASK_DATALEN | P_MASK_KEYNUM
#define P_MASK_POS_LEN_KEY P_MASK_POSBLK | P_MASK_DATALEN | P_MASK_KEYNUM
#define P_MASK_KEYNAVI                                                         \
    P_MASK_POSBLK | P_MASK_DATALEN | P_MASK_KEYBUF | P_MASK_KEYNUM
#define P_MASK_NODATA P_MASK_POSBLK | P_MASK_DATALEN | P_MASK_KEYBUF

// server side
#define P_MASK_READRESULT P_MASK_POSBLK | P_MASK_DATA | P_MASK_DATALEN
#define P_MASK_STAT P_MASK_DATA | P_MASK_DATALEN
#define P_MASK_READ_EXT P_MASK_STAT | P_MASK_KEYBUF
#define P_MASK_MOVPOS                                                          \
    P_MASK_POSBLK | P_MASK_DATA | P_MASK_DATALEN | P_MASK_KEYBUF
#define P_MASK_INS_AUTOINC P_MASK_MOVPOS

#pragma pack(push, 1)
pragma_pack1;


#define CLIENTID_SIZE                   2
#define PARAMMASK_SIZE                  2
#define TD_POSBLK_TRANSMIT_SIZE         4


/** tdap bosblk
*/

/* Dynamic data buffer alloc for client */
typedef void*(__STDCALL* DDBA_PTR)(client::table* tb, uint_td size);

/** tdap version info struct
 */

struct version
{
    ushort_td majorVersion;
    ushort_td minorVersion;
    uchar_td Type;
};

#pragma pack(push, 1)
pragma_pack1;

struct posblk
{
    posblk() { memset(this, 0, sizeof(posblk)); }

    unsigned int handle;
    client::table* tb;
    DDBA_PTR allocFunc;
    unsigned char lock;
    unsigned char bookmarkLen;
    char bookmark[1]; //dummy array
};
#pragma pack(pop)
pragma_pop;

struct clientID
{

#ifdef __x86_64__
    bzs::netsvc::client::connection* con;
    unsigned short transaction_isolation;
    unsigned short lock_wait_timeout;
#else // NOT __x86_64__

    bzs::netsvc::client::connection* con;
    unsigned short transaction_isolation;
    unsigned short lock_wait_timeout;
    char_td reserved[4];
#endif // NOT __x86_64__
    char_td aid[2];
    ushort_td id;
};

#pragma pack(pop)
pragma_pop;

class request
{
private:
    ushort_td varLenBytesPos; /* Ooffset of last var field */
    ushort_td varLenBytes; /* Bytes of last var field length */

public:
    ushort_td paramMask;

    union
    {
        short_td result;
        short_td op;
    };

    posblk* pbk;
    uint_td* datalen;
    void_td* data;
    keylen_td keylen;
    void_td* keybuf;
    char_td keyNum;
    uint_td resultLen;
    short_td* resltPtr;

    request()
        : paramMask(0), op(0), pbk(0), datalen(0), data(0), keylen(0),
          keybuf(0), keyNum(0), resultLen(0), resltPtr(0)
    {
    }

    virtual ~request() {}

    virtual void clear()
    {
        pbk = 0;
        datalen = 0;
        data = 0;
        keylen = 0;
        keybuf = 0;
        keyNum = 0;
        resltPtr = 0;
        reset();
    }

    void reset()
    {
        paramMask = 0;
        result = 0;
        varLenBytesPos = 0;
        varLenBytes = 0;
        resultLen = 0;
    }

    const struct bzs::db::blobHeader* blobHeader;

    inline unsigned int serializeBlobBody(
        blobBuffer* blob, char* buf, size_t curSize, unsigned int max_size,
        std::vector<boost::asio::const_buffer>* mbuffer, short& stat)
    {

        char* blobbuf = buf + curSize;
        unsigned int datalen = *((unsigned int*)buf);
        // write blob body
        unsigned int bloblen = 0;
        if (mbuffer)
            bloblen = blob->makeMultiBuffer(*mbuffer);
        else
            bloblen = blob->writeBuffer((unsigned char*)blobbuf,
                                        max_size - datalen - 200, stat);

        // write total

        datalen += bloblen;
        memcpy(buf, &datalen, sizeof(unsigned int));

        // write result
        if (stat && resltPtr)
            *resltPtr = stat;
        // return this buffer size;
        if (mbuffer)
            return (unsigned int)curSize;
        return (unsigned int)curSize + bloblen;
    }
};

} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs

#endif // BZS_DB_PROTOCOL_TDAP_TDAPREQUSET_H
