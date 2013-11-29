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

#include "tcpClient.h"
#include <bzs/env/crosscompile.h>
#ifndef _WIN32
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem.hpp>
#endif

using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;

// ---------------------------------------------------------------------------
#ifdef __BCPLUSPLUS__
#pragma package(smart_init)
#endif



namespace bzs
{
namespace netsvc
{
namespace client
{

char connections::port[PORTNUMBUF_SIZE] = {"8610"};
bool connections::m_usePipedLocal = true;

connections::connections(const char* pipeName):m_pipeName(pipeName)
{
#ifdef _WIN32
	DWORD len = MAX_PATH;
	char buf[MAX_PATH];
	if (GetWindowsDirectory(buf, len))
	{
		strcat_s(buf, MAX_PATH, "\\transactd.ini");
		char tmp[30];
		GetPrivateProfileString("transctd_client","use_piped_local", "1"
				, tmp, 30, buf);
		m_usePipedLocal = (atol(tmp)!=0);
		GetPrivateProfileString("transctd_client","port", "8610"
				, tmp, 30, buf);
		strcpy_s(port, PORTNUMBUF_SIZE, tmp);

	}
#else
	namespace fs = boost::filesystem;
	const fs::path path("/etc/transactd.cnf");
    boost::system::error_code error;
    const bool result = fs::exists(path, error);
    if (result && !error) 
	{
		boost::property_tree::ptree pt;
		try
		{
			boost::property_tree::ini_parser::read_ini("/etc/transactd.cnf", pt);
			std::string p = pt.get<std::string>("transctd_client.port");
			if (p == "")
				p = "8610";
			strcpy_s(port, PORTNUMBUF_SIZE, p.c_str());
		}
		catch(...){}
	}
#endif

}

/**  Use by procces detach
 *
 */
connections::~connections()
{
	for (int i=0;i<(int)m_conns.size();i++)
	{
		if (m_conns[i])
			delete m_conns[i];	
			
	}
}

/**	When threads differ, it have to make a different connection.
 */
connection* connections::getConnection(asio::ip::tcp::endpoint& ep)
{
	thread_id tid = threadid();
	for (int i=0;i<(int)m_conns.size();i++)
	{
		if ((m_conns[i]->endpoint()==ep) && (m_conns[i]->tid()==tid))
			return m_conns[i];
	}
	return NULL;
}

asio::ip::tcp::endpoint connections::endpoint(const std::string& host)
{
	asio::ip::tcp::resolver resolver(m_ios);
	asio::ip::tcp::resolver::query query(host, port);

	boost::asio::ip::tcp::resolver::iterator dest = resolver.resolve(query);
	asio::ip::tcp::endpoint endpoint;
	while (dest != boost::asio::ip::tcp::resolver::iterator())
		endpoint = *dest++;
	return endpoint;
}

connection* connections::getConnection(const std::string& host)
{
	mutex::scoped_lock lck(m_mutex);
	asio::ip::tcp::endpoint ep = endpoint(host);
	return getConnection(ep);
}

#ifdef USE_PIPE_CLIENT
connection* connections::getConnectionPipe()
{

	thread_id tid = threadid();
	for (int i=0;i<(int)m_conns.size();i++)
	{
		pipeConnection* pc = dynamic_cast<pipeConnection*>(m_conns[i]);
		if (pc && (pc->tid()==tid))
			return pc;
	}
	return NULL;
}
#endif

bool connections::disconnect(connection* c)
{
	//if (!c) return false;
	mutex::scoped_lock lck(m_mutex);
	c->release();
	if (c->refCount()==0)
	{
		for (int i = (int)m_conns.size()-1;i>=0;--i)
		{
			if (c == m_conns[i])
			{
				m_conns.erase(m_conns.begin()+i);
				break;
			}
		}
		delete c;
		return true;	
	}
	return false;
}

/** If ep is local then isUseNamedPipe is true 
 */
bool connections::isUseNamedPipe(asio::ip::tcp::endpoint& ep)
{
	asio::ip::tcp::endpoint local = endpoint("127.0.0.1");
	if (local == ep)
		return true;
	char buf[MAX_PATH];
	if (::gethostname(buf, MAX_PATH)==0)
	{
		local = endpoint(buf);
		if (local == ep)
			return true;
	}
	return false;
}

//The connection of found from connection list of same address is returned.
connection* connections::connect(const std::string& host, bool newConnection)
{
	mutex::scoped_lock lck(m_mutex);
	connection* c;
	asio::ip::tcp::endpoint ep = endpoint(host);
#ifdef USE_PIPE_CLIENT
	if (m_usePipedLocal && isUseNamedPipe(ep))
	{
		c = newConnection ? NULL: getConnectionPipe();
		if(!c)
		{
			c = new pipeConnection(ep, m_pipeName);
			c->connect();
			m_conns.push_back(c);
		}
	}else
#endif
	{

		c = newConnection ? NULL: getConnection(ep);
		if(!c)
		{
			c = new tcpConnection(ep);
			c->connect();
			m_conns.push_back(c);
		}
	}
	c->addref();
	return c;
}

int connections::connectionCount()
{

	return (int)m_conns.size();
};

}//namespace client
}//namespace netsvc
}//namespace bzs

