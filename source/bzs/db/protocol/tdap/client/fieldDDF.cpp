/* =================================================================
 Copyright (C) 2000-2013 BizStation Corp All rights reserved.

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
#include <bzs/env/tstring.h>
#pragma hdrstop

#include "fieldDDF.h"
#include "nsDatabase.h"
#pragma package(smart_init)

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

fieldDDF::fieldDDF(nsdatabase *pbe) : nstable(pbe) {m_keybuflen = 128;}

void fieldDDF::doOpen(const _TCHAR* Dir, char_td mode, const _TCHAR* OwnerName)
{
    _TCHAR FullPath[256];

    _tcscpy(m_DataDir, Dir);
    _tcscpy(FullPath, Dir);
    _tcscat(FullPath, PSEPARATOR);
    _tcscat(FullPath, _T("FIELD.DDF"));
    nstable::doOpen(FullPath, mode, OwnerName);

    m_pdata = &datbuf;
    m_buflen = sizeof(SFIELDDDF);
    m_datalen = sizeof(SFIELDDDF);
    m_keybuf = &keybuf[0];
    m_keynum = 0;
}

keylen_td fieldDDF::writeKeyData()
{
    keylen_td len = 2;
    switch (m_keynum)
    {
    case 0: memcpy(&keybuf[0], &fieldid, len);
        break;
    case 1: memcpy(&keybuf[0], &fileid, len);
        break;
    case 2:
        len = 20;
        memcpy(&keybuf[0], &name, len);
        break;
    case 3:
        memcpy(&keybuf[0], &fileid, len);
        memcpy(&keybuf[2], &name, 20);
        len += 20;
        break;
    }
    return len;
}

void fieldDDF::writeRecordData()
{
    datbuf.fieldid = fieldid;
    datbuf.fileid = fileid;
    strncpy(datbuf.name, name, 20);
    for (size_t i = strlen(datbuf.name); i < 20; i++)
        datbuf.name[i] = ' ';

#ifdef DDF_PSQLV_MODE
    if (type == ft_logical)
        type = ft_uinteger;

#endif
    datbuf.type = type;
    datbuf.pos = pos;
    datbuf.len = len;
    datbuf.dec = dec;

    datbuf.flag = flag;

}

void fieldDDF::onReadAfter()
{
    fieldid = datbuf.fieldid;
    fileid = datbuf.fileid;
    strncpy(name, datbuf.name, 20);
    name[20] = 0x00;

    type = datbuf.type;
    pos = datbuf.pos;
    len = datbuf.len;
    dec = datbuf.dec;
    flag = datbuf.flag;

}

void fieldDDF::createTable(const _TCHAR* fullpath)
{
    fileSpec *fs;
    fs = (fileSpec*)malloc(512);
    memset(fs, 512, 0x00);
    fs->recLen = 32;
    fs->pageSize = 4096;
    fs->indexCount = 5;
    fs->fileFlag.all = 0;
    fs->preAlloc = 0;
    fs->keySpecs[0].keyPos = 1;
    fs->keySpecs[0].keyLen = 2;
    fs->keySpecs[0].keyFlag.all = 260;
    fs->keySpecs[0].keyType = 14;

    fs->keySpecs[1].keyPos = 3;
    fs->keySpecs[1].keyLen = 2;
    fs->keySpecs[1].keyFlag.all = 263;
    fs->keySpecs[1].keyType = 14;

    fs->keySpecs[2].keyPos = 5;
    fs->keySpecs[2].keyLen = 20;
    fs->keySpecs[2].keyFlag.all = 1283;
    fs->keySpecs[2].keyType = 0;

    fs->keySpecs[3].keyPos = 3;
    fs->keySpecs[3].keyLen = 2;
    fs->keySpecs[3].keyFlag.all = 278;
    fs->keySpecs[3].keyType = 14;

    fs->keySpecs[4].keyPos = 5;
    fs->keySpecs[4].keyLen = 20;
    fs->keySpecs[4].keyFlag.all = 1282;
    fs->keySpecs[4].keyType = 0;

    fs->keySpecs[5].keyPos = 3;
    fs->keySpecs[5].keyLen = 2;
    fs->keySpecs[5].keyFlag.all = 279;
    fs->keySpecs[5].keyType = 14;

    fs->keySpecs[6].keyPos = 26;
    fs->keySpecs[6].keyLen = 2;
    fs->keySpecs[6].keyFlag.all = 279;
    fs->keySpecs[6].keyType = 14;

    fs->keySpecs[7].keyPos = 30;
    fs->keySpecs[7].keyLen = 1;
    fs->keySpecs[7].keyFlag.all = 263;
    fs->keySpecs[7].keyType = 14;

    nsdb()->createTable(fs, 412, fullpath, 0);
    free(fs);
    m_stat = nsdb()->stat();
}

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs
