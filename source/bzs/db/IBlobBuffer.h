#ifndef BZS_DB_IBLOBBUFFER_H
#define BZS_DB_IBLOBBUFFER_H
/*=================================================================
   Copyright (C) 2012 2013 BizStation Corp All rights reserved.

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
//#include <boost/asio/buffer.hpp>
#include <vector>
#include "blobStructs.h"

namespace boost{namespace asio{class const_buffer;}}
namespace bzs
{
namespace db
{

class IblobBuffer
{

public:
    virtual ~IblobBuffer(){};
    virtual void clear() = 0;
    virtual void addBlob(unsigned int bloblen, unsigned short fieldNum,
                         const unsigned char* dataPtr) = 0;
    virtual void setFieldCount(unsigned int v) = 0;
    virtual unsigned int fieldCount() = 0;
    virtual unsigned int
    makeMultiBuffer(std::vector<boost::asio::const_buffer>& mbuffer) = 0;
    virtual unsigned int writeBuffer(unsigned char* buffer,
                                     unsigned int maxsize, short& stat) = 0;
};

} // namespace db
} // namespace bzs

#endif // BZS_DB_IBLOBBUFFER_H
