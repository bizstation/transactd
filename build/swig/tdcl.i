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
%include exception.i


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
#include <bzs/db/protocol/tdap/client/field.h>
#include <bzs/db/protocol/tdap/client/fields.h>
#include <bzs/db/protocol/tdap/client/memRecord.h>
#include <bzs/db/protocol/tdap/client/memRecordset.h>
#include <bzs/db/protocol/tdap/client/trdormapi.h>
#include <bzs/db/protocol/tdap/client/groupQuery.h>

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

%ignore bzs::db::protocol::tdap::client::recordset::first;
%ignore bzs::db::protocol::tdap::client::recordset::last;
%ignore bzs::db::protocol::tdap::client::recordsetSorter;
%ignore bzs::db::protocol::tdap::client::map_orm_fdi;
%ignore bzs::db::protocol::tdap::client::multiRecordAlocatorImple;
%ignore bzs::db::protocol::tdap::client::setValue;
%ignore bzs::db::protocol::tdap::client::fieldValue;
%ignore bzs::db::protocol::tdap::client::create;
%ignore bzs::db::protocol::tdap::client::createFdi;
%ignore bzs::db::protocol::tdap::client::destroyFdi;
%ignore bzs::db::protocol::tdap::client::initFdi;

%ignore bzs::db::protocol::tdap::client::fieldShare;
%ignore bzs::db::protocol::tdap::client::autoMemory;

%ignore bzs::db::protocol::tdap::client::qlogic;
%ignore bzs::db::protocol::tdap::client::databaseManager;
%ignore bzs::db::protocol::tdap::client::query;
%ignore bzs::db::protocol::tdap::client::begin;
%ignore bzs::db::protocol::tdap::client::end;
%ignore bzs::db::protocol::tdap::client::push_back;
%ignore bzs::db::protocol::tdap::client::readBefore;
%ignore bzs::db::protocol::tdap::client::mdlsHandler;
%ignore bzs::db::protocol::tdap::client::compFunc;
%ignore bzs::db::protocol::tdap::client::sortFuncBase;
%ignore bzs::db::protocol::tdap::client::sortFunc;
%ignore bzs::db::protocol::tdap::client::sortFunctor;
%ignore bzs::db::protocol::tdap::client::sort;
%ignore bzs::db::protocol::tdap::client::mraResetter;

%ignore bzs::db::protocol::tdap::client::fields;
%ignore bzs::db::protocol::tdap::client::fieldsBase::fd;
%ignore bzs::db::protocol::tdap::client::fieldsBase::inproc_fd;
%ignore bzs::db::protocol::tdap::client::fieldsBase::operator[](const std::_tstring&) const;

%ignore bzs::db::protocol::tdap::client::memoryRecord::clear;
%ignore bzs::db::protocol::tdap::client::memoryRecord::setRecordData;
%ignore bzs::db::protocol::tdap::client::memoryRecord::ptr;
%ignore bzs::db::protocol::tdap::client::memoryRecord::memBlock;
%ignore bzs::db::protocol::tdap::client::memoryRecord::memBlockSize;
%ignore bzs::db::protocol::tdap::client::memoryRecord::copyToBuffer;
%ignore bzs::db::protocol::tdap::client::memoryRecord::copyFromBuffer;

%ignore bzs::db::protocol::tdap::client::activeTable::activeTable(databaseManager&);
%ignore bzs::db::protocol::tdap::client::activeTable::activeTable(database_ptr&);
%ignore bzs::db::protocol::tdap::client::activeTable::activeTable(databaseManager&, const _TCHAR*);
%ignore bzs::db::protocol::tdap::client::activeTable::activeTable(database_ptr&, const _TCHAR*);
%ignore bzs::db::protocol::tdap::client::activeTable::read(collection_vec_type&, queryBase&, validationFunc);
%ignore bzs::db::protocol::tdap::client::activeTable::read(collection_vec_type&, queryBase&);
%ignore bzs::db::protocol::tdap::client::activeTable<map_orm>::read(recordset&, queryBase&, validationFunc);

%ignore bzs::db::protocol::tdap::client::compByKey;
%ignore bzs::db::protocol::tdap::client::grouping_comp;

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
%rename(tdapLastErr) bzs::db::protocol::tdap::client::nstable::tdapErr(HWND, _TCHAR*);
%rename(createRecord) bzs::db::protocol::tdap::client::memoryRecord::create(fielddefs&);
%rename(Record) bzs::db::protocol::tdap::client::memoryRecord;
%rename(RecordSet) bzs::db::protocol::tdap::client::recordset;
%rename(getFielddef) bzs::db::protocol::tdap::client::fielddefs::operator[] (int index) const;
%rename(getFieldByIndex) bzs::db::protocol::tdap::client::fieldsBase::operator[](short) const;
%rename(getFieldByName)  bzs::db::protocol::tdap::client::fieldsBase::operator[](const _TCHAR*) const;


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
%include bzs/db/protocol/tdap/client/field.h
%include bzs/db/protocol/tdap/client/fields.h
%include bzs/db/protocol/tdap/client/memRecord.h
%include bzs/db/protocol/tdap/client/memRecordset.h
%include bzs/db/protocol/tdap/client/trdormapi.h
%include bzs/db/protocol/tdap/client/groupQuery.h


