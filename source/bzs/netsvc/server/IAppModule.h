#ifndef BZS_NETSVC_IAPPMODULE_H
#define BZS_NETSVC_IAPPMODULE_H
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

#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>

/** Application framework interface 
 *	IModuleBuilder: module builder. 
 *	IAppModule :The module created for every session.
 *              This receives received data and returns a result.
 *  It will be done, if these interfaces are implement and a server's constructor is passed.
 */

namespace bzs
{
namespace netsvc
{
namespace server
{

class iconnection;

#define EXECUTE_RESULT_SUCCESS 0
#define EXECUTE_RESULT_QUIT 1
#define EXECUTE_RESULT_FORCSE_ASYNC 2
#define EXECUTE_RESULT_FORCSE_SYNC  3

typedef std::vector<boost::asio::const_buffer> buffers;

class IAppModule
{
public:
		
	virtual ~IAppModule() {};
	virtual int execute(char* result, size_t& size, buffers* optionalData) = 0;
	virtual size_t onRead(const char* data, size_t size, bool& complete)=0;
	virtual size_t onAccept(char* message, size_t bufsize)=0;
	virtual void reset() = 0;
	virtual bool isShutDown() = 0;
	virtual bool checkHost(const char* hostCheckname) = 0;
	virtual void cleanup() = 0;
	virtual boost::mutex& mutex()const = 0;

		
};

//Defines at Implementing of IAppModule , two variables below.
extern boost::mutex modulesMutex;
extern std::vector<IAppModule*> modules;
	


#define SERVER_TYPE_TPOOL	1
#define SERVER_TYPE_CPT		0

/** 
 * used by worker thread
 * use with dynamic_cast for your application
 */
class IAppModuleBuilder
{
public:
	virtual ~IAppModuleBuilder(){};
	virtual IAppModule* createSessionModule(const boost::asio::ip::tcp::endpoint& endpoint
											, iconnection* connection
											, bool tpool) = 0;
};



}//namespace server
}//namespace netsvc
}//namespace bzs
#endif//BZS_NETSVC_IAPPMODULE_H
