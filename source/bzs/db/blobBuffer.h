#ifndef BZS_DB_BLOBBUFFER_H
#define BZS_DB_BLOBBUFFER_H
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
#include <boost/noncopyable.hpp>
#include "IBlobBuffer.h"
#include <assert.h>
#include <boost/shared_array.hpp>
#include <boost/asio/buffer.hpp>

extern unsigned int g_pipeCommSharememSize;
#define FILE_MAP_SIZE (g_pipeCommSharememSize - 1024)

namespace bzs
{
namespace db
{

static const char nullbyte[2] = { 0x00 };

/** holds blob ptr and make boost multiplebuffer
 */
class blobBuffer : public IblobBuffer, private boost::noncopyable
{
    std::vector<blob> m_blobs;
    blobHeader m_bh;
    std::vector<unsigned char> m_data;
    unsigned int m_datasize;
    unsigned int m_blobCount;

public:
    blobBuffer() { m_bh.fieldCount = 0; }
    const blobHeader* getHeader() const { return &m_bh; };

    void clear()
    {
        m_blobs.clear();
        m_bh.fieldCount = 0;
        m_bh.rows = 0;
        m_bh.dataSize = 0;
        m_data.clear();
        m_datasize = 0;
        m_blobCount = 0;
    }

    size_t blobs() { return m_blobCount; };
    /**
     *   @param dataPtr blob data body pointer in result record image.
     */
    void addBlob(unsigned int bloblen, unsigned short fieldNum,
                 const unsigned char* dataPtr)
    {
        if (m_data.size() < m_datasize + bloblen + sizeof(blobField) + 2)
            m_data.resize(m_datasize + bloblen + 1024);

        unsigned char* p = &m_data[m_datasize];

        blob b(bloblen, fieldNum, p + sizeof(blobField));
        // m_blobs.push_back(b);

        // write field desc
        memcpy(p, &b.bf, sizeof(blobField));
        p += sizeof(blobField);

        // write field body
        memcpy(p, dataPtr, b.bf.size);

        // write null terminate
        p += b.bf.size;
        memcpy(p, nullbyte, 2);
        p += 2;
        m_datasize = (unsigned int)(p - &m_data[0]);
        ++m_blobCount;
        m_bh.dataSize += b.bf.size;

    }

    //For client
    void addBlob(const blob& b)
    {
        m_blobs.push_back(b);
        m_bh.dataSize += b.bf.size;
        ++m_blobCount;
    }

    void setFieldCount(unsigned int v) { m_bh.fieldCount = v; };
    unsigned int fieldCount() { return m_bh.fieldCount; }

    /** Make boost::asio::multibuffer<const_buffer> from added blob.
     *  and write offset to record image of blob fields.
     *  @image pointer of normal result image.
     *  @size bytes of normal result image length.
     *  @return
     *  result buffer image
     *  ---------------------------
     *  n               record image
     *  2       fieldNum
     *  4       dataLength
     *  n(dataLength) data
     *  2       null terminate
     *  2       fieldNum ...
     *  ----------------------------
     */

    unsigned int
    makeMultiBuffer(std::vector<boost::asio::const_buffer>& mbuffer)
    {
        unsigned int size = 0;
        if (m_bh.fieldCount)
        {
            // write buffer header
            m_bh.rows = (unsigned short)(m_blobCount / m_bh.fieldCount);
            mbuffer.push_back(
                boost::asio::buffer((char*)&m_bh, sizeof(blobHeader)));
            size += sizeof(blobHeader);
            if (m_blobs.size() == 0)
            {
                mbuffer.push_back(
                    boost::asio::buffer((char*)&m_data[0], m_datasize));
                size += m_datasize;
            }
            else
            {
                for (size_t i = 0; i < m_blobs.size(); i++)
                {
                    const blob& b = m_blobs[i];
                    // add field desc
                    mbuffer.push_back(
                        boost::asio::buffer((char*)&b.bf, sizeof(blobField)));
                    size += sizeof(blobField);

                    // add field body
                    mbuffer.push_back(boost::asio::buffer(b.ptr, b.bf.size));

                    // add null terminate
                    mbuffer.push_back(boost::asio::buffer(nullbyte, 2));

                    // update record image pointer to offset from record start
                    size += b.bf.size + 2;
                }
            }
        }
        return size;
    }

    /** Copy to single buffer.
     *  this is use for pipe server only.
     *  @param stat [out] return additional status. ex. STATUS_BUFFERTOOSMALL
     *  @return length of written by this function.
     *  result buffer image
     *  ---------------------------
     *  n               record image
     *  2       fieldNum
     *  4       dataLength
     *  n(dataLength) data
     *  2       null terminate
     *  2       fieldNum ...
     *  ----------------------------
     */
    unsigned int writeBuffer(unsigned char* buffer, unsigned int maxsize,
                             short& stat)
    {
        stat = 0;
        unsigned char* p = buffer;
        if (m_bh.fieldCount)
        {
            // write buffer header
            m_bh.rows = (unsigned short)(m_blobCount / m_bh.fieldCount);
            memcpy(p, &m_bh, sizeof(blobHeader));
            p += sizeof(blobHeader);
            if (m_blobs.size() == 0)
            {
                if (maxsize - (p - buffer) < m_datasize)
                    stat = STATUS_BUFFERTOOSMALL;
                else
                {
                    memcpy(p, &m_data[0], m_datasize);
                    p += m_datasize;
                }
            }
            else
            {
                for (size_t i = 0; i < m_blobs.size(); i++)
                {
                    const blob& b = m_blobs[i];
                    if (maxsize > (p - buffer) + b.bf.size + 2)
                    {
                        // write field desc
                        memcpy(p, &b.bf, sizeof(blobField));
                        p += sizeof(blobField);
                        // write field body
                        memcpy(p, b.ptr, b.bf.size);

                        // write null terminate
                        p += b.bf.size;
                        memcpy(p, nullbyte, 2);
                        p += 2;
                    }
                    else
                    {
                        stat = STATUS_BUFFERTOOSMALL;
                        break;
                    }
                }
            }
        }
        return (unsigned int)(p - buffer);
    }
};

} // namespace db
} // namespace bzs
#endif // BZS_DB_BLOBBUFFER_H
