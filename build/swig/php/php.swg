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
      exception handler
=============================================== */
%exception {
  try {
    $action
  } catch (bzs::rtl::exception& e) {
    SWIG_exception(SWIG_RuntimeError, (* bzs::rtl::getMsg(e)).c_str());
  } catch (std::exception &e) {
    SWIG_exception(SWIG_RuntimeError, e.what());
  }
}


/* ===============================================
  General settings for PHP
=============================================== */
%{
#ifdef SWIGPHP
#undef realloc
#endif
%}

// SWIG PHP can't convert String to c++ wchar_t so define _TCHAR as char.
#ifdef _UNICODE
  #undef _UNICODE
#endif
#ifdef UNICODE
  #undef UNICODE
#endif
typedef char  _TCHAR;
#define _T(x) x

// define strtoll and strtoull for Visual Studio 2008 or lesser
%{
#if _MSC_VER
#define strtoll _strtoi64
#define strtoull _strtoui64
#endif
%}

// phpinfo
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

// General typemap
%apply short { char_td }

// typemaps for encoding support
%typemap(argout) (const char* u8, int u8size, char* mbc, int mbcsize) {
  ZVAL_STRING(return_value, (char *)$3, 1);
}
%typemap(argout) (const char* mbc, int mbcsize, char* u8, int u8size) {
  ZVAL_STRING(return_value, (char *)$3, 1);
}

// const TCHAR* fast convert
%typemap(in) const _TCHAR* {
  if ((*$input)->type != IS_NULL) {
     convert_to_string_ex($input);
     $1 = (_TCHAR *) Z_STRVAL_PP($input);
  }
}

// const TCHAR* fast check
%typemap(typecheck,precedence=SWIG_TYPECHECK_STRING) const _TCHAR* {
  $1 = 1;
}


/* ===============================================
  ignore / rename functions
=============================================== */
// * bzs/db/protocol/tdap/btrDate.h *
%ignore bzs::db::protocol::tdap::btrTimeStamp::btrTimeStamp(const wchar_t *);   // use char*
%ignore bzs::db::protocol::tdap::btrTimeStamp::toString(wchar_t *);             // use char*
%ignore bzs::db::protocol::tdap::btrTimeStamp::fromString(const wchar_t *);     // use char*
%ignore bzs::db::protocol::tdap::atobtrd(const wchar_t *);                  // use char*
%ignore bzs::db::protocol::tdap::atobtrt(const wchar_t *);                  // use char*
%ignore bzs::db::protocol::tdap::btrdtoa(int, wchar_t *);                   // use char*
%ignore bzs::db::protocol::tdap::btrdtoa(int, wchar_t *, bool);             // use char*
%ignore bzs::db::protocol::tdap::btrdtoa(const btrDate &, wchar_t *);       // use char*
%ignore bzs::db::protocol::tdap::btrdtoa(const btrDate &, wchar_t *, bool); // use char*
%ignore bzs::db::protocol::tdap::btrttoa(int, wchar_t *);                   // use char*
%ignore bzs::db::protocol::tdap::btrttoa(int, wchar_t *, bool);             // use char*
%ignore bzs::db::protocol::tdap::btrttoa(const btrTime &, wchar_t *);       // use char*
%ignore bzs::db::protocol::tdap::btrttoa(const btrTime &, wchar_t *, bool); // use char*

// * bzs/db/protocol/tdap/client/database.h *
%ignore bzs::db::protocol::tdap::client::database::onCopyData;
%ignore bzs::db::protocol::tdap::client::database::setOnCopyData;
%ignore bzs::db::protocol::tdap::client::database::onDeleteRecord;
%ignore bzs::db::protocol::tdap::client::database::setOnDeleteRecord;

// * bzs/db/protocol/tdap/client/field.h *
%ignore bzs::db::protocol::tdap::client::fielddefs::operator[] (const std::_tstring&) const;
%rename(getFielddef) bzs::db::protocol::tdap::client::fielddefs::operator[] (int index) const;
%rename(getFielddef) bzs::db::protocol::tdap::client::fielddefs::operator[] (const _TCHAR* name) const;
%ignore bzs::db::protocol::tdap::client::field::operator=(const char*);
%ignore bzs::db::protocol::tdap::client::field::operator=(const std::string&);
%ignore bzs::db::protocol::tdap::client::field::operator=(int);
%ignore bzs::db::protocol::tdap::client::field::operator=(float);
%rename(setFV) bzs::db::protocol::tdap::client::field::operator=(const _TCHAR*);
%rename(setFV) bzs::db::protocol::tdap::client::field::operator=(__int64);
%rename(setFV) bzs::db::protocol::tdap::client::field::operator=(double);
%rename(setFV) bzs::db::protocol::tdap::client::field::setBin;
%rename(__clone) bzs::db::protocol::tdap::client::fielddefs::clone;
  // getBin typemap
