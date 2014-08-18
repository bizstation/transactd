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

// Suppress warnings
#pragma SWIG nowarn=SWIGWARN_PARSE_USING_UNDEF
%warnfilter(SWIGWARN_LANG_OVERLOAD_SHADOW) btrstoa;
%warnfilter(SWIGWARN_PARSE_UNNAMED_NESTED_CLASS) nstable;


/* ===============================================
      type defines
=============================================== */
%apply unsigned long long { unsigned __int64 }
%apply long long { __int64 }


/* ===============================================
      HEADERS (for cpp compilation)
=============================================== */
%{
#include <bzs/env/crosscompile.h>
#include <bzs/env/compiler.h>
#include <bzs/db/protocol/tdap/client/export.h>
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
#include <bzs/db/protocol/tdap/client/recordset.h>
#include <bzs/db/protocol/tdap/client/activeTable.h>
#include <bzs/db/protocol/tdap/client/groupQuery.h>
#include <bzs/db/protocol/tdap/client/pooledDatabaseManager.h>

using namespace bzs::db::protocol::tdap;
using namespace bzs::db::protocol::tdap::client;
%}


/* ===============================================
      ignore, rename, new/delobject, extend
=============================================== */
// * bzs/db/protocol/tdap/client/activeTable.h *
%delobject bzs::db::protocol::tdap::client::activeTable::release;
%extend bzs::db::protocol::tdap::client::activeTable {
  recordset* read(queryBase& q) {
    recordset* rs = new recordset();
    self->read(*rs, q);
    return rs;
  }
};

// * bzs/db/protocol/tdap/client/activeTableImple.h *
%ignore bzs::db::protocol::tdap::client::map_orm_fdi;
%ignore bzs::db::protocol::tdap::client::createFdi;
%ignore bzs::db::protocol::tdap::client::destroyFdi;
%ignore bzs::db::protocol::tdap::client::initFdi;

// * bzs/db/protocol/tdap/btrDate.h *
%ignore bzs::db::protocol::tdap::c_str;

// * bzs/db/protocol/tdap/client/client.h *
%ignore bzs::db::protocol::tdap::client::create;

// * bzs/db/protocol/tdap/client/connectionPool.h *
%ignore bzs::db::protocol::tdap::client::busyWaitArguments;
%ignore bzs::db::protocol::tdap::client::busyWait;
%ignore bzs::db::protocol::tdap::client::scopedLock;
%ignore bzs::db::protocol::tdap::client::connectionPool;
%ignore bzs::db::protocol::tdap::client::stdDbCconnectionPool;
%ignore bzs::db::protocol::tdap::client::stdDbmCconnectionPool;
%ignore bzs::db::protocol::tdap::client::stdCconnectionPool;
%ignore bzs::db::protocol::tdap::client::dllUnloadCallbackFunc;
%ignore bzs::db::protocol::tdap::client::releaseConnection;
%ignore bzs::db::protocol::tdap::client::cpool;

// * bzs/db/protocol/tdap/client/database.h *
%ignore bzs::db::protocol::tdap::client::database::operator=;
%ignore bzs::db::protocol::tdap::client::database::destroy;
  // add new/delobject define for database
%delobject bzs::db::protocol::tdap::client::database::release;
%delobject bzs::db::protocol::tdap::client::database::~database;
%extend bzs::db::protocol::tdap::client::database {
  ~database() {
    self->release();
  }
};
  // add newobject define for table
%newobject bzs::db::protocol::tdap::client::database::openTable;

// * bzs/db/protocol/tdap/client/dbDef.h *
%ignore bzs::db::protocol::tdap::client::dbdef::fieldNumByViewNum;

