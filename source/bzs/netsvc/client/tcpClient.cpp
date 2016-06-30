/* =================================================================
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
 ================================================================= */
#include "tcpClient.h"
#include <bzs/env/crosscompile.h>
#if (!defined(_WIN32))
#include <boost/filesystem.hpp>
#if (BOOST_VERSION > 104900)
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#endif
#endif

#pragma package(smart_init)

using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;

namespace bzs
{
namespace netsvc
{
namespace client
{

char connections::m_port[PORTNUMBUF_SIZE] = { "8610" };
bool connections::m_usePipedLocal = true;
#ifdef _WIN32
int connections::connectTimeout = 20000;
int connections::netTimeout = 180000;
#else
int connections::connectTimeout = 20;
int connections::netTimeout = 180;
#endif

#define DEFAULT_CONNECT_TIMEOUT "20"
#define DEFAULT_NET_TIMEOUT "180"

/* redefine tdapapi.h */
#define HST_OPTION_CLEAR_CACHE 8

connections::connections(const char* pipeName) : m_pipeName(pipeName),
    m_resolver(m_ios), m_haNameResolver(NULL)
{

#ifdef _WIN32
    DWORD len = MAX_PATH;
    char buf[MAX_PATH];
    if (GetWindowsDirectory(buf, len))
    {
        strcat_s(buf, MAX_PATH, "\\transactd.ini");
        char tmp[30];
        GetPrivateProfileString("transctd_client", "use_piped_local", "1", tmp,
                                30, buf);
        m_usePipedLocal = (atol(tmp) != 0);
        GetPrivateProfileString("transctd_client", "port", "8610", tmp, 30,
                                buf);
        strcpy_s(m_port, PORTNUMBUF_SIZE, tmp);
        GetPrivateProfileString("transctd_client", "connectTimeout", 
            DEFAULT_CONNECT_TIMEOUT, tmp, 30, buf);
        connectTimeout = atoi(tmp)*1000;
        GetPrivateProfileString("transctd_client", "netTimeout", 
            DEFAULT_NET_TIMEOUT, tmp, 30, buf);
        netTimeout = atoi(tmp)*1000;
    }
#else // NOT _WIN32
#if (BOOST_VERSION > 104900)

    namespace fs = boost::filesystem;
    const fs::path path("/etc/transactd.cnf");
    boost::system::error_code error;
    const bool result = fs::exists(path, error);
    if (result && !error)
    {
        boost::property_tree::ptree pt;
        try
        {
            boost::property_tree::ini_parser::read_ini("/etc/transactd.cnf",
                                                       pt);
            std::string p = pt.get<std::string>("transctd_client.port");
            if (p == "")
                p = "8610";
            strcpy_s(m_port, PORTNUMBUF_SIZE, p.c_str());

            p = pt.get<std::string>("transctd_client.connectTimeout");
            connectTimeout = atol(p.c_str());
            if (connectTimeout == 0)
                connectTimeout = atoi(DEFAULT_CONNECT_TIMEOUT);
            p = pt.get<std::string>("transctd_client.netTimeout");
            netTimeout = atol(p.c_str());
            if (netTimeout == 0)
                netTimeout = atoi(DEFAULT_NET_TIMEOUT);
        }
        catch (...)
        {
        }
    }
#endif // BOOST_VERSION > 104900
#endif // NOT _WIN32
}

/** Use by procces detach
 *
 */
connections::~connections()
{
    for (int i = 0; i < (int)m_conns.size(); i++)
    {
        if (m_conns[i])
            delete m_conns[i];
    }
}

/** When threads differ, it have to make a different connection.
 */
connection* connections::getConnection(asio::ip::tcp::endpoint& ep)
{
    for (int i = 0; i < (int)m_conns.size(); i++)
    {
        if (m_conns[i]->endpoint() == ep)
            return m_conns[i];
    }
    return NULL;
}

asio::ip::tcp::endpoint connections::endpoint(const std::string& host,
                                              const char* port,
                                              boost::system::error_code& ec)
{

    if (!port || port[0] == 0x00) port = m_port;
    tcp::resolver::query query(host, port);

    tcp::resolver::iterator dest = m_resolver.resolve(query, ec);
    tcp::endpoint endpoint;
    if (!ec)
    {
        while (dest != tcp::resolver::iterator())
            endpoint = *dest++;
    }else if (host == "localhost") 
    {
        endpoint.address(ip::address::from_string("127.0.0.1"));
        endpoint.port((unsigned short)atoi(port));
        ec.clear();
    }
    return endpoint;
}

#ifdef USE_PIPE_CLIENT

connection* connections::getConnectionPipe(unsigned short port)
{
    for (int i = 0; i < (int)m_conns.size(); i++)
    {
        pipeConnection* pc = dynamic_cast<pipeConnection*>(m_conns[i]);
        if(pc && (pc->endpoint().port() == port))
            return pc;
    }
    return NULL;
}
#endif // USE_PIPE_CLIENT

bool connections::disconnect(connection* c)
{
    // if (!c) return false;
    mutex::scoped_lock lck(m_mutex);
    c->release();
    if (c->refCount() == 0)
    {
        for (int i = (int)m_conns.size() - 1; i >= 0; --i)
        {
            if (c == m_conns[i])
            {
                m_conns.erase(m_conns.begin() + i);
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
    asio::ip::tcp::endpoint local(ip::address::from_string("127.0.0.1"), ep.port()); 
    if (local == ep)
        return true;
    char buf[MAX_PATH];
    if (::gethostname(buf, MAX_PATH) == 0)
    {
        boost::system::error_code ec;
        local = endpoint(buf, m_port, ec);
        if (local == ep)
            return true;
    }
    return false;
}

inline connection* connections::createConnection(asio::ip::tcp::endpoint& ep,
                                     bool namedPipe)
{
#ifdef USE_PIPE_CLIENT
    if (namedPipe)
        return new pipeConnection(ep, m_pipeName);
    else
#endif
#ifdef __APPLE__
        return new asio_tcpConnection(ep);
#else
        return new native_tcpConnection(ep);
#endif
}

inline connection* connections::doConnect(connection* c)
{
    try
    {
        c->connect();
        if (c->isConnected())
            return c;
        m_e = c->error();
        delete c;
        return NULL;
    }
    catch (bzs::netsvc::client::exception& e)
    {
        m_e = boost::system::error_code(e.error(), get_system_category());
        delete c;
        return NULL;
    }
    catch (boost::system::system_error& e)
    {
        m_e = e.code();
        delete c;
        return NULL;
    }
    catch (std::exception& /*e*/)
    {
        m_e = boost::system::error_code(1, get_system_category());
        delete c;
        return NULL;
    }
    catch (...)
    {
        m_e = boost::system::error_code(1, get_system_category());
        delete c;
        return NULL;
    }
}

inline bool connections::doHandShake(connection* c, handshake f, void* data)
{
    bool ret = true;
    try
    {
        // 1.0 - 2.1 namepd pipe is not HandShakable.
        if (c->isHandShakable())
        {
            if (!f)
                c->read();
            else
                ret = f(c, data);
            if (c->error())
                m_e = asio::error::connection_refused;
            else if (!ret)
                m_e = asio::error::no_permission;
            if (!ret)
                delete c;
        }
        return ret;
    }
    catch (bzs::netsvc::client::exception& e)
    {
        delete c;
        m_e = boost::system::error_code(e.error(), get_system_category());
        return false;
    }
    catch (boost::system::system_error& e)
    {
        m_e = e.code();
        delete c;
        return false;
    }
}

#if defined(__BCPLUSPLUS__)
#pragma warn -8004
#endif
// The connection of found from connection list of same address is returned.
connection* connections::connect(const std::string& hst, const char* port,
        handshake f, void* data, bool newConnection, bool clearNRCache)
{
    bool namedPipe = false;
    connection* c;
    std::string host = hst;
    unsigned int opt = clearNRCache ? HST_OPTION_CLEAR_CACHE : 0;
    char buf[MAX_PATH];
    if (m_haNameResolver)
    {
        m_haNameResolver(hst.c_str(), port, buf, opt);
        char* p = strchr(buf, ':');
        if (p)
        {
            *p = 0x00;
            port = p + 1;
        }
        host = buf;
    }
    mutex::scoped_lock lck(m_mutex);
    asio::ip::tcp::endpoint ep = endpoint(host, port, m_e);
    if (m_e)
        return NULL;
#ifdef USE_PIPE_CLIENT
    namedPipe =  (m_usePipedLocal && isUseNamedPipe(ep));
    if (namedPipe)
        c = newConnection ? NULL : getConnectionPipe(ep.port());
    else
#endif
        c = newConnection ? NULL : getConnection(ep);
    if (newConnection || !c)
    {
        c = createConnection(ep, namedPipe);
        c->setUserOptions(opt);
        c = doConnect(c);
        if (!c || !doHandShake(c, f, data))
            return NULL;
        m_conns.push_back(c);
    }
    c->addref();
    return c;
}
#if defined(__BCPLUSPLUS__)
#pragma warn .8004
#endif

bool connections::reconnect(connection* c, const std::string& hst, const char* port,
                        handshake f, void* data)
{
    boost::system::error_code ec;
    std::string host = hst;
    unsigned int opt = HST_OPTION_CLEAR_CACHE;
    char buf[MAX_PATH];
    if (m_haNameResolver)
    {
        m_haNameResolver(hst.c_str(), port, buf, opt);
        char* p = strchr(buf, ':');
        if (p)
        {
            *p = 0x00;
            port = p + 1;
        }
        host = buf;
    }
    mutex::scoped_lock lck(m_mutex);
    asio::ip::tcp::endpoint ep = endpoint(host, port, ec);
    if (ec)
        return false;
    c->setUserOptions(opt);
    c->reconnect(ep);
    if (!c || !doHandShake(c, f, data))
        return false;
    return true;
}

int connections::connectionCount()
{
    return (int)m_conns.size();
}

void connections::registHaNameResolver(HANAME_RESOLVER_PTR func)
{
    mutex::scoped_lock lck(m_mutex);
    m_haNameResolver = func;    
}


} // namespace client
} // namespace netsvc
} // namespace bzs
