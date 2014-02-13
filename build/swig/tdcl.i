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
/* ===============================================
      Settings specific to each languages.
=============================================== */
#if defined(SWIGRUBY)
  %include "ruby/ruby.swg"
#elif defined(SWIGPHP)
  %include "php/php.swg"
#endif

%module transactd

%include typemaps.i
%include std_string.i
%include wchar.i


/* ===============================================
      HEADERS (for cpp compilation)
=============================================== */
%{
#ifdef SWIGPHP
#undef realloc
#endif
#include <bzs/env/crosscompile.h>
#include <bzs/env/compiler.h>
#include <bzs/db/protocol/tdap/btrDate.h>
#include <bzs/db/blobStructs.h>
#include <bzs/db/protocol/tdap/mysql/characterset.h>
#include <bzs/db/transactd/connectionRecord.h>
%}
#ifndef SWIGWIN
%{
#include <linux/linuxTypes.h>
#include <linux/charsetConvert.h>
#include <bzs/env/mbcswchrLinux.h>
%}
#endif
%{
#include <bzs/env/tstring.h>
#include <bzs/rtl/benchmark.h>
#include <bzs/rtl/datetime.h>
#include <bzs/rtl/stringBuffers.h>
#include <bzs/rtl/strtrim.h>
#include <bzs/db/protocol/tdap/btrDate.h>
#include <bzs/db/protocol/tdap/myDateTime.cpp>
#include <bzs/db/protocol/tdap/client/sharedData.h>
#include <bzs/db/protocol/tdap/tdapcapi.h>
#include <bzs/db/protocol/tdap/tdapSchema.h>
#include <bzs/db/protocol/tdap/client/nsTable.h>
#include <bzs/db/protocol/tdap/client/connMgr.h>
#include <bzs/db/protocol/tdap/client/bulkInsert.h>
#include <bzs/db/protocol/tdap/client/dbDef.h>
#include <bzs/db/protocol/tdap/client/table.h>
#include <bzs/db/protocol/tdap/client/nsDatabase.h>
#include <bzs/db/protocol/tdap/client/database.h>
#include <bzs/db/protocol/tdap/mysql/characterset.h>

using namespace bzs::db::protocol::tdap;
using namespace bzs::db::protocol::tdap::client;
%}