// * bzs/db/protocol/tdap/client/field.h *
%ignore bzs::db::protocol::tdap::client::compBlob;
%ignore bzs::db::protocol::tdap::client::fieldValue;
%ignore bzs::db::protocol::tdap::client::fieldShare;
%ignore bzs::db::protocol::tdap::client::field::operator=(const field&);
%ignore bzs::db::protocol::tdap::client::field::initialize;
%ignore bzs::db::protocol::tdap::client::field::ptr;
%ignore bzs::db::protocol::tdap::client::fielddefs::operator[] (const std::_tstring&) const;
%ignore bzs::db::protocol::tdap::client::field::operator!=;
%ignore bzs::db::protocol::tdap::client::field::operator==;
%ignore bzs::db::protocol::tdap::client::field::operator=(const std::_tstring&);
%rename(getFielddef) bzs::db::protocol::tdap::client::fielddefs::operator[] (int index) const;

// * bzs/db/protocol/tdap/client/fields.h *
%ignore bzs::db::protocol::tdap::client::fields;
%ignore bzs::db::protocol::tdap::client::fieldsBase::fd;
%ignore bzs::db::protocol::tdap::client::fieldsBase::operator[](const std::_tstring&) const;
%rename(Record) bzs::db::protocol::tdap::client::fieldsBase;
  // add methods
%extend bzs::db::protocol::tdap::client::fieldsBase {
  void getFieldByIndexRef(short index, field& return_field) const {
    return_field = self->operator[](index);
  }
  void getFieldByNameRef(const _TCHAR* name, field& return_field) const {
    return_field = self->operator[](name);
  }
};

// * bzs/db/protocol/tdap/client/groupComp.h *
%ignore bzs::db::protocol::tdap::client::compByKey;
%ignore bzs::db::protocol::tdap::client::grouping_comp;

// * bzs/db/protocol/tdap/client/groupQuery.h *
%ignore bzs::db::protocol::tdap::client::fieldNames;
%ignore bzs::db::protocol::tdap::client::fieldNames::operator=;
%ignore bzs::db::protocol::tdap::client::fieldNames::fieldNamesImple;
%ignore bzs::db::protocol::tdap::client::groupFuncBase::operator=;
%ignore bzs::db::protocol::tdap::client::groupFuncBase::groupFuncBaseImple;
%ignore bzs::db::protocol::tdap::client::groupQuery::operator=;
%ignore bzs::db::protocol::tdap::client::groupQuery::groupQueryImple;
%ignore bzs::db::protocol::tdap::client::recordsetImple;
%ignore bzs::db::protocol::tdap::client::recordsetQuery::operator=;
%ignore bzs::db::protocol::tdap::client::recordsetQuery::recordsetQueryImple;
%ignore bzs::db::protocol::tdap::client::sortField;
%ignore bzs::db::protocol::tdap::client::sortFields;
%ignore bzs::db::protocol::tdap::client::sortFields::operator[];
  // groupFunctions
    // recordsetQuery
%ignore bzs::db::protocol::tdap::client::recordsetQuery::create;
%ignore bzs::db::protocol::tdap::client::recordsetQuery::release;
%newobject bzs::db::protocol::tdap::client::recordsetQuery::recordsetQuery;
%extend bzs::db::protocol::tdap::client::recordsetQuery {
  recordsetQuery() {
    return bzs::db::protocol::tdap::client::recordsetQuery::create();
  }
  ~recordsetQuery() { self->release(); }
};
%ignore bzs::db::protocol::tdap::client::recordsetQuery::recordsetQuery;
    // avg
%ignore bzs::db::protocol::tdap::client::avg::create;
%newobject bzs::db::protocol::tdap::client::avg::avg;
%extend bzs::db::protocol::tdap::client::avg {
  avg(const _TCHAR* targetName , const _TCHAR* resultName) {
    return bzs::db::protocol::tdap::client::avg::create(targetName, resultName);
  }
  ~avg() { self->release(); }
};
%ignore bzs::db::protocol::tdap::client::avg::avg;
    // count
