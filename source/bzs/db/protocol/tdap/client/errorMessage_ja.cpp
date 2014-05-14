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
	case STATUS_CANNOT_LOCK_TABLE: p = _T("このレコードは既に他のユーザーによりロックされています。");break;
	case STATUS_CHANGE_CONFLICT: p = _T("このレコードは今、他のユーザーにより変更されました。");break;
	case STATUS_TABLE_YET_OPEN: p = _T("このテーブルはまだ OPEN されていません。");break;
	case STATUS_DURING_TRANSACTION: p = _T("このテーブルはまだトランザクション中です。close できません。");break;
	case STATUS_NO_ACR_UPDATE_DELETE: p = _T("更新または削除アクセス権がありません。");break;
	case STATUS_NO_ACR_INSERT: p = _T("追加アクセス権がありません。");break;
	case STATUS_NO_ACR_READ: p = _T("読み取りアクセス権がありません。");break;
	case STATUS_CANT_ALLOC_MEMORY: p = _T("メモリが確保できませんでした。");break;
	case STATUS_USE_KEYFIELD: p = _T("このフィールドはキーで使用されているため削除できません。");break;
	case STATUS_TOO_MANY_TABLES: p = _T("管理可能なテーブル数を超えています。");break;
	case STATUS_INVARID_PRM_KEY_NUM: p = _T("MainKeyキー番号が不正です。");break;
	case STATUS_INVARID_PNT_KEY_NUM: p = _T("ParentKeyキー番号が不正です。");break;
	case STATUS_INVARID_REP_KEY_NUM: p = _T("ReplicaKeyキー番号が不正です。");break;
	case STATUS_INVARID_FIELD_IDX: p = _T("フィールドインデックスが有効範囲にありません。");break;
	case STATUS_ALREADY_DELETED: p = _T("このアイテムは削除されています。");break;
	case STATUS_LMITS_MAX_TABLES: p = _T("オープンできるテーブルの最大数を超えています。");break;
	case STATUS_DB_YET_OPEN: p = _T("データベースがオープンされていません。");break;
	case STATUS_TABLENAME_NOTFOUND: p = _T("指定したテーブル名が見つかりません。");break;
	case STATUS_DIFFERENT_DBVERSION: p = _T("データベースのバージョンが違うか、破損しています。");break;
	case STATUS_DUPLICATE_FIELDNAME: p = _T("フィールド名が重複しています。");break;
	case STATUS_INVALID_TABLE_IDX: p = _T("定義されないテーブル番号です。");break;
	case STATUS_AUTH_DENIED: p = _T("ユーザー名またはパスワードが不正です。");break;
	case STATUS_TOO_MANY_FIELDS: p = _T("管理可能なフィールド数を超えています。");break;
	case STATUS_FILTERSTRING_ERROR: p = _T("フィルタ文字列に誤りがあります。");break;
	case STATUS_INVALID_FIELDLENGTH: p = _T("フィールド長が不正です。");break;
	case STATUS_INVALID_KEYTYPE: p = _T("使用できないキータイプが指定されています。");break;
	case STATUS_LVAR_NOTE_NOT_LAST: p = _T("Note及びLvarタイプはフィールドの一番最後でなければなりません。");break;
	case STATUS_INVALID_VARIABLETABLE: p = _T("可変長テーブルの最後のフィールドはNote Lvar varbinaryタイプが必要です。");break;
	case STATUS_NODEF_FOR_CONVERT: p = _T("コンバート元定義がありません。");break;
	case STATUS_TRD_NEED_VARLENGTH: p = _T("可変長テーブルの指定が必要です。");break;
	case STATUS_TOO_LONG_OWNERNAME: p = _T("オーナーネームが長すぎます。");break;
	case STATUS_CANT_DEL_FOR_REL: p = _T("参照整合性のため削除できません。");break;
	case STATUS_NO_AUTOINC_SPACE: p = _T("AutoIncExのスペースがありません。");break;
	case STATUS_INVALID_RECLEN: p = _T("レコード長定義が不正か、テーブルがオープンされていません。");break;
	case STATUS_INVALID_FIELDVALUE: p = _T("フィールドの値が不正です。");break;
	case STATUS_INVALID_VALLEN: p = _T("可変長レコードの長さがバッファサイズを超えています。");break;
	case STATUS_FIELDTYPE_NOTSUPPORT: p = _T("This field type is not supported.");break;
	case STATUS_DUPPLICATE_KEYVALUE: p = _T("キー値が重複しているため登録できません。");break;
	case STATUS_REQUESTER_DEACTIVE: p = _T("データベースエンジンをロードできません。\r\nTerminal Service及び、WindowsXPのユーザー切り替え") _T
			("において、同時に多数のユーザーが利用するには、Pervasive.SQL 2000i Server以上のインストールが必要です。");break;
	case STATUS_ACCESS_DENIED: p = _T("更新のためのアクセス権がありません。\r\nファイルはリードオンリーかパスワードが正しくありません。");break;
	case STATUS_CANT_CREATE:p = _T("データベースの作成に失敗しました。既にデータベースが存在していないか確認してください。");break;
	default:
		_stprintf_s(buf, 256, _T("データベースオペレーションでエラーが発生しました。\r\nエラー番号は %d \r\n 処理を中止します。"), errorCode);
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
