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
#include "client.h"
#include "sqlBuilder.h"
#include "stringConverter.h"

using namespace bzs::db::protocol::tdap::client;

bzs::netsvc::client::connections* m_cons = NULL;

namespace bzs
{
namespace db
{
namespace protocol
{
namespace tdap
{
namespace client
{

#ifdef USETLS
tls_key g_tlsiID;
#else
__THREAD client* __THREAD_BCB g_client = NULL;
#endif

#if defined(__BCPLUSPLUS__)
#pragma warn -8004
#endif
short errorCode(const boost::system::error_code& e)
{
    short ret = 0;
    switch (e.value())
    {
    case 11004:
    case 11001:
        ret = ERROR_TD_HOSTNAME_NOT_FOUND;
        break;
    case 10060:
    case 10057: //blocking fire wall
    case 110:   //connect: Connection timed out
    case 121:   //timeout sema
    case 11:    //EAGAIN
        ret = ERROR_TD_NET_TIMEOUT;
        break;
    case 32:    //write:brokn pipe
    case 111:   //connect: Connection refused
    case 10061:
        ret = ERROR_TD_CONNECTION_FAILURE;
        break;
    case 104:   //write: Connection reset by peer
    case 10054:
        ret = ERROR_TD_NET_REMOTE_DISCONNECT;
        break;
    case 232:
    case 109:
    case 2:
    case 1:
        ret = ERROR_TD_INVALID_CLINETHOST;
        break;
    default:
        ret = ERROR_TD_NET_OTHER;
    }
    return ret;
}
#if defined(__BCPLUSPLUS__)
#pragma warn .8004
#endif

int client::getServerCharsetIndex()
{
    bzs::netsvc::client::connection* c = con();
    if (!c)
        return -1;
    int v = c->charsetServer();
    if (v != -1)
        return v;
        
    request req = m_req;
    req.paramMask = P_MASK_POSBLK | P_MASK_DATA | P_MASK_DATALEN;
    trdVersiton ver;
    memset(&ver, 0, sizeof(trdVersiton));
    ver.cherserServer[0] = 0x00;
    ver.clMajor = (ushort_td)atoi(C_INTERFACE_VER_MAJOR);
    ver.clMinor = (ushort_td)atoi(C_INTERFACE_VER_MINOR);
    ver.clRelease = (ushort_td)atoi(C_INTERFACE_VER_RELEASE);
    uint_td len = sizeof(trdVersiton);
    req.op = TD_GETSERVER_CHARSET;
    req.data = &ver;
    req.datalen = &len;

    mutex::scoped_lock lck(m_mutex);
    char* p = con()->sendBuffer(m_req.sendLenEstimate());
    unsigned int size = req.serialize(p);
    p = con()->asyncWriteRead(size);
    if (!con()->error())
    {
        req.parse(p, false);
        if (req.result == 0)
        {
            if (!checkVersion(ver))
                return -1;
            c->setCharsetServer(mysql::charsetIndex(ver.cherserServer));
            return  c->charsetServer();
        }
    }
    return -1;
}

void client::addSecondCharsetData(unsigned int destCodePage, std::string& src)
{
    stringConverter cv(destCodePage, CP_UTF8);
    int osize = (int)src.size() * 3;
    char* srvchar = new char[osize];

    size_t len = cv.convert(srvchar, osize, src.c_str(), src.size());
    srvchar[len] = 0x00;
    src += std::string("\t") + srvchar;
    delete[] srvchar;
}

bool client::buildDualChasetKeybuf()
{
    int charsetIndexServer = getServerCharsetIndex();
    if (charsetIndexServer == -1)
        return false;

    // remove auth info
    {
        _TCHAR tmp[MAX_KEYLEN+128]={0};
        m_serverCharData = stripAuth((const char*)m_req.keybuf, tmp, MAX_KEYLEN);

        if (CHARSET_UTF8 != charsetIndexServer)
            addSecondCharsetData(mysql::codePage(charsetIndexServer),
                                 m_serverCharData);
        else
            m_serverCharData += std::string("\t") + tmp;
    }
    //Add  Auth infomation
    if (m_cryptPwd)
        m_serverCharData += std::string("\t") +
           std::string(m_cryptPwd, strlen(m_cryptPwd + 20) + MYSQL_SCRAMBLE_LENGTH);

    m_req.keybuf = (void_td*)m_serverCharData.c_str();
    m_req.keylen = (keylen_td)m_serverCharData.size();
    return true;
}

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs
