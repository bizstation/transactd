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

bool checkVersion(trdVersiton& ver)
{
    if ((ver.srvMajor < 1) || ((ver.srvMajor == 1) && (ver.srvMinor < 3)))
        return false;
    return true;
}

bool client::readServerCharsetIndex()
{
    if (m_charsetIndexServer == -1)
    {
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
        req.parse(p, false);
        if (req.result == 0)
        {
            if (!checkVersion(ver))
                return false;
            m_charsetIndexServer = mysql::charsetIndex(ver.cherserServer);
            return true;
        }
        return false;
    }
    return true;
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
    if (!readServerCharsetIndex())
        return false;
    // m_serverCharData = std::string((char*)m_req.keybuf, m_req.keylen);
    m_serverCharData = (char*)m_req.keybuf;
    if (CHARSET_UTF8 != m_charsetIndexServer)
        addSecondCharsetData(mysql::codePage(m_charsetIndexServer),
                             m_serverCharData);
    else
        m_serverCharData += std::string("\t") + (char*)m_req.keybuf;

    m_req.keybuf = (void_td*)m_serverCharData.c_str();
    m_req.keylen = (keylen_td)m_serverCharData.size() + 1;
    return true;
}

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs
