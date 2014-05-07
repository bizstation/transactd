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
_TCHAR* getErrorMessageLocale(int errorCode, _TCHAR* buf, size_t size)
{
    const _TCHAR* p = 0x00;
	switch (errorCode)
    {
	case STATUS_LOCK_ERROR:
	case STATUS_FILE_LOCKED:
    case STATUS_CANNOT_LOCK_TABLE:p = _T("This record is locked already by other user");break;
    case STATUS_CHANGE_CONFLICT:p = _T("Change of this record is conflicted with other user's"); break;
    case STATUS_TABLE_YET_OPEN: p = _T("This table has not been opened yet");break;
    case STATUS_DURING_TRANSACTION: p = _T("This table is still during a transaction.It cannot close");break;
    case STATUS_NO_ACR_UPDATE_DELETE: p =  _T("Does not have access rights of Updates or Deletes");break;
    case STATUS_NO_ACR_INSERT: p =  _T("Does not have access rights of Inserts");break;
    case STATUS_NO_ACR_READ: p =  _T("Does not have access rights of Reads");break;
    case STATUS_CANT_ALLOC_MEMORY: p =  _T("Memory was not able to be allocated");break;
    case STATUS_USE_KEYFIELD: p =  _T("This field is used by the key,that  cannot be deleted");break;
    case STATUS_TOO_MANY_TABLES: p =  _T("There are too many tables");break;
    case STATUS_INVARID_PRM_KEY_NUM: p =  _T("Primarykey number is invalid");break;
    case STATUS_INVARID_PNT_KEY_NUM: p =  _T("Parentkey number is invalid");break;
    case STATUS_INVARID_REP_KEY_NUM: p =  _T("Replicakey number is invalid");break;
    case STATUS_INVARID_FIELD_IDX: p =  _T("The field index is out of range");break;
    case STATUS_ALREADY_DELETED: p =  _T("This item is deleted already");break;
    case STATUS_LMITS_MAX_TABLES: p =  _T("It is over the maximum of the table which can be opened");break;
    case STATUS_DB_YET_OPEN: p =  _T("This database has not been opened yet");break;
    case STATUS_TABLENAME_NOTFOUND: p =  _T("The specified table name is not found");break;
    case STATUS_DIFFERENT_DBVERSION: p =  _T("The database version was different or it has damaged");break;
    case STATUS_DUPLICATE_FIELDNAME: p =  _T("Illegal duplicate field name");break;
    case STATUS_INVALID_TABLE_IDX: p =  _T("The table index is out of range");break;
    case STATUS_AUTH_DENIED: p =  _T("The user name or The password is invalid.");break;
    case STATUS_TOO_MANY_FIELDS: p =  _T("There are too many fields");break;
    case STATUS_FILTERSTRING_ERROR: p =  _T("The filter character string has an error");break;
    case STATUS_INVALID_FIELDLENGTH: p =  _T("Field length is out of range");break;
    case STATUS_INVALID_KEYTYPE: p =  _T("The specified key type cannot be used");break;
    case STATUS_LVAR_NOTE_NOT_LAST: p =  _T("If Note or a Lvar type is not the last of a record, it will not become");break;
    case STATUS_NODEF_FOR_CONVERT: p =  _T("There is no definition of the origin to convert");break;
	case STATUS_TRD_NEED_VARLENGTH: p =  _T("A variable-length table needs to be specified");break;
    case STATUS_TOO_LONG_OWNERNAME: p =  _T("The owner name is too long");break;
    case STATUS_CANT_DEL_FOR_REL: p =  _T("It cannot delete because of relationship");break;
    case STATUS_NO_AUTOINC_SPACE: p =  _T("There is no space of the auto increment number");break;
    case STATUS_INVALID_RECLEN: p =  _T("It is not opened table or invalid the record length definition");break;
    case STATUS_INVALID_FIELDVALUE: p =  _T("The field value is not right");break;
    case STATUS_INVALID_VALLEN: p =  _T("The length of the variable-length record is over buffer size.");break;
    case STATUS_FIELDTYPE_NOTSUPPORT: p =  _T("this field type is not supported.");break;
    case STATUS_DUPPLICATE_KEYVALUE: p =  _T("Illegal duplicate key value");break;
    case STATUS_REQUESTER_DEACTIVE:p =  _T("Client database engine cannot be loaded");break;
    case STATUS_ACCESS_DENIED: p =  _T("Does not have access rights of changes\nThe table is opened read-only or its password is not right. ");break;
    case STATUS_CANT_CREATE:p = _T("Cannot create databse. Please check the database exists already.") ;break;
    default:
        _stprintf_s(buf, size, _T("The error occurred by database operation. \nThe error code is %d.\nProcessing is stopped."), errorCode);
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
