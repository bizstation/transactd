#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_TABLE_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_TABLE_H
/* =================================================================
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
 ================================================================= */
#include "nsTable.h"
#include <vector>
namespace bzs
{

namespace db
{
	struct blobHeader;
	struct blob;
namespace protocol
{
namespace tdap
{
namespace client
{

/** @cond INTERNAL */
class database;
class queryBase;
class fields;
#define null_str _T("")

#pragma warning(disable:4251)

static const int mra_nojoin = 0;
static const int mra_first = 0;
static const int mra_nextrows = 1;
static const int mra_innerjoin = 2;
static const int mra_outerjoin = 4;
static const int mra_current_block = -1;

class multiRecordAlocator
{

public:
	virtual ~multiRecordAlocator(){}
	virtual void init(size_t recordCount, size_t recordLen,int addType, const class table* tb) = 0;
	virtual unsigned char* ptr(size_t row, int stat) = 0;
	virtual void setJoinType(int v)=0;
	virtual void setInvalidRecord(size_t row, bool v) = 0;
	virtual void setJoinRowMap(const std::vector< std::vector<int> >* v/*, size_t size*/)=0;
	virtual const std::vector<std::vector<int> >* joinRowMap()const  = 0;

};

/** @endcond */

class AGRPACK table : public nstable
{
	friend class recordCache;
	friend class database;
	friend class filter;
	friend class fields;

	struct tbimpl* m_impl;
	class fielddefs* m_fddefs;
	tabledef* m_tableDef;

	uchar_td charset() const ;
	bool checkFindDirection(ushort_td op);
	bool checkIndex(short index) const;
	void getKeySpec(keySpec* ks, bool SpecifyKeyNum = false);
	const bzs::db::blobHeader* getBlobHeader();
	void setBlobFieldPointer(char*ptr, const ::bzs::db::blobHeader* p);
	void addSendBlob(const bzs::db::blob* blob);
	void addBlobEndRow();
	void resetSendBlob();
	void btrvGetExtend(ushort_td op);
	void getRecords(ushort_td op);
	uint_td doGetWriteImageLen();   // orverride
	void doUpdate(bool ncc = false); // orverride
	ushort_td doCommitBulkInsert(bool autoCommit = false); // orverride
	void doAbortBulkInsert(); // orverride
	void doCreateIndex(bool SpecifyKeyNum = false); // orverride
	uint_td doRecordCount(bool estimate, bool fromCurrent, eFindType direction); // orverride
	short_td doBtrvErr(HWND hWnd, _TCHAR* retbuf = NULL); // orverride

	//double getFVnumeric(short index);
	//double getFVDecimal(short index);
	//void setFVDecimal(short index, double data);
	//void setFVNumeric(short index, double data);
	void doFind( ushort_td op, bool notIncCurrent);
	bool setSeekValueField(int row);

protected:
	explicit table(nsdatabase *pbe); // Inheritance is impossible
	virtual ~table();
	void* dataBak() const ;
	void setDataBak(void* v);
	void setBookMarks(int StartId, void* Data, ushort_td Count);
	uint_td unPack(char*ptr, size_t size);
	uint_td pack(char*ptr, size_t size);
	keylen_td writeKeyData(); // orverride
	keylen_td writeKeyDataTo(uchar_td* to);

	void writeRecordData(){};

	void onReadAfter();  // orverride
	bool onUpdateCheck(eUpdateType type);     // orverride

	int onUpdateBefore(){return 0;}; // orverride

	void onUpdateAfter(int beforeResult); // orverride
	bool onDeleteCheck(bool inkey); // orverride

	int onInsertBefore() {return 0;}; // orverride

	void onInsertAfter(int beforeResult); // orverride
	bool isUniqeKey(char_td keynum);//orverride
	void init(tabledef* def, short filenum, bool regularDir);
	void* attachBuffer(void* newPtr, bool unpack = false, size_t size = 0);
	void dettachBuffer();

	virtual void doInit(tabledef* def, short filenum, bool regularDir);

	virtual void onRecordCounting(size_t count, bool& complate){};

	virtual void setNoUpdateTimeStamp(bool v) {};


public:
	using nstable::eFindType;

	inline const tabledef* tableDef() const {return m_tableDef;};
	inline const tabledef** tableDefPtr() const {return const_cast<const tabledef**>(&m_tableDef);};

	inline bool valiableFormatType() const {return m_tableDef->optionFlags.bitA;}

	inline bool blobFieldUsed() const {return m_tableDef->optionFlags.bitB;}

	bool logicalToString() const ;
	void setLogicalToString(bool v);
	bool trimPadChar() const ;
	void setTrimPadChar(bool v);
	bool usePadChar() const ;
	void setUsePadChar(bool v);
	void* optionalData() const;
	void setOptionalData(void* v);
	bool myDateTimeValueByBtrv() const ;
	int bookMarksCount() const ;
	void moveBookmarksId(long Id);
	void clearBuffer();
	unsigned int getRecordHash();
	void smartUpdate();

	void setMra(multiRecordAlocator* p);
	multiRecordAlocator* mra() const;