/* ===============================================
      Ignore section
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
// overwrote functions
%ignore bzs::db::protocol::tdap::client::nsdatabase::createTable;
%ignore bzs::db::protocol::tdap::client::nsdatabase::getBtrVersion(btrVersions*, uchar_td*);
// ignore members used BizStation internal only
%ignore bzs::db::protocol::tdap::fielddef::chainChar;
%ignore bzs::db::protocol::tdap::fielddef::setChainChar;
%ignore bzs::db::protocol::tdap::fielddef_t::defValue;
%ignore bzs::db::protocol::tdap::fielddef_t::defViewWidth;
%ignore bzs::db::protocol::tdap::fielddef_t::enableFlags;
%ignore bzs::db::protocol::tdap::fielddef_t::filterId;
%ignore bzs::db::protocol::tdap::fielddef_t::filterKeynum;
%ignore bzs::db::protocol::tdap::fielddef_t::lookDBNum;
%ignore bzs::db::protocol::tdap::fielddef_t::lookField;
%ignore bzs::db::protocol::tdap::fielddef_t::lookFields;
%ignore bzs::db::protocol::tdap::fielddef_t::lookTable;
%ignore bzs::db::protocol::tdap::fielddef_t::userOption;
%ignore bzs::db::protocol::tdap::fielddef_t::viewNum;
%ignore bzs::db::protocol::tdap::fielddef_t::viewWidth;
%ignore bzs::db::protocol::tdap::tabledef::autoIncExSpace;
%ignore bzs::db::protocol::tdap::tabledef::convertFileNum;
%ignore bzs::db::protocol::tdap::tabledef::iconIndex;
%ignore bzs::db::protocol::tdap::tabledef::iconIndex2;
%ignore bzs::db::protocol::tdap::tabledef::iconIndex3;
%ignore bzs::db::protocol::tdap::tabledef::optionFlags;
%ignore bzs::db::protocol::tdap::tabledef::parentKeyNum;
%ignore bzs::db::protocol::tdap::tabledef::replicaKeyNum;
%ignore bzs::db::protocol::tdap::tabledef::treeIndex;
%ignore bzs::db::protocol::tdap::tabledef::filler0;
%ignore bzs::db::protocol::tdap::tabledef::reserved;
%ignore bzs::db::protocol::tdap::client::dbdef::fieldNumByViewNum;
// ignore operator=
%ignore bzs::db::protocol::tdap::client::nsdatabase::operator=;
%ignore bzs::db::protocol::tdap::client::database::operator=;
// ignore internal structures / functions
%ignore bzs::rtl::benchmark::report;
// ignore methods that will be replaced other methods
%ignore bzs::db::protocol::tdap::client::table::keyValueDescription(_TCHAR* buf, int bufsize);
%ignore bzs::db::protocol::tdap::client::queryBase::addField;
%ignore bzs::db::protocol::tdap::client::queryBase::addLogic;
%ignore bzs::db::protocol::tdap::client::queryBase::addSeekKeyValue;
// ignore queryBase destructor. use release instead of it.
%ignore bzs::db::protocol::tdap::client::queryBase::~queryBase;
%newobject bzs::db::protocol::tdap::client::queryBase::create;
%delobject bzs::db::protocol::tdap::client::queryBase::release;


/* ===============================================
      Rename section
=============================================== */
// duplicate name (class method and instance method)
%rename(tdapLastErr) bzs::db::protocol::tdap::client::nstable::tdapErr(HWND, _TCHAR*);


/* ===============================================
      Memory management
=============================================== */
// add new/delobject define for database
%newobject bzs::db::protocol::tdap::client::database::createObject;
%delobject bzs::db::protocol::tdap::client::database::release;
%ignore bzs::db::protocol::tdap::client::database::destroy;
%ignore bzs::db::protocol::tdap::client::database::release;

// add new/delobject define for table
%newobject bzs::db::protocol::tdap::client::database::openTable;
%delobject bzs::db::protocol::tdap::client::table::release;
%ignore bzs::db::protocol::tdap::client::nstable::release;
%ignore bzs::db::protocol::tdap::client::table::release;


/* ===============================================
      type defines
=============================================== */
%apply unsigned long long { unsigned __int64 }
%apply long long { __int64 }


/* ===============================================
  typemap and extend for table::keyValueDescription
=============================================== */
%typemap(in,numinputs=0) (_TCHAR* keyValueDescription_buf) (_TCHAR tmpbuf[1024 * 8]) {
  $1 = tmpbuf;
}
%extend bzs::db::protocol::tdap::client::table {
  const _TCHAR* keyValueDescription(_TCHAR* keyValueDescription_buf) {
    self->keyValueDescription(keyValueDescription_buf, 1024 * 8);
    return keyValueDescription_buf;
  }
};


/* ===============================================
      external symbols
=============================================== */
%include bzs/env/compiler.h
#ifdef SWIGWIN
#define CP_ACP  0
#define CP_UTF8 65001
#else
%include bzs/env/mbcswchrLinux.h
#endif
#undef pragma_pack1
#define pragma_pack1
#undef pragma_pop
#define pragma_pop
%include bzs/db/protocol/tdap/tdapcapi.h
%include bzs/db/protocol/tdap/tdapSchema.h
%include bzs/db/protocol/tdap/client/nsTable.h
%include bzs/db/protocol/tdap/client/dbDef.h
%include bzs/db/protocol/tdap/client/table.h
%include bzs/db/protocol/tdap/client/nsDatabase.h
%include bzs/db/protocol/tdap/client/database.h
%include bzs/rtl/benchmark.h
%include bzs/db/protocol/tdap/mysql/characterset.h
// typemap for btrTimeStamp::toString/btrdtoa/btrttoa/btrstoa --
%typemap(in,numinputs=0) (char * retbuf) (char tmpbuf[255]) { $1=tmpbuf; }
%include bzs/db/protocol/tdap/btrDate.h
%clear char * retbuf;
// clear typemap for typemap for btrTimeStamp::toString/btrdtoa/btrttoa/btrstoa --