/* ===============================================
      expand activeTable and use template
=============================================== */
%extend bzs::db::protocol::tdap::client::activeTable<map_orm> {
  activeTable<map_orm>& _keyValue(const _TCHAR* kv0, const _TCHAR* kv1 = NULL, const _TCHAR* kv2 = NULL,
                        const _TCHAR* kv3 = NULL, const _TCHAR* kv4 = NULL, const _TCHAR* kv5 = NULL,
                        const _TCHAR* kv6 = NULL, const _TCHAR* kv7 = NULL) {
    if (kv1 == NULL) {
      return self->keyValue(kv0);
    } else if (kv2 == NULL) {
      return self->keyValue(kv0, kv1);
    } else if (kv3 == NULL) {
      return self->keyValue(kv0, kv1, kv2);
    } else if (kv4 == NULL) {
      return self->keyValue(kv0, kv1, kv2, kv3);
    } else if (kv5 == NULL) {
      return self->keyValue(kv0, kv1, kv2, kv3, kv4);
    } else if (kv6 == NULL) {
      return self->keyValue(kv0, kv1, kv2, kv3, kv4, kv5);
    } else if (kv7 == NULL) {
      return self->keyValue(kv0, kv1, kv2, kv3, kv4, kv5, kv6);
    }
    return self->keyValue(kv0, kv1, kv2, kv3, kv4, kv5, kv6, kv7);
  }
};
%template(keyValue1) bzs::db::protocol::tdap::client::activeTable::keyValue<_TCHAR*>;
%template(keyValue2) bzs::db::protocol::tdap::client::activeTable::keyValue<_TCHAR*, _TCHAR*>;
%template(keyValue3) bzs::db::protocol::tdap::client::activeTable::keyValue<_TCHAR*, _TCHAR*, _TCHAR*>;
%template(keyValue4) bzs::db::protocol::tdap::client::activeTable::keyValue<_TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*>;
%template(keyValue5) bzs::db::protocol::tdap::client::activeTable::keyValue<_TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*>;
%template(keyValue6) bzs::db::protocol::tdap::client::activeTable::keyValue<_TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*>;
%template(keyValue7) bzs::db::protocol::tdap::client::activeTable::keyValue<_TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*>;
%template(keyValue8) bzs::db::protocol::tdap::client::activeTable::keyValue<_TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*>;
%template(read_rs)     bzs::db::protocol::tdap::client::activeTable::read<recordset>;
%template(join)        bzs::db::protocol::tdap::client::activeTable::join<recordset>;
%template(outerJoin)   bzs::db::protocol::tdap::client::activeTable::outerJoin<recordset>;
%template(ActiveTable) bzs::db::protocol::tdap::client::activeTable<map_orm>;


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


/* ===============================================
      expand field, fieldsBase, recordset
=============================================== */
%extend bzs::db::protocol::tdap::client::fieldsBase {
  void getFieldByIndexRef(short index, field& return_field) const {
    return_field = self->operator[](index);
  }
  void getFieldByNameRef(const _TCHAR* name, field& return_field) const {
    return_field = self->operator[](name);
  }
};
%extend bzs::db::protocol::tdap::client::recordset {
  void getRow(size_t index, bzs::db::protocol::tdap::client::memoryRecord** return_record){
    *return_record = (self->operator[](index)).get();
  }
  bzs::db::protocol::tdap::client::memoryRecord* getRecord(size_t index) {
    return (self->operator[](index)).get();
  }
};


/* ===============================================
  cpointer
=============================================== */
#if defined(SWIGRUBY)
%{
  // %pointer_functions(bzs::db::protocol::tdap::client::memoryRecord*, memoryRecord_p_p)
  static bzs::db::protocol::tdap::client::memoryRecord* *new_memoryRecord_p_p() {
    return new bzs::db::protocol::tdap::client::memoryRecord*();
  }
  static void delete_memoryRecord_p_p(bzs::db::protocol::tdap::client::memoryRecord* *obj) {
    if (obj) delete obj;
  }
  static bzs::db::protocol::tdap::client::memoryRecord* memoryRecord_p_p_value(bzs::db::protocol::tdap::client::memoryRecord* *obj) {
    return *obj;
  }
%}
#else
%include "cpointer.i"
%pointer_functions(bzs::db::protocol::tdap::client::memoryRecord*, memoryRecord_p_p)
#endif
