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
#include "indexDDF.h"
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

indexDDF::indexDDF(nsdatabase *pbe) : nstable(pbe) {m_keybuflen = 128;}

void indexDDF::doOpen(const _TCHAR* Dir, char_td mode, const _TCHAR* OwnerName)
{
	_TCHAR FullPath[256];

	_tcscpy(m_DataDir, Dir);
	_tcscpy(FullPath, Dir);
	_tcscat(FullPath, PSEPARATOR);
	_tcscat(FullPath, _T("INDEX.DDF"));
	nstable::doOpen(FullPath, mode, OwnerName);

	m_pdata = &datbuf;
	m_buflen = sizeof(SIDXDDF);
	m_datalen = sizeof(SIDXDDF);
	m_keybuf = &keybuf[0];
	m_keybuflen = 128;
	m_keynum = 0;
}

keylen_td indexDDF::writeKeyData()
{
	keylen_td len = 2;
	switch (m_keynum)
	{
	case 0: memcpy(&keybuf[0], &fileid, len);
		break;
	case 1: memcpy(&keybuf[0], &fieldid, len);
		break;

	}
	return len;
}

void indexDDF::writeRecordData()
{
	datbuf.fieldid = fieldid;
	datbuf.fileid = fileid;
	datbuf.flag = flag;
	datbuf.keyid = keyid;
	datbuf.segmentnum = segmentnum;

}

void indexDDF::onReadAfter()
{
	fieldid = datbuf.fieldid;
	fileid = datbuf.fileid;
	flag = datbuf.flag;
	keyid = datbuf.keyid;
	segmentnum = datbuf.segmentnum;

}

void indexDDF::createTable(const _TCHAR* fullpath)
{
	fileSpec *fs;
	fs = (fileSpec*)malloc(512);
	memset(fs, 512, 0x00);
	fs->recLen = 10;
	fs->pageSize = 4096;
	fs->indexCount = 3;
	fs->fileFlag.all = 0;
	fs->preAlloc = 0;
	fs->keySpecs[0].keyPos = 1;
	fs->keySpecs[0].keyLen = 2;
	fs->keySpecs[0].keyFlag.all = 261;
	fs->keySpecs[0].keyType = 14;

	fs->keySpecs[1].keyPos = 3;
	fs->keySpecs[1].keyLen = 2;
	fs->keySpecs[1].keyFlag.all = 261;
	fs->keySpecs[1].keyType = 14;

	fs->keySpecs[2].keyPos = 1;
	fs->keySpecs[2].keyLen = 2;
	fs->keySpecs[2].keyFlag.all = 278;
	fs->keySpecs[2].keyType = 14;

	fs->keySpecs[3].keyPos = 5;
	fs->keySpecs[3].keyLen = 2;
	fs->keySpecs[3].keyFlag.all = 278;
	fs->keySpecs[3].keyType = 14;

	fs->keySpecs[4].keyPos = 7;
	fs->keySpecs[4].keyLen = 2;
	fs->keySpecs[4].keyFlag.all = 262;
	fs->keySpecs[4].keyType = 14;

	nsdb()->createTable(fs, 412, fullpath, 0);
	free(fs);
	m_stat = nsdb()->stat();
}

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs
