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

char connections::port[PORTNUMBUF_SIZE] = { "8610" };
bool connections::m_usePipedLocal = true;
#ifdef _WIN32
int connections::connectTimeout = 3000;
int connections::netTimeout = 20000;
#else
int connections::connectTimeout = 3;
int connections::netTimeout = 20;
#endif


connections::connections(const char* pipeName) : m_pipeName(pipeName),m_resolver(m_ios)
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
        strcpy_s(port, PORTNUMBUF_SIZE, tmp);
        GetPrivateProfileString("transctd_client", "connectTimeout", "3", tmp, 30,
                                buf);
        connectTimeout = (short)atol(tmp)*1000;
        GetPrivateProfileString("transctd_client", "netTimeout", "20", tmp, 30,
                                buf);
        netTimeout = (short)atol(tmp)*1000;
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
            strcpy_s(port, PORTNUMBUF_SIZE, p.c_str());

            p = pt.get<std::string>("transctd_client.connectTimeout");
            connectTimeout = atol(p.c_str());
            if (connectTimeout == 0)
                connectTimeout = 3;
            p = pt.get<std::string>("transctd_client.netTimeout");
            netTimeout = atol(p.c_str());
            if (netTimeout == 0)
                netTimeout = 20;
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
                                              boost::system::error_code& ec)
{
    tcp::resolver::query query(host, port);

    tcp::resolver::iterator dest = m_resolver.resolve(query, ec);
    tcp::endpoint endpoint;
    if (!ec)
    {
        while (dest != tcp::resolver::iterator())
            endpoint = *dest++;
    }
    return endpoint;
}

connection* connections::getConnection(const std::string& host)
{
    mutex::scoped_lock lck(m_mutex);
    boost::system::error_code ec;
    tcp::endpoint ep = endpoint(host, ec);
    if (!ec)
        return getConnection(ep);
    return NULL;
}

#ifdef USE_PIPE_CLIENT

connection* connections::getConnectionPipe()
{
    for (int i = 0; i < (int)m_conns.size(); i++)
    {
        pipeConnection* pc = dynamic_cast<pipeConnection*>(m_conns[i]);
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
    boost::system::error_code ec;
    asio::ip::tcp::endpoint local = endpoint("127.0.0.1", ec);
    if (!ec && (local == ep))
        return true;
    char buf[MAX_PATH];
    if (::gethostname(buf, MAX_PATH) == 0)
    {
        local = endpoint(buf, ec);
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
        return new native_tcpConnection(ep);
        //return new asio_tcpConnection(ep);
}

inline connection* connections::doConnect(connection* c)
{
    try
    {
        c->connect();
        return c;
    }
    catch (bzs::netsvc::client::exception& e)
    {
        delete c;
        throw e;
    }
    catch (boost::system::system_error& e)
    {
        delete c;
        throw e;
    }
    catch (std::exception& e)
    {
        delete c;
        throw e;
    }
    catch (...)
    {
        delete c;
        throw;
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
            if (!ret)
                delete c;
        }
        return ret;
    }
    catch (bzs::netsvc::client::exception& /*e*/)
    {
        delete c;
        throw;
    }
    catch (boost::system::system_error& /*e*/)
    {
        delete c;
        throw;
    }
}

// The connection of found from connection list of same address is returned.
connection* connections::connect(const std::string& host, handshake f, void* data, bool newConnection)
{
    bool namedPipe = false;
    boost::system::error_code ec;
    connection* c;
    mutex::scoped_lock lck(m_mutex);
    asio::ip::tcp::endpoint ep = endpoint(host, ec);
    if (ec)
        return NULL;
#ifdef USE_PIPE_CLIENT
    namedPipe =  (m_usePipedLocal && isUseNamedPipe(ep));
    if (namedPipe)
        c = newConnection ? NULL : getConnectionPipe();
    else
#endif
        c = newConnection ? NULL : getConnection(ep);
    if (newConnection || !c)
    {
        c = createConnection(ep, namedPipe); 
        c = doConnect(c);
        if (!c || !doHandShake(c, f, data))
            return NULL;
        m_conns.push_back(c);
    }
    c->addref();
    return c;
}

bool connections::reconnect(connection* c, const std::string& host,
                        handshake f, void* data)
{
    boost::system::error_code ec;
    mutex::scoped_lock lck(m_mutex);
    asio::ip::tcp::endpoint ep = endpoint(host, ec);
    if (ec)
        return false;
    c->reconnect(ep);
    if (!c || !doHandShake(c, f, data))
        return false;
    return true;
}

int connections::connectionCount()
{
    return (int)m_conns.size();
}

} // namespace client
} // namespace netsvc
} // namespace bzs
