/*=================================================================
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
=================================================================*/
#include "nsTable.h"
#include <stdio.h>

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

PACKAGE _TCHAR* getErrorMessageLocale(int errorCode, _TCHAR* buf, size_t size)
{
    const _TCHAR* p = 0x00;
    switch (errorCode)
    {
    case STATUS_LOCK_ERROR:
    case STATUS_FILE_LOCKED:
    case STATUS_CANNOT_LOCK_TABLE:
        p = _T("‚±‚ÌƒŒƒR[ƒh‚ÍŠù‚É‘¼‚Ìƒ†[ƒU[‚É‚æ‚èƒƒbƒN‚³‚ê‚Ä‚¢‚Ü‚·B");
        break;
    case STATUS_CHANGE_CONFLICT:
        p = _T("‚±‚ÌƒŒƒR[ƒh‚Í¡A‘¼‚Ìƒ†[ƒU[‚É‚æ‚è•ÏX‚³‚ê‚Ü‚µ‚½B");
        break;
    case STATUS_TABLE_YET_OPEN:
        p = _T("‚±‚Ìƒe[ƒuƒ‹‚Í‚Ü‚¾ OPEN ‚³‚ê‚Ä‚¢‚Ü‚¹‚ñB");
        break;
    case STATUS_DURING_TRANSACTION:
        p = _T("‚±‚Ìƒe[ƒuƒ‹‚Í‚Ü‚¾ƒgƒ‰ƒ“ƒUƒNƒVƒ‡ƒ“’†‚Å‚·Bclose ‚Å‚«‚Ü‚¹‚ñB");
        break;
    case STATUS_NO_ACR_UPDATE_DELETE:
        p = _T("XV‚Ü‚½‚ÍíœƒAƒNƒZƒXŒ ‚ª‚ ‚è‚Ü‚¹‚ñB");
        break;
    case STATUS_NO_ACR_INSERT:
        p = _T("’Ç‰ÁƒAƒNƒZƒXŒ ‚ª‚ ‚è‚Ü‚¹‚ñB");
        break;
    case STATUS_NO_ACR_READ:
        p = _T("“Ç‚İæ‚èƒAƒNƒZƒXŒ ‚ª‚ ‚è‚Ü‚¹‚ñB");
        break;
    case STATUS_CANT_ALLOC_MEMORY:
        p = _T("ƒƒ‚ƒŠ‚ªŠm•Û‚Å‚«‚Ü‚¹‚ñ‚Å‚µ‚½B");
        break;
    case STATUS_USE_KEYFIELD:
        p = _T("‚±‚ÌƒtƒB[ƒ‹ƒh‚ÍƒL[‚Åg—p‚³‚ê‚Ä‚¢‚é‚½‚ßíœ‚Å‚«‚Ü‚¹‚ñB");
        break;
    case STATUS_TOO_MANY_TABLES:
        p = _T("ŠÇ—‰Â”\‚Èƒe[ƒuƒ‹”‚ğ’´‚¦‚Ä‚¢‚Ü‚·B");
        break;
    case STATUS_INVARID_PRM_KEY_NUM:
        p = _T("MainKeyƒL[”Ô†‚ª•s³‚Å‚·B");
        break;
    case STATUS_INVARID_PNT_KEY_NUM:
        p = _T("ParentKeyƒL[”Ô†‚ª•s³‚Å‚·B");
        break;
    case STATUS_INVARID_REP_KEY_NUM:
        p = _T("ReplicaKeyƒL[”Ô†‚ª•s³‚Å‚·B");
        break;
    case STATUS_INVARID_FIELD_IDX:
        p = _T("ƒtƒB[ƒ‹ƒhƒCƒ“ƒfƒbƒNƒX‚ª—LŒø”ÍˆÍ‚É‚ ‚è‚Ü‚¹‚ñB");
        break;
    case STATUS_ALREADY_DELETED:
        p = _T("‚±‚ÌƒAƒCƒeƒ€‚Ííœ‚³‚ê‚Ä‚¢‚Ü‚·B");
        break;
    case STATUS_LMITS_MAX_TABLES:
        p = _T("ƒI[ƒvƒ“‚Å‚«‚éƒe[ƒuƒ‹‚ÌÅ‘å”‚ğ’´‚¦‚Ä‚¢‚Ü‚·B");
        break;
    case STATUS_DB_YET_OPEN:
        p = _T("ƒf[ƒ^ƒx[ƒX‚ªƒI[ƒvƒ“‚³‚ê‚Ä‚¢‚Ü‚¹‚ñB");
        break;
    case STATUS_TABLENAME_NOTFOUND:
        p = _T("w’è‚µ‚½ƒe[ƒuƒ‹–¼‚ªŒ©‚Â‚©‚è‚Ü‚¹‚ñB");
        break;
    case STATUS_DIFFERENT_DBVERSION:
        p = _T("ƒf[ƒ^ƒx[ƒX‚Ìƒo[ƒWƒ‡ƒ“‚ªˆá‚¤‚©A”j‘¹‚µ‚Ä‚¢‚Ü‚·B");
        break;
    case STATUS_DUPLICATE_FIELDNAME:
        p = _T("ƒtƒB[ƒ‹ƒh–¼‚ªd•¡‚µ‚Ä‚¢‚Ü‚·B");
        break;
    case STATUS_INVALID_TABLE_IDX:
        p = _T("’è‹`‚³‚ê‚È‚¢ƒe[ƒuƒ‹”Ô†‚Å‚·B");
        break;
    case STATUS_AUTH_DENIED:
        p = _T("ƒ†[ƒU[–¼‚Ü‚½‚ÍƒpƒXƒ[ƒh‚ª•s³‚Å‚·B");
        break;
    case STATUS_TOO_MANY_FIELDS:
        p = _T("ŠÇ—‰Â”\‚ÈƒtƒB[ƒ‹ƒh”‚ğ’´‚¦‚Ä‚¢‚Ü‚·B");
        break;
    case STATUS_FILTERSTRING_ERROR:
        p = _T("ƒtƒBƒ‹ƒ^•¶š—ñ‚ÉŒë‚è‚ª‚ ‚è‚Ü‚·B");
        break;
    case STATUS_INVALID_FIELDLENGTH:
        p = _T("ƒtƒB[ƒ‹ƒh’·‚ª•s³‚Å‚·B");
        break;
    case STATUS_INVALID_KEYTYPE:
        p = _T("g—p‚Å‚«‚È‚¢ƒL[ƒ^ƒCƒv‚ªw’è‚³‚ê‚Ä‚¢‚Ü‚·B");
        break;
    case STATUS_LVAR_NOTE_NOT_LAST:
        p = _T("Note‹y‚ÑLvarƒ^ƒCƒv‚ÍƒtƒB[ƒ‹ƒh‚Ìˆê”ÔÅŒã‚Å‚È‚¯‚ê‚Î‚È‚è‚Ü‚¹‚ñ")
            _T("B");
        break;
    case STATUS_INVALID_VARIABLETABLE:
        p = _T("‰Â•Ï’·ƒe[ƒuƒ‹‚ÌÅŒã‚ÌƒtƒB[ƒ‹ƒh‚ÍNote Lvar ")
            _T("varbinaryƒ^ƒCƒv‚ª•K—v‚Å‚·B");
        break;
    case STATUS_NODEF_FOR_CONVERT:
        p = _T("ƒRƒ“ƒo[ƒgŒ³’è‹`‚ª‚ ‚è‚Ü‚¹‚ñB");
        break;
    case STATUS_TRD_NEED_VARLENGTH:
        p = _T("‰Â•Ï’·ƒe[ƒuƒ‹‚Ìw’è‚ª•K—v‚Å‚·B");
        break;
    case STATUS_TOO_LONG_OWNERNAME:
        p = _T("ƒI[ƒi[ƒl[ƒ€‚ª’·‚·‚¬‚Ü‚·B");
        break;
    case STATUS_CANT_DEL_FOR_REL:
        p = _T("QÆ®‡«‚Ì‚½‚ßíœ‚Å‚«‚Ü‚¹‚ñB");
        break;
    case STATUS_NO_AUTOINC_SPACE:
        p = _T("AutoIncEx‚ÌƒXƒy[ƒX‚ª‚ ‚è‚Ü‚¹‚ñB");
        break;
    case STATUS_INVALID_RECLEN:
        p = _T("ƒŒƒR[ƒh’·’è‹`‚ª•s³‚©Aƒe[ƒuƒ‹‚ªƒI[ƒvƒ“‚³‚ê‚Ä‚¢‚Ü‚¹‚ñB");
        break;
    case STATUS_INVALID_FIELDVALUE:
        p = _T("ƒtƒB[ƒ‹ƒh‚Ì’l‚ª•s³‚Å‚·B");
        break;
    case STATUS_INVALID_VALLEN:
        p = _T("‰Â•Ï’·ƒŒƒR[ƒh‚Ì’·‚³‚ªƒoƒbƒtƒ@ƒTƒCƒY‚ğ’´‚¦‚Ä‚¢‚Ü‚·B");
        break;
    case STATUS_FIELDTYPE_NOTSUPPORT:
        p = _T("This field type is not supported.");
        break;
    case STATUS_DUPPLICATE_KEYVALUE:
        p = _T("ƒL[’l‚ªd•¡‚µ‚Ä‚¢‚é‚½‚ß“o˜^‚Å‚«‚Ü‚¹‚ñB");
        break;
    case STATUS_REQUESTER_DEACTIVE:
        p = _T("ƒf[ƒ^ƒx[ƒXƒGƒ“ƒWƒ“‚ğƒ[ƒh‚Å‚«‚Ü‚¹‚ñB\r\nTerminal ")
            _T("Service‹y‚ÑAWindowsXP‚Ìƒ†[ƒU[Ø‚è‘Ö‚¦")
            _T
			("‚É‚¨‚¢‚ÄA“¯‚É‘½”‚Ìƒ†[ƒU[‚ª—˜—p‚·‚é‚É‚ÍAPervasive.SQL 2000i ServerˆÈã‚ÌƒCƒ“ƒXƒg[ƒ‹‚ª•K—v‚Å‚·B");
        break;
    case STATUS_ACCESS_DENIED:
        p = _T("XV‚Ì‚½‚ß‚ÌƒAƒNƒZƒXŒ ‚ª‚ ‚è‚Ü‚¹‚ñB\r\nƒtƒ@ƒCƒ‹‚ÍƒŠ[ƒhƒIƒ“ƒŠ")
            _T("[‚©ƒpƒXƒ[ƒh‚ª³‚µ‚­‚ ‚è‚Ü‚¹‚ñB");
        break;
    case STATUS_CANT_CREATE:
        p = _T("ƒf[ƒ^ƒx[ƒX‚Ìì¬‚É¸”s‚µ‚Ü‚µ‚½BŠù‚Éƒf[ƒ^ƒx[ƒX‚ª‘¶İ‚µ‚Ä‚¢")
            _T("‚È‚¢‚©Šm”F‚µ‚Ä‚­‚¾‚³‚¢B");
        break;
    default:
        _stprintf_s(buf, 256, _T("ƒf[ƒ^ƒx[ƒXƒIƒyƒŒ[ƒVƒ‡ƒ“‚ÅƒGƒ‰[‚ª”­¶‚µ‚Ü")
                              _T("‚µ‚½B\r\nƒGƒ‰[”Ô†‚Í %d \r\n ")
                              _T("ˆ—‚ğ’†~‚µ‚Ü‚·B"),
                    errorCode);
    }
    if (p)
    {
        _tcsncpy(buf, p, size);
        buf[size - 1] = 0x00;
    }
    return buf;
}

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs
