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

#include "fileDDF.h"
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

fileDDF::fileDDF(nsdatabase *pbe) : nstable(pbe) {m_keybuflen = 128;

}

fileDDF::~fileDDF() {

}

void fileDDF::doOpen(const _TCHAR* Dir, char_td mode, const _TCHAR* OwnerName)
{
	_TCHAR FullPath[256];

	_tcscpy(m_DataDir, Dir);
	_tcscpy(FullPath, Dir);
	_tcscat(FullPath, PSEPARATOR);
	_tcscat(FullPath, _T("FILE.DDF"));
	nstable::doOpen(FullPath, mode, OwnerName);

	m_pdata = &datbuf;
	m_buflen = sizeof(SFILEDDF);
	m_datalen = sizeof(SFILEDDF);
	m_keybuf = &keybuf[0];
	m_keybuflen = 128;
	m_keynum = 0;
}

keylen_td fileDDF::writeKeyData()
{
	keylen_td len = 2;
	switch (m_keynum)
	{
	case 0: memcpy(&keybuf[0], &id, len);
		break;
	case 1:
		len = 20;
		memcpy(&keybuf[0], &tablename, len);

		break;
	}
	return len;
}

void fileDDF::writeRecordData()
{
	datbuf.id = id;
	strncpy(datbuf.tablename, tablename, 20);
	for (size_t i = strlen(datbuf.tablename); i < 20; i++)
		datbuf.tablename[i] = ' ';

	strncpy(datbuf.filename, filename, 64);
	for (size_t i = strlen(datbuf.filename); i < 64; i++)
		datbuf.filename[i] = ' ';

	datbuf.flag = flag;
	memset(datbuf.filler, 0, 9);

}

void fileDDF::onReadAfter()
{
	id = datbuf.id;
	strncpy(tablename, datbuf.tablename, 20);
	tablename[20] = 0x00;
	strncpy(filename, datbuf.filename, 64);
	filename[64] = 0x00;
	flag = datbuf.flag;

}

void fileDDF::createTable(const _TCHAR* fullpath)
{
	fileSpec *fs;
	fs = (fileSpec*)malloc(512);
	memset(fs, 512, 0x00);
	fs->recLen = 97;
	fs->pageSize = 4096;
	fs->indexCount = 2;
	fs->fileFlag.all = 0;
	fs->preAlloc = 0;
	fs->keySpecs[0].keyPos = 1;
	fs->keySpecs[0].keyLen = 2;
	fs->keySpecs[0].keyFlag.all = 260;
	fs->keySpecs[0].keyType = 14;

	fs->keySpecs[1].keyPos = 3;
	fs->keySpecs[1].keyLen = 20;
	fs->keySpecs[1].keyFlag.all = 1282;
	fs->keySpecs[1].keyType = 0;

	nsdb()->createTable(fs, 412, fullpath, 0);
	free(fs);
	m_stat = nsdb()->stat();
}

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs
