#ifndef BZS_DB_PROTOCOL_ICOMMANDEXECUTER_H
#define BZS_DB_PROTOCOL_ICOMMANDEXECUTER_H
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
#include <bzs/netsvc/server/IAppModule.h>

namespace bzs
{
namespace db
{
namespace protocol
{
 
class ICommandExecuter
{
		
public:
	virtual ~ICommandExecuter(){};
	virtual size_t perseRequestEnd(const char* p, size_t size, bool& comp)const=0;
	virtual size_t getAcceptMessage(char* message, size_t bufsize)=0;
	virtual bool parse(const char* p, size_t size)=0;
	//virtual int execute(netsvc::server::IResultBuffer& resultBuffer, size_t& size, netsvc::server::buffers* optionalData) = 0;
	virtual int execute(netsvc::server::netWriter* nw) = 0;
	virtual bool isShutDown() = 0;
	virtual void cleanup() = 0;
};




}//namespace protocol
}//namespace db
}//namespace bzs

#endif //BZS_DB_PROTOCOL_ICOMMANDEXECUTER_H