%ignore bzs::db::protocol::tdap::client::count::create;
%newobject bzs::db::protocol::tdap::client::count::count;
%extend bzs::db::protocol::tdap::client::count {
  count(const _TCHAR* resultName) {
    return bzs::db::protocol::tdap::client::count::create(resultName);
  }
  ~count() { self->release(); }
};
%ignore bzs::db::protocol::tdap::client::count::count;
    // max
%ignore bzs::db::protocol::tdap::client::max::create;
%newobject bzs::db::protocol::tdap::client::max::max;
%extend bzs::db::protocol::tdap::client::max {
  max(const _TCHAR* targetName , const _TCHAR* resultName) {
    return bzs::db::protocol::tdap::client::max::create(targetName, resultName);
  }
  ~max() { self->release(); }
};
%ignore bzs::db::protocol::tdap::client::max::max;
    // min
%ignore bzs::db::protocol::tdap::client::min::create;
%newobject bzs::db::protocol::tdap::client::min::min;
%extend bzs::db::protocol::tdap::client::min {
  min(const _TCHAR* targetName , const _TCHAR* resultName) {
    return bzs::db::protocol::tdap::client::min::create(targetName, resultName);
  }
  ~min() { self->release(); }
};
%ignore bzs::db::protocol::tdap::client::min::min;
    // sum
%ignore bzs::db::protocol::tdap::client::sum::create;
%newobject bzs::db::protocol::tdap::client::sum::sum;
%extend bzs::db::protocol::tdap::client::sum {
  sum(const _TCHAR* targetName , const _TCHAR* resultName) {
    return bzs::db::protocol::tdap::client::sum::create(targetName, resultName);
  }
  ~sum() { self->release(); }
};
%ignore bzs::db::protocol::tdap::client::sum::sum;

// * bzs/db/protocol/tdap/client/memRecord.h *
%ignore bzs::db::protocol::tdap::client::autoMemory;
%ignore bzs::db::protocol::tdap::client::autoMemory::operator=;
%ignore bzs::db::protocol::tdap::client::memoryRecord::clear;
%ignore bzs::db::protocol::tdap::client::memoryRecord::setRecordData;
%rename(createRecord) bzs::db::protocol::tdap::client::memoryRecord::create(fielddefs&);

// * bzs/db/protocol/tdap/client/nsDatabase.h *
%ignore bzs::db::protocol::tdap::client::nsdatabase::btrvFunc;
%ignore bzs::db::protocol::tdap::client::nsdatabase::operator=;
%ignore bzs::db::protocol::tdap::client::nsdatabase::createTable;
%ignore bzs::db::protocol::tdap::client::nsdatabase::getBtrVersion(btrVersions*, uchar_td*);
%ignore bzs::db::protocol::tdap::client::nsdatabase::getDllUnloadCallbackFunc;

// * bzs/db/protocol/tdap/client/nsTable.h *
%ignore bzs::db::protocol::tdap::client::nstable::release;
%rename(tdapLastErr) bzs::db::protocol::tdap::client::nstable::tdapErr(HWND, _TCHAR*);

// * bzs/db/protocol/tdap/client/recordset.h *
%ignore bzs::db::protocol::tdap::client::recordset::operator new;
%ignore bzs::db::protocol::tdap::client::recordset::operator delete;
%ignore bzs::db::protocol::tdap::client::recordset::operator=;
%rename(RecordSet) bzs::db::protocol::tdap::client::recordset;
%rename(unionRecordSet) bzs::db::protocol::tdap::client::recordset::operator+=;
  // add method
%extend bzs::db::protocol::tdap::client::recordset {
  void getRow(size_t index, bzs::db::protocol::tdap::client::fieldsBase** return_record){
    *return_record = &(self->operator[](index));
  }
};

// * bzs/db/protocol/tdap/client/recordsetImple.h *
%ignore bzs::db::protocol::tdap::client::recordsetSorter;
%ignore bzs::db::protocol::tdap::client::multiRecordAlocatorImple;

// * bzs/db/protocol/tdap/client/table.h *
%ignore bzs::db::protocol::tdap::client::keyValuePtr;
%ignore bzs::db::protocol::tdap::client::queryBase::operator=;
%ignore bzs::db::protocol::tdap::client::queryBase::addField;
%ignore bzs::db::protocol::tdap::client::queryBase::addLogic;
%ignore bzs::db::protocol::tdap::client::queryBase::addSeekKeyValue;
%ignore bzs::db::protocol::tdap::client::queryBase::addSeekKeyValuePtr;
  // ignore keyValueDescription and re-define with expand.
%ignore bzs::db::protocol::tdap::client::table::keyValueDescription(_TCHAR* buf, int bufsize);
%typemap(in,numinputs=0) (_TCHAR* keyValueDescription_buf) (_TCHAR tmpbuf[1024 * 8]) {
  $1 = tmpbuf;
}
%extend bzs::db::protocol::tdap::client::table {
  const _TCHAR* keyValueDescription(_TCHAR* keyValueDescription_buf) {
    self->keyValueDescription(keyValueDescription_buf, 1024 * 8);
    return keyValueDescription_buf;
  }
};
  // ignore queryBase destructor. use release instead of it.
%ignore bzs::db::protocol::tdap::client::queryBase::~queryBase;
%newobject bzs::db::protocol::tdap::client::queryBase::create;
%delobject bzs::db::protocol::tdap::client::queryBase::release;
  // add delobject define for table
%delobject bzs::db::protocol::tdap::client::table::release;
%ignore bzs::db::protocol::tdap::client::table::release;
  // add methods
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
  queryBase* and_(const _TCHAR* field, const _TCHAR* op, const _TCHAR* val)
  {
    self->addLogic(_T("and") ,field, op, val);
    return self;
  }
  queryBase* or_(const _TCHAR* field, const _TCHAR* op, const _TCHAR* val)
  {
    self->addLogic(_T("or") ,field, op, val);
    return self;
  }
  queryBase* in(const _TCHAR* value1, const _TCHAR* value2 = NULL,
    const _TCHAR* value3 = NULL, const _TCHAR* value4 = NULL, const _TCHAR* value5 = NULL,
    const _TCHAR* value6 = NULL, const _TCHAR* value7 = NULL, const _TCHAR* value8 = NULL)
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
    return self;
  }
  queryBase* addInValue(const _TCHAR* value, bool reset = false)
  {
    self->addSeekKeyValue(value, reset);
    return self;
  }
};