%typemap(in, numinputs=0) (uint_td & size) (uint_td temp) {
  $1 = &temp;
}
%typemap(argout) (uint_td & size) {
  ZVAL_STRINGL($result, (char *)result, *$1, 1);
}

// * bzs/db/protocol/tdap/client/fields.h *
%ignore bzs::db::protocol::tdap::client::fieldsBase::operator[](const std::_tstring&) const;
%rename(getField) bzs::db::protocol::tdap::client::fieldsBase::operator[] (short index) const;
%rename(getField) bzs::db::protocol::tdap::client::fieldsBase::operator[] (const _TCHAR* name) const;

// * bzs/db/protocol/tdap/client/groupQuery.h *
%rename(__clone) bzs::db::protocol::tdap::client::groupFuncBase::clone;
%rename(__clone) bzs::db::protocol::tdap::client::avg::clone;
%rename(__clone) bzs::db::protocol::tdap::client::count::clone;
%rename(__clone) bzs::db::protocol::tdap::client::max::clone;
%rename(__clone) bzs::db::protocol::tdap::client::min::clone;
%rename(__clone) bzs::db::protocol::tdap::client::sum::clone;
%rename(getFieldName) bzs::db::protocol::tdap::client::fieldNames::operator[] (int index) const;

// * bzs/db/protocol/tdap/client/nsDatabase.h *
%rename(__clone) bzs::db::protocol::tdap::client::nsdatabase::clone;

// * bzs/db/protocol/tdap/client/pooledDatabaseManager.h *
%rename(c_use) bzs::db::protocol::tdap::client::pooledDbManager::use(const connectParams* param = NULL);

// * bzs/db/protocol/tdap/client/recordset.h *
%rename(__clone) bzs::db::protocol::tdap::client::recordset::clone;
%ignore bzs::db::protocol::tdap::client::recordset::first;
%ignore bzs::db::protocol::tdap::client::recordset::last;
%rename(getRecord) bzs::db::protocol::tdap::client::recordset::operator[] (size_t index) const;

// * bzs/db/protocol/tdap/client/table.h *
%ignore bzs::db::protocol::tdap::client::table::setFV(short, float); // use double
%ignore bzs::db::protocol::tdap::client::table::setFV(short, unsigned char);  // use __int64
%ignore bzs::db::protocol::tdap::client::table::setFV(short, short);          // use __int64
%ignore bzs::db::protocol::tdap::client::table::setFV(short, int);            // use __int64
%ignore bzs::db::protocol::tdap::client::table::setFV(const _TCHAR *, float); // use double
%ignore bzs::db::protocol::tdap::client::table::setFV(const _TCHAR *, unsigned char); // use __int64
%ignore bzs::db::protocol::tdap::client::table::setFV(const _TCHAR *, short);         // use __int64
%ignore bzs::db::protocol::tdap::client::table::setFV(const _TCHAR *, int);           // use __int64
%ignore bzs::db::protocol::tdap::client::table::setFVW;     // use setFV
%ignore bzs::db::protocol::tdap::client::table::getFVWstr;  // use getFVstr
  // define setFVbin
%ignore bzs::db::protocol::tdap::client::table::setFV(short, const void *, uint_td);
%ignore bzs::db::protocol::tdap::client::table::setFV(const _TCHAR*, const void *, uint_td);
%extend bzs::db::protocol::tdap::client::table {
  void setFV(short index, const char * data, uint_td size) {
    return self->setFV(index, (void const *) data, size);
  }
  void setFV(const _TCHAR *fieldName, const char * data, uint_td size) {
    return self->setFV(fieldName, (void const *) data, size);
  }
};
  // fix getFVbin
%typemap(in, numinputs=0) (uint_td & table_getFVbin_size) (uint_td temp) {
  $1 = &temp;
}
%typemap(argout) (uint_td & table_getFVbin_size) {
  ZVAL_STRINGL($result, (char *)result, *$1, 1);
}
%extend bzs::db::protocol::tdap::client::table {
  void* getFVbin(short index, uint_td& table_getFVbin_size)
  {
    return self->getFVbin(index, table_getFVbin_size);
  }
  void* getFVbin(const _TCHAR *fieldName, uint_td& table_getFVbin_size)
  {
    return self->getFVbin(fieldName, table_getFVbin_size);
  }
};
%ignore bzs::db::protocol::tdap::client::table::getFVbin(short, uint_td&);
%ignore bzs::db::protocol::tdap::client::table::getFVbin(const _TCHAR *, uint_td&);

// * bzs/db/protocol/tdap/client/trdboostapi.h *
%rename(c_use) bzs::db::protocol::tdap::client::idatabaseManager::use(const connectParams* param = NULL);


/* ===============================================
  add code to init section
=============================================== */
%init %{
    bzs::db::protocol::tdap::client::nsdatabase::setCheckTablePtr(true);
%}
#endif // =============== For PHP ===============