/* ===============================================
      expand queryBase class
  * Replace functions for convenience
=============================================== */
%extend bzs::db::protocol::tdap::client::queryBase {
  queryBase* select(const _TCHAR* value1,
    const _TCHAR* value2 = NULL, const _TCHAR* value3 = NULL, const _TCHAR* value4 = NULL,
    const _TCHAR* value5 = NULL, const _TCHAR* value6 = NULL, const _TCHAR* value7 = NULL,
    const _TCHAR* value8 = NULL, const _TCHAR* value9 = NULL, const _TCHAR* value10 = NULL)
  {
    self->addField(value1);
    if (value2 != NULL)
      self->addField(value2);
    if (value3 != NULL)
      self->addField(value3);
    if (value4 != NULL)
      self->addField(value4);
    if (value5 != NULL)
      self->addField(value5);
    if (value6 != NULL)
      self->addField(value6);
    if (value7 != NULL)
      self->addField(value7);
    if (value8 != NULL)
      self->addField(value8);
    if (value9 != NULL)
      self->addField(value9);
    if (value10 != NULL)
      self->addField(value10);
    return self;
  }
  queryBase* where(const _TCHAR* field, const _TCHAR* op, const _TCHAR* val)
  {
    self->addLogic(field, op, val);
    return self;
  }
  queryBase* And(const _TCHAR* field, const _TCHAR* op, const _TCHAR* val)
  {
    self->addLogic(_T("and") ,field, op, val);
    return self;
  }
  queryBase* Or(const _TCHAR* field, const _TCHAR* op, const _TCHAR* val)
  {
    self->addLogic(_T("or") ,field, op, val);
    return self;
  }
  queryBase* In(const _TCHAR* value1,
    const _TCHAR* value2 = NULL, const _TCHAR* value3 = NULL, const _TCHAR* value4 = NULL,
    const _TCHAR* value5 = NULL, const _TCHAR* value6 = NULL, const _TCHAR* value7 = NULL,
    const _TCHAR* value8 = NULL, const _TCHAR* value9 = NULL, const _TCHAR* value10 = NULL)
  {
    self->addSeekKeyValue(value1, false);
    if (value2 != NULL)
      self->addSeekKeyValue(value2, false);
    if (value3 != NULL)
      self->addSeekKeyValue(value3, false);
    if (value4 != NULL)
      self->addSeekKeyValue(value4, false);
    if (value5 != NULL)
      self->addSeekKeyValue(value5, false);
    if (value6 != NULL)
      self->addSeekKeyValue(value6, false);
    if (value7 != NULL)
      self->addSeekKeyValue(value7, false);
    if (value8 != NULL)
      self->addSeekKeyValue(value8, false);
    if (value9 != NULL)
      self->addSeekKeyValue(value9, false);
    if (value10 != NULL)
      self->addSeekKeyValue(value10, false);
    return self;
  }
  queryBase* addInValue(const _TCHAR* value, bool reset = false)
  {
    self->addSeekKeyValue(value, reset);
    return self;
  }
};


/* ===============================================
      expand some classes
  * many languages can not use pointer as array so add helper functions.
=============================================== */
%extend bzs::db::protocol::tdap::keydef {
  keySegment* segment(const int index)
  {
    return &(self->segments[index]);
  }
};
%extend bzs::db::protocol::tdap::tabledef {
  fielddef* fieldDef(const int index)
  {
    return &(self->fieldDefs[index]);
  }
  keydef* keyDef(const int index)
  {
    return &(self->keyDefs[index]);
  }
};
%extend bzs::db::protocol::tdap::btrVersions {
  btrVersion* version(const int index) {
    return &(self->versions[index]);
  }
};
