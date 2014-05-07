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

#include "appModule.h"
#include "appBuilderImple.h"
#include <bzs/db/engine/mysql/dbManager.h>
#include <bzs/db/engine/mysql/mysqlThd.h>
#include <bzs/db/protocol/tdap/mysql/tdapCommandExecuter.h>
#include <bzs/netsvc/server/iserver.h>
#include <stdlib.h>
#include <boost/asio/ip/address_v4.hpp>
#ifdef USE_HANDLERSOCKET
#include <bzs/db/protocol/hs/hsCommandExecuter.h>
#endif



using namespace bzs::netsvc::server;

IAppModule* IMyPluginModule::create(const boost::asio::ip::tcp::endpoint& endpoint
									, iconnection* connection		
									, bool tpool, int type)
{
	return new bzs::db::transactd::module(endpoint, connection, tpool, type);
}

namespace bzs
{
namespace netsvc
{
namespace server
{

	boost::mutex modulesMutex;
	std::vector<IAppModule*> modules;
	using namespace bzs::netsvc::server;

}//namespace server
}//namespace netsvc

namespace db
{
	using namespace protocol;
namespace transactd
{

/** The module created for every connection
 *  In the case of a thread pool, thread termination processing is not performed by a destructor.
 */
module::module(const boost::asio::ip::tcp::endpoint& endpoint
			, iconnection* connection, bool tpool, int type)
	:m_endpoint(endpoint), m_connection(connection), m_useThreadPool(tpool)
{
	if (type == PROTOCOL_TYPE_BTRV)
		m_commandExecuter.reset(new protocol::tdap::mysql::commandExecuter((unsigned __int64)this));
#ifdef USE_HANDLERSOCKET
	else if(type == PROTOCOL_TYPE_HS)
		m_commandExecuter.reset(new protocol::hs::commandExecuter((unsigned __int64)this));
#endif
	boost::mutex::scoped_lock lck(modulesMutex);
	modules.push_back(this);
}

module::~module(void)
{
	boost::mutex::scoped_lock lck(modulesMutex);
	modules.erase( find(modules.begin(), modules.end(), this));
	m_commandExecuter.reset();
	if (m_useThreadPool==false)
	{
		my_thread_end();
		endThread();
	}
}

void module::reset()
{
}

/**  It is called from the handler of async_read. 
 *   A value is analyzed and it is answered whether a lead is completion.
 *	 @return Size required for a lead buffer is returned on the occasion of reading by addition.
 *	         When zero are returned, it is shown that it is not necessary to enlarge a buffer further.
 */
size_t module::onRead(const char* data, size_t size, bool& complete)
{
	m_readBuf = data;
	m_readSize = size;
	return m_commandExecuter->perseRequestEnd(data, size, complete);
}

size_t module::onAccept(char* message, size_t bufsize)
{
	return m_commandExecuter->getAcceptMessage(message, bufsize);
}

static const char* addressMasks[3] = {".0.0.0/255.0.0.0", ".0.0/255.255.0.0", ".0/255.255.255.0" };

char* addressClass(char* buf,int bufsize, const char *host, int type)
{
	strcpy_s(buf, bufsize, host);
	for (int i = 3; i >= type; i--)
	{
		char* p = strrchr(buf, '.');
		if (p == NULL) return buf;
		*p = 0x00;
	}
	strcat_s(buf, bufsize, addressMasks[type-1]);
	return buf;
}

bool isAclUser(const char *host, const char *user)
{
	bool ret = is_acl_user(host, user);
	if (ret)return true;
	for (int i = 1; i <= 3; i++)
	{
		char buf[256];
		ret = is_acl_user( addressClass(buf, 256, host, i), user);
		if (ret)return true;
	}
	return false;
	
}

bool module::checkHost( const char* hostCheckname)
{
	std::string addr = m_endpoint.address().to_string();
	size_t pos = addr.find_last_of(":");
	if (pos != std::string::npos)
		addr = addr.substr(pos + 1);
	
	bool ret = true;
	if (!isAclUser(addr.c_str(), hostCheckname))
	{
		ret = isAclUser(m_endpoint.address().to_string().c_str(), hostCheckname);
		if (!ret && m_endpoint.address().is_v4())
		{
			if (addr ==  std::string("127.0.0.1"))
				ret = isAclUser("localhost", hostCheckname);
		}
	}
	return ret;
}

//int module::execute(char* result, size_t& size, netsvc::server::buffers* optionalData)
int module::execute(netsvc::server::IResultBuffer& result, size_t& size, netsvc::server::buffers* optionalData)
{
	m_commandExecuter->parse(m_readBuf, m_readSize);
	boost::mutex::scoped_lock lck(m_mutex);
	if (m_useThreadPool)
	{
		int ret = m_commandExecuter->execute(result, size, optionalData);
		cleanup();
		return ret;
	}
	return m_commandExecuter->execute(result, size, optionalData);
}

void  module::disconnect()
{
	m_connection->close();
}


}//namespace transactd
}//namespace db
}//namespace bzs
