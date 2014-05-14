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
	case STATUS_CANNOT_LOCK_TABLE: p = _T("���̃��R�[�h�͊��ɑ��̃��[�U�[�ɂ�胍�b�N����Ă��܂��B");break;
	case STATUS_CHANGE_CONFLICT: p = _T("���̃��R�[�h�͍��A���̃��[�U�[�ɂ��ύX����܂����B");break;
	case STATUS_TABLE_YET_OPEN: p = _T("���̃e�[�u���͂܂� OPEN ����Ă��܂���B");break;
	case STATUS_DURING_TRANSACTION: p = _T("���̃e�[�u���͂܂��g�����U�N�V�������ł��Bclose �ł��܂���B");break;
	case STATUS_NO_ACR_UPDATE_DELETE: p = _T("�X�V�܂��͍폜�A�N�Z�X��������܂���B");break;
	case STATUS_NO_ACR_INSERT: p = _T("�ǉ��A�N�Z�X��������܂���B");break;
	case STATUS_NO_ACR_READ: p = _T("�ǂݎ��A�N�Z�X��������܂���B");break;
	case STATUS_CANT_ALLOC_MEMORY: p = _T("���������m�ۂł��܂���ł����B");break;
	case STATUS_USE_KEYFIELD: p = _T("���̃t�B�[���h�̓L�[�Ŏg�p����Ă��邽�ߍ폜�ł��܂���B");break;
	case STATUS_TOO_MANY_TABLES: p = _T("�Ǘ��\�ȃe�[�u�����𒴂��Ă��܂��B");break;
	case STATUS_INVARID_PRM_KEY_NUM: p = _T("MainKey�L�[�ԍ����s���ł��B");break;
	case STATUS_INVARID_PNT_KEY_NUM: p = _T("ParentKey�L�[�ԍ����s���ł��B");break;
	case STATUS_INVARID_REP_KEY_NUM: p = _T("ReplicaKey�L�[�ԍ����s���ł��B");break;
	case STATUS_INVARID_FIELD_IDX: p = _T("�t�B�[���h�C���f�b�N�X���L���͈͂ɂ���܂���B");break;
	case STATUS_ALREADY_DELETED: p = _T("���̃A�C�e���͍폜����Ă��܂��B");break;
	case STATUS_LMITS_MAX_TABLES: p = _T("�I�[�v���ł���e�[�u���̍ő吔�𒴂��Ă��܂��B");break;
	case STATUS_DB_YET_OPEN: p = _T("�f�[�^�x�[�X���I�[�v������Ă��܂���B");break;
	case STATUS_TABLENAME_NOTFOUND: p = _T("�w�肵���e�[�u������������܂���B");break;
	case STATUS_DIFFERENT_DBVERSION: p = _T("�f�[�^�x�[�X�̃o�[�W�������Ⴄ���A�j�����Ă��܂��B");break;
	case STATUS_DUPLICATE_FIELDNAME: p = _T("�t�B�[���h�����d�����Ă��܂��B");break;
	case STATUS_INVALID_TABLE_IDX: p = _T("��`����Ȃ��e�[�u���ԍ��ł��B");break;
	case STATUS_AUTH_DENIED: p = _T("���[�U�[���܂��̓p�X���[�h���s���ł��B");break;
	case STATUS_TOO_MANY_FIELDS: p = _T("�Ǘ��\�ȃt�B�[���h���𒴂��Ă��܂��B");break;
	case STATUS_FILTERSTRING_ERROR: p = _T("�t�B���^������Ɍ�肪����܂��B");break;
	case STATUS_INVALID_FIELDLENGTH: p = _T("�t�B�[���h�����s���ł��B");break;
	case STATUS_INVALID_KEYTYPE: p = _T("�g�p�ł��Ȃ��L�[�^�C�v���w�肳��Ă��܂��B");break;
	case STATUS_LVAR_NOTE_NOT_LAST: p = _T("Note�y��Lvar�^�C�v�̓t�B�[���h�̈�ԍŌ�łȂ���΂Ȃ�܂���B");break;
	case STATUS_INVALID_VARIABLETABLE: p = _T("�ϒ��e�[�u���̍Ō�̃t�B�[���h��Note Lvar varbinary�^�C�v���K�v�ł��B");break;
	case STATUS_NODEF_FOR_CONVERT: p = _T("�R���o�[�g����`������܂���B");break;
	case STATUS_TRD_NEED_VARLENGTH: p = _T("�ϒ��e�[�u���̎w�肪�K�v�ł��B");break;
	case STATUS_TOO_LONG_OWNERNAME: p = _T("�I�[�i�[�l�[�����������܂��B");break;
	case STATUS_CANT_DEL_FOR_REL: p = _T("�Q�Ɛ������̂��ߍ폜�ł��܂���B");break;
	case STATUS_NO_AUTOINC_SPACE: p = _T("AutoIncEx�̃X�y�[�X������܂���B");break;
	case STATUS_INVALID_RECLEN: p = _T("���R�[�h����`���s�����A�e�[�u�����I�[�v������Ă��܂���B");break;
	case STATUS_INVALID_FIELDVALUE: p = _T("�t�B�[���h�̒l���s���ł��B");break;
	case STATUS_INVALID_VALLEN: p = _T("�ϒ����R�[�h�̒������o�b�t�@�T�C�Y�𒴂��Ă��܂��B");break;
	case STATUS_FIELDTYPE_NOTSUPPORT: p = _T("This field type is not supported.");break;
	case STATUS_DUPPLICATE_KEYVALUE: p = _T("�L�[�l���d�����Ă��邽�ߓo�^�ł��܂���B");break;
	case STATUS_REQUESTER_DEACTIVE: p = _T("�f�[�^�x�[�X�G���W�������[�h�ł��܂���B\r\nTerminal Service�y�сAWindowsXP�̃��[�U�[�؂�ւ�") _T
			("�ɂ����āA�����ɑ����̃��[�U�[�����p����ɂ́APervasive.SQL 2000i Server�ȏ�̃C���X�g�[�����K�v�ł��B");break;
	case STATUS_ACCESS_DENIED: p = _T("�X�V�̂��߂̃A�N�Z�X��������܂���B\r\n�t�@�C���̓��[�h�I�����[���p�X���[�h������������܂���B");break;
	case STATUS_CANT_CREATE:p = _T("�f�[�^�x�[�X�̍쐬�Ɏ��s���܂����B���Ƀf�[�^�x�[�X�����݂��Ă��Ȃ����m�F���Ă��������B");break;
	default:
		_stprintf_s(buf, 256, _T("�f�[�^�x�[�X�I�y���[�V�����ŃG���[���������܂����B\r\n�G���[�ԍ��� %d \r\n �����𒆎~���܂��B"), errorCode);
	}
	if (p)
	{
		_tcsncpy(buf, p, size);
		buf[size -1] = 0x00;
	}
	return buf;
}

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs
