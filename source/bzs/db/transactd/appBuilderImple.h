#ifndef BZS_DB_TRANSACTD_APPBUILDERIMPLE_H
#define BZS_DB_TRANSACTD_APPBUILDERIMPLE_H
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

#define PROTOCOL_TYPE_BTRV				1
#define PROTOCOL_TYPE_HS				2
#define PROTOCOL_TYPE_ASYNCWRITE		4
#define PROTOCOL_TYPE_MEMBUFFER			8


/** IAppModule factry
 *	Implements IAppModule and Implements IMyPluginModule::create too.
 */
class IMyPluginModule : public bzs::netsvc::server::IAppModule
{
public:
	static bzs::netsvc::server::IAppModule* create(
				const boost::asio::ip::tcp::endpoint& endpoint
				, bzs::netsvc::server::iconnection* connection
				, bool tpool, int type);
};

class transctionalIF : public bzs::netsvc::server::IAppModuleBuilder
{
	bzs::netsvc::server::IAppModule* createSessionModule(
								const boost::asio::ip::tcp::endpoint& endpoint
									, bzs::netsvc::server::iconnection* connection
									, bool tpool)
	{
		return IMyPluginModule::create(endpoint, connection, tpool, m_type);
	}

	int m_type;
public:

	transctionalIF(int type):m_type(type){};
};

#endif //BZS_DB_TRANSACTD_APPLICATIONIMPLE_H
