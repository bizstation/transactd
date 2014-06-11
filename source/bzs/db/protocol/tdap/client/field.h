#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_FIELD_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_FIELD_H
/*=================================================================
   Copyright (C) 2014 BizStation Corp All rights reserved.

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
#include "fieldNameAlias.h"

class CField; //atl interface

namespace bzs
{
	namespace rtl {class stringBuffer;}
namespace db
{
namespace protocol
{
namespace tdap
{
namespace client
{

class stringConverter;
/** @cond INTERNAL */
class DLLLIB fieldShare
{
	friend class field;
	friend class table;
	friend class recordCache;

private:
	struct
	{
	unsigned char myDateTimeValueByBtrv: 1;
	unsigned char trimPadChar: 1;
	unsigned char usePadChar: 1;
	unsigned char logicalToString: 1;
	};
	struct Imple* m_imple;

protected:
	fieldShare();

	virtual ~fieldShare();
	stringConverter* cv();
	bzs::rtl::stringBuffer* strBufs();
	void blobPushBack(char* p);
	void blobClear();
};

/** @endcond */


class DLLLIB fielddefs : public fieldShare
{
	struct infoImple* m_imple;
	void aliasing(fielddef* p) const;
	fielddefs();
	~fielddefs();
	friend class table;
	friend class recordsetImple;
	friend class writableRecord;
	friend class memoryRecord;
	friend class recordsetQuery;
	friend struct recordsetQueryImple;

	void addAllFileds(tabledef* def);
	void copyFrom(const class table* tb);
	bool canUnion(const fielddefs& r) const;
	size_t totalFieldLen() const;
	void resetUpdateIndicator();
	void setAliases(const aliasMap_type* p);
	void push_back(const fielddef* p, bool rePosition=false);
	void remove(int index);
	void reserve(size_t size);
	void clear();

public:
	fielddefs* clone() const;
	int indexByName(const std::_tstring& name)const;
	const fielddef& operator[] (int index) const;
	const fielddef& operator[] (const _TCHAR* name) const;
	const fielddef& operator[] (const std::_tstring& name)const;
	bool checkIndex(int index)const;
	size_t size() const;
	static fielddefs* create();
	static void destroy(fielddefs* p);
};



typedef int (*compFieldFunc)(const class field& l, const class field& r, char logType);

class DLLLIB field
{
	friend class table;
	friend class fieldsBase;
	friend class CField;//atl interface
	/** @cond INTERNAL */
	friend int compBlob(const field& l, const field& r, char logType);
	/** @endcond */
	fielddef* m_fd;
	unsigned char* m_ptr;
	class fielddefs* m_fds;

	compFieldFunc getCompFunc(char logType)const;
	int blobLenBytes() const {return m_fd->blobLenBytes();}

private:
	//  ---- bigin regacy interfaces ----  //
	unsigned char getFVbyt() const;
	short getFVsht() const;
	int getFVint() const;
	int getFVlng() const;
	__int64 getFV64() const;
	float getFVflt() const;
	double getFVdbl() const;
	void* getFVbin(uint_td& size ) const;
	const char* getFVAstr() const;
#ifdef _WIN32
	const wchar_t* getFVWstr() const;
	void setFVW(const wchar_t* data);
#endif
	void setFV(float data);
	void setFV(double data);
	void setFV(unsigned char data);
	void setFV(short data);
	void setFV(int data);
	void setFV(__int64 data);
	void setFVA(const char* data);
	void setFV(const void* data, uint_td size);
#ifdef _UNICODE
	inline const wchar_t* getFVstr() const {return getFVWstr();};
	inline void setFV(const wchar_t* data) {setFVW(data);};
#else
	inline const char* getFVstr() const {return getFVAstr();};
	inline void setFV(const char* data) {setFVA(data);};
#endif
	double getFVnumeric() const;
	double getFVDecimal() const;
	void setFVDecimal(double data);
	void setFVNumeric(double data);
	//  ---- end regacy interfaces ----  //

	inline field(unsigned char* ptr, const fielddef& fd, fielddefs* fds)
			: m_ptr(ptr), m_fd((fielddef*)&fd), m_fds(fds) {};

/** @cond INTERNAL */
#if defined(SWIG) || defined(SWIG_BUILDING) //SWIG Wrapper need public constructor
public:
#endif
	inline field()
			: m_ptr(NULL), m_fd(NULL), m_fds(NULL) {};
/** @endcond */

public:

	void* ptr() const;
	inline field& operator=(const field& r)
	{
		m_fd = r.m_fd;
		m_ptr = r.m_ptr;
		m_fds = r.m_fds;
		return *this;
	}

	unsigned char type() const {return m_fd->type;}
	unsigned short len() const {return m_fd->len;}

	inline const _TCHAR* c_str() const {return getFVstr();}

	inline const char* a_str() const {return getFVAstr();}

	inline int i() const {return getFVint();}

	inline int i8() const {return getFVbyt();}

	inline short i16() const {return getFVsht();}

	inline __int64 i64() const {return getFV64();}

	inline float f() const {return getFVflt();}

	inline double d() const {return getFVdbl();}

	inline field& operator = (const _TCHAR* p)
	{
		setFV(p);
		m_fd->enableFlags.bitE = true;
		return *this;
	}

	inline field& operator = (const std::_tstring& p)
	{
		setFV(p.c_str());
		m_fd->enableFlags.bitE = true;
		return *this;
	}

#ifdef _UNICODE
	inline field& operator = (const char* p)
	{
		setFVA(p);
		m_fd->enableFlags.bitE = true;
		return *this;
	}

	inline field& operator = (const std::string& p)
	{
		setFVA( p.c_str());
		m_fd->enableFlags.bitE = true;
		return *this;
	}

#endif

	inline field& operator = (int v)
	{
		setFV( v);
		m_fd->enableFlags.bitE = true;
		return *this;
	}

	inline field& operator = (__int64 v)
	{
		setFV(v);
		m_fd->enableFlags.bitE = true;
		return *this;
	}

	inline field& operator = (float v)
	{
		setFV(v);
		m_fd->enableFlags.bitE = true;
		return *this;
	}

	inline field& operator = (double v)
	{
		setFV(v);
		m_fd->enableFlags.bitE = true;
		return *this;
	}

	inline bool operator != (const _TCHAR* p) {return (_tcscmp(p, c_str()) != 0);};
	inline bool operator == (const _TCHAR* p) {return (_tcscmp(p, c_str())==0);};

	inline bool operator != (int v) {return (v != i());};
	inline bool operator == (int v) {return (v == i());};

	inline bool operator != (short v) {return (v != i16());};
	inline bool operator == (short v) {return (v == i16());};

	inline bool operator != (__int64 v) {return (v != i64());};
	inline bool operator == (__int64 v) {return (v == i64());};

	inline bool operator != (float v) {return (v != f());};
	inline bool operator == (float v) {return (v == f());};

	inline bool operator != (double v) {return (v != d());};
	inline bool operator == (double v) {return (v == d());};

	inline void setBin(const void* data, uint_td size)
	{
		setFV(data, size);
		m_fd->enableFlags.bitE = true;
	}
	inline void* getBin(uint_td& size){return getFVbin(size);};

	int comp(const field& r, char logType=CMPLOGICAL_VAR_COMP_ALL) const;

	bool isCompPartAndMakeValue();

};

/* For template tget type num by type.*/
inline int getFieldType(int )
{
	return ft_integer;
}

inline int getFieldType(__int64 )
{
	return ft_integer;
}

inline int getFieldType(short )
{
	return ft_integer;
}

inline int getFieldType(char )
{
	return ft_integer;
}

inline int getFieldType(double )
{
	return ft_float;
}

inline int getFieldType(float )
{
	return ft_float;
}

inline __int64 fieldValue(const field& fd, __int64 ) {return fd.i64();}

inline int fieldValue(const field& fd, int ) {return fd.i();}

inline short fieldValue(const field& fd, short ) {return (short)fd.i();}

inline char fieldValue(const field& fd, char ) {return (char)fd.i();}

inline double fieldValue(const field& fd, double ) {return fd.d();}

inline float fieldValue(const field& fd, float ) {return fd.f();}

inline const _TCHAR* fieldValue(const field& fd, const _TCHAR* ) {return fd.c_str();}


DLLLIB const fielddef& dummyFd();

}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs
#endif //BZS_DB_PROTOCOL_TDAP_CLIENT_FIELD_H