// * bzs/db/protocol/tdap/client/trdboostapi.h *
%ignore bzs::db::protocol::tdap::client::autoBulkinsert;
%ignore bzs::db::protocol::tdap::client::eFindCurrntType;
%ignore bzs::db::protocol::tdap::client::eIndexOpType;
%ignore bzs::db::protocol::tdap::client::eStepOpType;
%ignore bzs::db::protocol::tdap::client::filterdIterator;
%ignore bzs::db::protocol::tdap::client::qlogic;
%ignore bzs::db::protocol::tdap::client::snapshot;
%ignore bzs::db::protocol::tdap::client::tableIterator;
%ignore bzs::db::protocol::tdap::client::transaction;
%ignore bzs::db::protocol::tdap::client::connect;
%ignore bzs::db::protocol::tdap::client::connectOpen;
%ignore bzs::db::protocol::tdap::client::convertTable;
%ignore bzs::db::protocol::tdap::client::createDatabase;
%ignore bzs::db::protocol::tdap::client::createDatabaseForConnectionPool;
%ignore bzs::db::protocol::tdap::client::createDatadaseObject;
%ignore bzs::db::protocol::tdap::client::deleteRecord;
%ignore bzs::db::protocol::tdap::client::disconnect;
%ignore bzs::db::protocol::tdap::client::dropDatabase;
%ignore bzs::db::protocol::tdap::client::filterdIterator;
%ignore bzs::db::protocol::tdap::client::filterdIterator::operator++;
%ignore bzs::db::protocol::tdap::client::filterdIterator::operator==;
%ignore bzs::db::protocol::tdap::client::filterdIterator::operator!=;
%ignore bzs::db::protocol::tdap::client::filterdIterator::operator*;
%ignore bzs::db::protocol::tdap::client::filterdIterator::operator->;
%ignore bzs::db::protocol::tdap::client::find;
%ignore bzs::db::protocol::tdap::client::findRv;
%ignore bzs::db::protocol::tdap::client::for_each;
%ignore bzs::db::protocol::tdap::client::getFindIterator;
%ignore bzs::db::protocol::tdap::client::getTable;
%ignore bzs::db::protocol::tdap::client::insertField;
%ignore bzs::db::protocol::tdap::client::insertKey;
%ignore bzs::db::protocol::tdap::client::insertRecord;
%ignore bzs::db::protocol::tdap::client::insertTable;
%ignore bzs::db::protocol::tdap::client::isSameUri;
%ignore bzs::db::protocol::tdap::client::lexical_cast;
%ignore bzs::db::protocol::tdap::client::openDatabase;
%ignore bzs::db::protocol::tdap::client::openTable;
%ignore bzs::db::protocol::tdap::client::readIndex;
%ignore bzs::db::protocol::tdap::client::readIndex_v;
%ignore bzs::db::protocol::tdap::client::readIndexRv;
%ignore bzs::db::protocol::tdap::client::readIndexRv_v;
%ignore bzs::db::protocol::tdap::client::readStep;
%ignore bzs::db::protocol::tdap::client::readStepRv;
%ignore bzs::db::protocol::tdap::client::releaseDatabase;
%ignore bzs::db::protocol::tdap::client::releaseTable;
%ignore bzs::db::protocol::tdap::client::snapshot;
%ignore bzs::db::protocol::tdap::client::tableIterator;
%ignore bzs::db::protocol::tdap::client::tableIterator::operator++;
%ignore bzs::db::protocol::tdap::client::tableIterator::operator--;
%ignore bzs::db::protocol::tdap::client::tableIterator::operator==;
%ignore bzs::db::protocol::tdap::client::tableIterator::operator!=;
%ignore bzs::db::protocol::tdap::client::tableIterator::operator*;
%ignore bzs::db::protocol::tdap::client::tableIterator::operator->;
%ignore bzs::db::protocol::tdap::client::transaction;
%ignore bzs::db::protocol::tdap::client::updateRecord;
%ignore bzs::db::protocol::tdap::client::updateTableDef;
%ignore bzs::db::protocol::tdap::client::filter_validate_value;
%ignore bzs::db::protocol::tdap::client::filter_validate_block;
%ignore bzs::db::protocol::tdap::client::filter_invalidate_value;

// * bzs/db/protocol/tdap/client/trdormapi.h *
%ignore bzs::db::protocol::tdap::client::setValue;
%ignore bzs::db::protocol::tdap::client::begin;
%ignore bzs::db::protocol::tdap::client::end;
%ignore bzs::db::protocol::tdap::client::push_back;
%ignore bzs::db::protocol::tdap::client::readBefore;
%ignore bzs::db::protocol::tdap::client::mdlsHandler;
%ignore bzs::db::protocol::tdap::client::compFunc;
%ignore bzs::db::protocol::tdap::client::sortFunc;
%ignore bzs::db::protocol::tdap::client::sortFuncBase;
%ignore bzs::db::protocol::tdap::client::sortFunctor;
%ignore bzs::db::protocol::tdap::client::sort;
%ignore bzs::db::protocol::tdap::client::mraResetter;

// * bzs/db/protocol/tdap/tdapSchema.h *
%ignore DLLUNLOADCALLBACK_PTR;
%ignore dllUnloadCallback;

// * bzs/db/protocol/tdap/tdapSchema.h *
%ignore bzs::db::protocol::tdap::keySpec;
%ignore bzs::db::protocol::tdap::fileSpec;
%ignore bzs::db::protocol::tdap::fielddef::dataLen;
%ignore bzs::db::protocol::tdap::fielddef::keyCopy;
%ignore bzs::db::protocol::tdap::fielddef::keyData;
%ignore bzs::db::protocol::tdap::fielddef::keyDataLen;
%ignore bzs::db::protocol::tdap::fielddef::blobDataPtr;
%ignore bzs::db::protocol::tdap::fielddef::blobDataLen;
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
  // add methods
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

// * bzs/rtl/benchmark.h *
%ignore bzs::rtl::benchmark::report;
%ignore bzs::rtl::benchmark::report2;
%ignore bzs::rtl::benchmarkMt;


/* ===============================================
      external symbols
=============================================== */
#define DLLLIB
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
%include bzs/db/protocol/tdap/client/trdboostapi.h
%include bzs/db/protocol/tdap/client/groupQuery.h
%include bzs/db/protocol/tdap/client/recordset.h
%include bzs/db/protocol/tdap/client/activeTable.h
%include bzs/db/protocol/tdap/client/pooledDatabaseManager.h


/* ===============================================
      template
=============================================== */
// * bzs/db/protocol/tdap/client/activeTable.h *
%template(keyValue) bzs::db::protocol::tdap::client::activeTable::keyValue<_TCHAR*>;
%template(keyValue) bzs::db::protocol::tdap::client::activeTable::keyValue<_TCHAR*, _TCHAR*>;
%template(keyValue) bzs::db::protocol::tdap::client::activeTable::keyValue<_TCHAR*, _TCHAR*, _TCHAR*>;
%template(keyValue) bzs::db::protocol::tdap::client::activeTable::keyValue<_TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*>;
%template(keyValue) bzs::db::protocol::tdap::client::activeTable::keyValue<_TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*>;
%template(keyValue) bzs::db::protocol::tdap::client::activeTable::keyValue<_TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*>;
%template(keyValue) bzs::db::protocol::tdap::client::activeTable::keyValue<_TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*>;
%template(keyValue) bzs::db::protocol::tdap::client::activeTable::keyValue<_TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*>;
%template(keyValue) bzs::db::protocol::tdap::client::activeTable::keyValue<__int64>;
%template(keyValue) bzs::db::protocol::tdap::client::activeTable::keyValue<__int64, __int64>;
%template(keyValue) bzs::db::protocol::tdap::client::activeTable::keyValue<__int64, __int64, __int64>;
%template(keyValue) bzs::db::protocol::tdap::client::activeTable::keyValue<__int64, __int64, __int64, __int64>;
%template(keyValue) bzs::db::protocol::tdap::client::activeTable::keyValue<__int64, __int64, __int64, __int64, __int64>;
%template(keyValue) bzs::db::protocol::tdap::client::activeTable::keyValue<__int64, __int64, __int64, __int64, __int64, __int64>;
%template(keyValue) bzs::db::protocol::tdap::client::activeTable::keyValue<__int64, __int64, __int64, __int64, __int64, __int64, __int64>;
%template(keyValue) bzs::db::protocol::tdap::client::activeTable::keyValue<__int64, __int64, __int64, __int64, __int64, __int64, __int64, __int64>;
%template(keyValue) bzs::db::protocol::tdap::client::activeTable::keyValue<double>;
%template(keyValue) bzs::db::protocol::tdap::client::activeTable::keyValue<double, double>;
%template(keyValue) bzs::db::protocol::tdap::client::activeTable::keyValue<double, double, double>;
%template(keyValue) bzs::db::protocol::tdap::client::activeTable::keyValue<double, double, double, double>;
%template(keyValue) bzs::db::protocol::tdap::client::activeTable::keyValue<double, double, double, double, double>;
%template(keyValue) bzs::db::protocol::tdap::client::activeTable::keyValue<double, double, double, double, double, double>;
%template(keyValue) bzs::db::protocol::tdap::client::activeTable::keyValue<double, double, double, double, double, double, double>;
%template(keyValue) bzs::db::protocol::tdap::client::activeTable::keyValue<double, double, double, double, double, double, double, double>;

