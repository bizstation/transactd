#ifndef BZS_NETSVC_SERVER_ISERVER_H
#define BZS_NETSVC_SERVER_ISERVER_H
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
namespace bzs
{
namespace netsvc
{
namespace server
{

/** Super class have to implement multithread safety.
 */
class inotifyHandler
{

public:
    virtual ~inotifyHandler(){};
    virtual void printError(const char* msg) = 0;
    virtual void printInfo(const char* msg) = 0;
};

class iserver
{
public:
    virtual ~iserver(){};
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void registerErrorHandler(inotifyHandler*) = 0;
};

/** connection close method for application
 */
class iconnection
{

public:
    virtual ~iconnection(){};
    virtual void close() = 0;
    virtual void asyncWrite(const char* p, unsigned int) = 0;
};

} // namespace sever
} // namespace netsvc
} // namespace bzs

#endif // BZS_NETSVC_SERVER_ISERVER_H
