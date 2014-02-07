/*=================================================================
   Copyright (C) 2013 BizStation Corp All rights reserved.

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
#if defined(SWIGPHP)  // ======= For PHP =======

/* ===============================================
  ignore functions
=============================================== */
// functions used by internal C++
%ignore bzs::db::protocol::tdap::client::nsdatabase::btrvFunc;
%ignore bzs::db::protocol::tdap::fileSpec;
%ignore bzs::db::protocol::tdap::keySpec;
%ignore bzs::db::protocol::tdap::fielddef::dataLen;
%ignore bzs::db::protocol::tdap::fielddef::keyCopy;
%ignore bzs::db::protocol::tdap::fielddef::keyData;
%ignore bzs::db::protocol::tdap::fielddef::keyDataLen;
%ignore bzs::db::protocol::tdap::fielddef::blobDataPtr;
%ignore bzs::db::protocol::tdap::fielddef::blobDataLen;
// original callback
%ignore bzs::db::protocol::tdap::client::database::onCopyData;
%ignore bzs::db::protocol::tdap::client::database::setOnCopyData;
%ignore bzs::db::protocol::tdap::client::database::onDeleteRecord;
%ignore bzs::db::protocol::tdap::client::database::setOnDeleteRecord;
// overwrote functions
%ignore bzs::db::protocol::tdap::client::nsdatabase::createTable;
%ignore bzs::db::protocol::tdap::client::nsdatabase::getBtrVersion(btrVersions*, uchar_td*);
// overloaded functions
%ignore bzs::db::protocol::tdap::client::table::setFV(short, float); // use double instead of this
%ignore bzs::db::protocol::tdap::client::table::setFV(short, unsigned char);  // use __int64 instead of this
%ignore bzs::db::protocol::tdap::client::table::setFV(short, short);          // use __int64 instead of this
%ignore bzs::db::protocol::tdap::client::table::setFV(short, int);            // use __int64 instead of this
%ignore bzs::db::protocol::tdap::client::table::setFV(const _TCHAR *, float); // use double instead of this
%ignore bzs::db::protocol::tdap::client::table::setFV(const _TCHAR *, unsigned char); // use __int64 instead of this
%ignore bzs::db::protocol::tdap::client::table::setFV(const _TCHAR *, short);         // use __int64 instead of this
%ignore bzs::db::protocol::tdap::client::table::setFV(const _TCHAR *, int);           // use __int64 instead of this
%ignore bzs::db::protocol::tdap::client::table::setFVW;     // use setFV instead of this
%ignore bzs::db::protocol::tdap::client::table::getFVWstr;  // use getFVstr instead of this
%ignore bzs::db::protocol::tdap::btrTimeStamp::btrTimeStamp(const wchar_t *);   // use char* instead of this
%ignore bzs::db::protocol::tdap::btrTimeStamp::toString(wchar_t *);             // use char* instead of this
%ignore bzs::db::protocol::tdap::btrTimeStamp::fromString(const wchar_t *);     // use char* instead of this
%ignore bzs::db::protocol::tdap::atobtrd(const wchar_t *);                  // use char* instead of this
%ignore bzs::db::protocol::tdap::atobtrt(const wchar_t *);                  // use char* instead of this
%ignore bzs::db::protocol::tdap::btrdtoa(int, wchar_t *);                   // use char* instead of this
%ignore bzs::db::protocol::tdap::btrdtoa(int, wchar_t *, bool);             // use char* instead of this
%ignore bzs::db::protocol::tdap::btrdtoa(const btrDate &, wchar_t *);       // use char* instead of this
%ignore bzs::db::protocol::tdap::btrdtoa(const btrDate &, wchar_t *, bool); // use char* instead of this
%ignore bzs::db::protocol::tdap::btrttoa(int, wchar_t *);                   // use char* instead of this
%ignore bzs::db::protocol::tdap::btrttoa(int, wchar_t *, bool);             // use char* instead of this
%ignore bzs::db::protocol::tdap::btrttoa(const btrTime &, wchar_t *);       // use char* instead of this
%ignore bzs::db::protocol::tdap::btrttoa(const btrTime &, wchar_t *, bool); // use char* instead of this


/* ===============================================
  Rename functions
=============================================== */
// Rename function which has keyword of PHP as its name.
%rename(andWhere) bzs::db::protocol::tdap::client::queryBase::And;
%rename(orWhere) bzs::db::protocol::tdap::client::queryBase::Or;


/* ===============================================
  set execCodePage on createObject
=============================================== */
%ignore bzs::db::protocol::tdap::client::database::create();
%extend bzs::db::protocol::tdap::client::database {
  static bzs::db::protocol::tdap::client::database* createObject()
  {
    bzs::db::protocol::tdap::client::nsdatabase::setExecCodePage(CP_UTF8);
    return bzs::db::protocol::tdap::client::database::create();
  }
};
%newobject bzs::db::protocol::tdap::client::database::createObject;
%delobject bzs::db::protocol::tdap::client::database::release;


/* ===============================================
  SWIG PHP can't convert String to c++ wchar_t
  so define _TCHAR as char.
=============================================== */
#ifdef _UNICODE
  #undef _UNICODE
#endif
#ifdef UNICODE
  #undef UNICODE
#endif
typedef char  _TCHAR;
#define _T(x) x


/* ===============================================
  define strtoll and strtoull
    for Visual Studio 2008 or lesser
=============================================== */
%{
#if _MSC_VER
#define strtoll _strtoi64
#define strtoull _strtoui64
#endif
%}


/* ===============================================
  typemap
=============================================== */
%apply short { char_td }
%typemap(argout) (const char* u8, int u8size, char* mbc, int mbcsize) {
  ZVAL_STRING(return_value, (char *)$3, 1);
}
%typemap(argout) (const char* mbc, int mbcsize, char* u8, int u8size) {
  ZVAL_STRING(return_value, (char *)$3, 1);
}


/* ===============================================
  setFV / getFV for binary data
=============================================== */
%ignore bzs::db::protocol::tdap::client::table::setFV(short, const void *, uint_td);
%extend bzs::db::protocol::tdap::client::table {
  void setFV(short index, const char * data, uint_td size) {
    return self->setFV(index, (void const *) data, size);
  }
  void setFV(const _TCHAR *fieldName, const char * data, uint_td size) {
    return self->setFV(fieldName, (void const *) data, size);
  }
};
%typemap(in, numinputs=0) (uint_td & size) (uint_td temp) {
  $1 = &temp;
}
%typemap(argout) (uint_td & size) {
  ZVAL_STRINGL($result, (char *)result, *$1, 1);
}


/* ===============================================
  phpinfo
=============================================== */
#ifndef SWIGWIN
%{
  #define TRANSACTD_PHP_REQUIRED_MODULE_PREFIX "lib"
%}
#else
%{
  #define TRANSACTD_PHP_REQUIRED_MODULE_PREFIX ""
%}
#endif
%pragma(php) phpinfo="
  php_info_print_table_start();
  php_info_print_table_row(2, \"Version\", CPP_INTERFACE_VER_MAJOR \".\" CPP_INTERFACE_VER_MINOR \".\" CPP_INTERFACE_VER_RELEASE);
  php_info_print_table_row(2, \"Required Module\", TRANSACTD_PHP_REQUIRED_MODULE_PREFIX TDCLC_LIBNAME);
  php_info_print_table_end();
"

#endif // =============== For PHP ===============