	void find(eFindType type=findForword);
	void findFirst();
	void findLast();
	void findNext(bool notIncCurrent = true);
	void findPrev(bool notIncCurrent = true);
	bookmark_td bookmarkFindCurrent() const;
	void setQuery(const queryBase* query);
	void setFilter(const _TCHAR* str, ushort_td rejectCount, ushort_td cacheCount, bool autoEscape = true);
	short fieldNumByName(const _TCHAR* name);
	unsigned char getFVbyt(short index);
	short getFVsht(short index);
	int getFVint(short index);
	int getFVlng(short index);
	__int64 getFV64(short index);
	float getFVflt(short index);
	double getFVdbl(short index);
	const char* getFVAstr(short index);
	void* getFVbin(short index, uint_td& size);
	unsigned char getFVbyt(const _TCHAR* fieldName);
	short getFVsht(const _TCHAR* fieldName);
	int getFVint(const _TCHAR* fieldName);
	int getFVlng(const _TCHAR* fieldName);
	__int64 getFV64(const _TCHAR* fieldName);
	float getFVflt(const _TCHAR* fieldName);
	double getFVdbl(const _TCHAR* fieldName);
	const char* getFVAstr(const _TCHAR* fieldName);
	void* getFVbin(const _TCHAR* fieldName, uint_td& size);
	void setFV(short index, double data);
	void setFV(short index, float data);
	void setFV(short index, unsigned char data);
	void setFV(short index, short data);
	void setFVA(short index, const char* data);
	void setFV(short index, int data);
	void setFV(short index, const void* data, uint_td size);
	void setFV(const _TCHAR* fieldName, int data);
	void setFVA(const _TCHAR* fieldName, const char* data);
	void setFVW(short index, const wchar_t* data);
	void setFVW(const _TCHAR* fieldName, const wchar_t* data);

#ifdef _WIN32
	const wchar_t* getFVWstr(const _TCHAR* fieldName);
	const wchar_t* getFVWstr(short index);

#endif

#ifdef _UNICODE
	inline const wchar_t* getFVstr(short index) {return getFVWstr(index);};
	inline const wchar_t* getFVstr(const wchar_t* fieldName) {return getFVWstr(fieldName);};
#else
	inline const char* getFVstr(short index) {return getFVAstr(index);};
	inline const char* getFVstr(const char* fieldName) {return getFVAstr(fieldName);};
#endif

	inline void setFV(short index, const wchar_t* data) {setFVW(index, data);};
	inline void setFV(short index, const char* data) {setFVA(index, data);};
	inline void setFV(const _TCHAR* fieldName, const char* data) {setFVA(fieldName, data);};
	inline void setFV(const _TCHAR* fieldName, const wchar_t* data) {setFVW(fieldName, data);};

	void setFV(const _TCHAR* fieldName, double data);
	void setFV(const _TCHAR* fieldName, float data);
	void setFV(const _TCHAR* fieldName, unsigned char data);
	void setFV(const _TCHAR* fieldName, short data);
	void setFV(const _TCHAR* fieldName, const void* data, uint_td size);
	void setFV(short index, __int64 data);
	void setFV(const _TCHAR* fieldName, __int64 data);
	void* fieldPtr(short index) const;
	void keyValueDescription(_TCHAR* buf, int bufsize);
	short getCurProcFieldCount() const;
	short getCurProcFieldIndex(short index) const;
	client::fields& fields();

};


class AGRPACK queryBase
{
	friend class filter;
	struct impl* m_impl;

	const std::vector<std::_tstring>& getSelects() const;
	const std::vector<std::_tstring>& getWheres() const;
	const std::vector<std::_tstring>& getSeekKeyValues() const;
	const std::vector<const void*>& getSeekValuesPtr() const;
	
public:
	enum eOptimize{none=0, joinKeyValuesUnique=1, joinWhereFields=2};

	queryBase();
	virtual ~queryBase();
	void reset();
	void clearSeekKeyValues();
	void clearSelectFields();
	void addField(const _TCHAR* name);
	void addLogic(const _TCHAR* name, const _TCHAR* logic,  const _TCHAR* value);
	void addLogic(const _TCHAR* combine, const _TCHAR* name, const _TCHAR* logic,  const _TCHAR* value);
	void addSeekKeyValue(const _TCHAR* value, bool reset=false);
	void addSeekKeyValuePtr(const void* value, bool reset=false);
	void reserveSeekKeyValueSize(size_t v);
	queryBase& queryString(const _TCHAR* str, bool autoEscape = false);
	queryBase& reject(int v);
	queryBase& limit(int v);
	queryBase& direction(table::eFindType v);
	queryBase& all();
	queryBase& optimize(eOptimize v);
	queryBase& bookmarkAlso(bool v);

	const _TCHAR* toString() const;
	table::eFindType getDirection() const;
	int getReject()const;
	int getLimit()const;
	bool isAll()const;
	eOptimize getOptimize() const;
	bool isBookmarkAlso()const;
	short selectCount() const;
	const _TCHAR* getSelect(short index) const;
	short whereTokens() const;
	const _TCHAR* getWhereToken(short index) const;
	void reverseAliasName(const _TCHAR* alias, const _TCHAR* src);

	void release();
	static queryBase* create();
};


#pragma warning(default:4251)


}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs

#endif//BZS_DB_PROTOCOL_TDAP_CLIENT_TABLE_H
