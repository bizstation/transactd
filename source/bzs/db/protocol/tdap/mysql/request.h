#ifndef BZS_DB_PROTOCOL_TDAP_MYSQL_REQUSET_H
#define BZS_DB_PROTOCOL_TDAP_MYSQL_REQUSET_H
/*=================================================================
   Copyright (C) 2013 2014 BizStation Corp All rights reserved.

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
#include <bzs/netsvc/server/IAppModule.h>

/** readRecords reserved buffer size
 */
#define RETBUF_EXT_RESERVE_SIZE 12

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
#ifdef DEBUG_LOG_ERR
    const char* m_readBuffer;
#endif
    void clear()
    {
        bzs::db::protocol::tdap::request::clear();
        cid = 0;
    }

    request() : bzs::db::protocol::tdap::request(), cid(0) {}

    inline unsigned int serializeForExt(engine::mysql::table* tb,
                                        netsvc::server::netWriter* nw)
    {
        // The result contents is copied or sent allready.

        paramMask = nw->getParamMask(tb->getBlobFieldCount() != 0);
        unsigned int allreadysent = nw->allreadySent();
        nw->writeHeadar(paramMask, result);

        if (paramMask & P_MASK_KEYBUF)
        {
            keylen = tb->keyPackCopy((uchar*)nw->curPtr() + sizeof(keylen_td));
            nw->write((const char*)&keylen, sizeof(keylen_td));
            nw->write(NULL, keylen, netsvc::server::netWriter::curSeekOnly);
        }
        // write final ret
        if (paramMask & P_MASK_FINALRET)
        {
            nw->write((const char*)&result, sizeof(short_td));
            resltPtr = (short_td*)(nw->curPtr() - 2);
        }
        unsigned int* totalLen = (unsigned int*)nw->ptr();
        *totalLen = nw->resultLen();

        return *totalLen - allreadysent;
    }

    inline unsigned int serialize(engine::mysql::table* tb, char* buf)
    {
        char* p = buf;
        char* datalenPtr = NULL;

        p += sizeof(unsigned int); // space of totalLen

        //add posblk bookmark
        if ((P_MASK_POSBLK & paramMask) && tb->cursor())
        {
            paramMask |= P_MASK_PB_BOOKMARK;
            if (tb->isDelayAutoCommit())
                paramMask |= P_MASK_PB_LOCKED;
        }
        memcpy(p, (const char*)(&paramMask), sizeof(ushort_td));
        p += sizeof(ushort_td);

        memcpy(p, (const char*)(&result), sizeof(short_td));
        resltPtr = (short_td*)p;
        p += sizeof(short_td);

        if (P_MASK_POSBLK & paramMask)
        {
            memcpy(p, (const char*)pbk, TD_POSBLK_TRANSMIT_SIZE);
            p += TD_POSBLK_TRANSMIT_SIZE;
            if (P_MASK_PB_BOOKMARK & paramMask)
            {
                uint v = tb->posPtrLenRaw();
                memcpy(p++, &v, 1);
                memcpy(p, tb->position(true), v);
                p += v;
            }
        }

        if (P_MASK_DATALEN & paramMask)
        {
            datalenPtr = p;
            p += sizeof(uint_td);
        }

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
        clear();
#ifdef DEBUG_LOG_ERR
        m_readBuffer = p;
#endif
        p += sizeof(unsigned int);
        paramMask = *((ushort_td*)p);
        p += sizeof(ushort_td);
        op = *((ushort_td*)p);
        p += sizeof(ushort_td);

        if (P_MASK_POSBLK & paramMask)
        {
            pbk = (posblk*)p;
            p += TD_POSBLK_TRANSMIT_SIZE;
        }
        if (P_MASK_DATALEN & paramMask)
        {
            datalen = (uint_td*)p;
            p += sizeof(uint_td);
        }

        if (P_MASK_EX_SENDLEN & paramMask)
        {
            data = (void_td*)p;
            int v = *((int*)p);
            v &= 0xFFFFFFF; // 28bit
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
            p += keylen + 1; // null terminate for server
        }
        if (P_MASK_KEYNUM & paramMask)
        {
            keyNum = *((char_td*)p);
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

} // namespace mysql
} // namespace protocol
} // namespace db
} // namespace tdap
} // namespace bzs

#endif // BZS_DB_PROTOCOL_TDAP_MYSQL_REQUSET_H
