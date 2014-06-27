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
#if defined(SWIGRUBY)  // ======= For RUBY =======

%{
#if defined(__MINGW32__) && defined(__GNUC__)
#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ <= 5)
#define _SYS_TIME_H_
#else
#define _GETTIMEOFDAY_DEFINED
#endif
#endif
%}

/* ===============================================
  Suppress warning
=============================================== */
%warnfilter(
  SWIGWARN_RUBY_WRONG_NAME
);


/* ===============================================
  ignore functions
=============================================== */
//  original callback
%ignore bzs::db::protocol::tdap::client::database::onCopyData;
%ignore bzs::db::protocol::tdap::client::database::setOnCopyData;
%ignore bzs::db::protocol::tdap::client::database::onDeleteRecord;
%ignore bzs::db::protocol::tdap::client::database::setOnDeleteRecord;


/* ===============================================
  Rename functions
=============================================== */
// Rename function which has keyword of Ruby as its name.
%rename(andWhere) bzs::db::protocol::tdap::client::queryBase::And;
%rename(orWhere) bzs::db::protocol::tdap::client::queryBase::Or;


/* ===============================================
  Ruby can't convert String to c++ wchar_t
  so define _TCHAR as char.
=============================================== */
#ifdef _UNICODE
# undef _UNICODE
#endif
#ifdef UNICODE
# undef UNICODE
#endif
typedef char  _TCHAR;
#define _T(x) x


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
  const char * getFVbin(short index, uint_td& size)
  {
    return (const char *)(self->getFVbin(index, size));
  }
  const char * getFVbin(const _TCHAR *fieldName, uint_td& size)
  {
    return (const char *)(self->getFVbin(fieldName, size));
  }
};
%typemap(in, numinputs=0) (uint_td & size) (uint_td temp) {
  $1 = &temp;
}
%typemap(argout) (uint_td & size) {
  $result = rb_str_new((const char *)result, *$1);
}
%ignore bzs::db::protocol::tdap::client::table::getFVbin(short, uint_td&);
%ignore bzs::db::protocol::tdap::client::table::getFVbin(const _TCHAR *, uint_td&);


/* ===============================================
  add encoding support for ruby 1.9
=============================================== */
%{
#include "ruby/encoding.h"
#undef stat
#include <bzs/env/tstring.h>

rb_encoding* rb_enc_find_from_codepage(int codePage)
{
  if (codePage <= 0 || codePage >= 10000000)
    codePage = GetACP();
  char cpname[10];
  sprintf(cpname, "CP%2d", codePage);
  return rb_enc_find(cpname);
}
%}

%typemap(argout) (const char* u8, int u8size, char* mbc, int mbcsize) {
  %append_output(rb_str_new($3, strlen($3)));
}
%typemap(argout) (const char* mbc, int mbcsize, char* u8, int u8size) {
  %append_output(rb_str_new($3, strlen($3)));
}

%typemap(out) const char* {
  vresult = rb_enc_str_new($1, strlen($1), rb_enc_find_from_codepage(CP_UTF8));
}

%typemap(out) char* {
  vresult = rb_enc_str_new($1, strlen($1), rb_enc_find_from_codepage(CP_UTF8));
}

#endif // =============== For RUBY ===============