// * bzs/db/protocol/tdap/client/groupQuery.h *
%template(when) bzs::db::protocol::tdap::client::recordsetQuery::when<_TCHAR*>;
%template(and_) bzs::db::protocol::tdap::client::recordsetQuery::and_<_TCHAR*>;
%template(or_)  bzs::db::protocol::tdap::client::recordsetQuery::or_<_TCHAR*>;

// * bzs/db/protocol/tdap/client/trdboostapi.h *
%template(where) bzs::db::protocol::tdap::client::query::where<_TCHAR*>;
%template(and_) bzs::db::protocol::tdap::client::query::and_<_TCHAR*>;
%template(or_)  bzs::db::protocol::tdap::client::query::or_<_TCHAR*>;
%template(in)   bzs::db::protocol::tdap::client::query::in<_TCHAR*>;
%template(in)   bzs::db::protocol::tdap::client::query::in<_TCHAR*, _TCHAR*>;
%template(in)   bzs::db::protocol::tdap::client::query::in<_TCHAR*, _TCHAR*, _TCHAR*>;
%template(in)   bzs::db::protocol::tdap::client::query::in<_TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*>;
%template(in)   bzs::db::protocol::tdap::client::query::in<_TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*>;
%template(in)   bzs::db::protocol::tdap::client::query::in<_TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*>;
%template(in)   bzs::db::protocol::tdap::client::query::in<_TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*>;
%template(in)   bzs::db::protocol::tdap::client::query::in<_TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*, _TCHAR*>;
%template(in)   bzs::db::protocol::tdap::client::query::in<__int64>;
%template(in)   bzs::db::protocol::tdap::client::query::in<__int64, __int64>;
%template(in)   bzs::db::protocol::tdap::client::query::in<__int64, __int64, __int64>;
%template(in)   bzs::db::protocol::tdap::client::query::in<__int64, __int64, __int64, __int64>;
%template(in)   bzs::db::protocol::tdap::client::query::in<__int64, __int64, __int64, __int64, __int64>;
%template(in)   bzs::db::protocol::tdap::client::query::in<__int64, __int64, __int64, __int64, __int64, __int64>;
%template(in)   bzs::db::protocol::tdap::client::query::in<__int64, __int64, __int64, __int64, __int64, __int64, __int64>;
%template(in)   bzs::db::protocol::tdap::client::query::in<__int64, __int64, __int64, __int64, __int64, __int64, __int64, __int64>;
%template(in)   bzs::db::protocol::tdap::client::query::in<double>;
%template(in)   bzs::db::protocol::tdap::client::query::in<double, double>;
%template(in)   bzs::db::protocol::tdap::client::query::in<double, double, double>;
%template(in)   bzs::db::protocol::tdap::client::query::in<double, double, double, double>;
%template(in)   bzs::db::protocol::tdap::client::query::in<double, double, double, double, double>;
%template(in)   bzs::db::protocol::tdap::client::query::in<double, double, double, double, double, double>;
%template(in)   bzs::db::protocol::tdap::client::query::in<double, double, double, double, double, double, double>;
%template(in)   bzs::db::protocol::tdap::client::query::in<double, double, double, double, double, double, double, double>;

/* ===============================================
      cpointer
=============================================== */
#if defined(SWIGPHP)
%include "cpointer.i"
%pointer_functions(bzs::db::protocol::tdap::client::fieldsBase*, fieldsBase_p_p)
#endif
