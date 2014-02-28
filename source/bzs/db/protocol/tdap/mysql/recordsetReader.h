/*=================================================================
   Copyright (C) 2012 2013 BizStation Corp All rights reserved.

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
#ifndef BZS_DB_PROTOCOL_TDAP_MYSQL_RECORDSETREADER_H
#define BZS_DB_PROTOCOL_TDAP_MYSQL_RECORDSETREADER_H

#include "request.h"
#include <bzs/rtl/exception.h>
#include <bzs/db/engine/mysql/IReadRecords.h>
#include <bzs/db/engine/mysql/fieldAccess.h>
#include <boost/shared_ptr.hpp>

#ifndef MAX_KEY_SEGMENT
#define MAX_KEY_SEGMENT 8
#endif
namespace bzs
{
namespace db
{
namespace protocol
{
namespace tdap
{
namespace mysql
{

#pragma option -a-
pragma_pack1

struct logicalField;
struct resultField;

/** calculate record position
 */
class position
{
	engine::mysql::table* m_tb;
	const char* m_record;
public:
	inline position();
	inline void setTable(engine::mysql::table* tb);
	inline char* fieldPtr(const resultField* rf) const;
	inline bool isBlobField(const resultField* rf) const;
	inline void addBlobBuffer(int fieldNum){m_tb->addBlobBuffer(fieldNum);};
	inline void setBlobFieldCount(int v){m_tb->setBlobFieldCount(v);};
	inline unsigned short packLen(const resultField* rf) const;
	inline const char* record()const{return m_record;}
	inline ulong recordLenCl()const{return m_tb->recordLenCl();}
	inline int recordFormatType()const {return m_tb->recordFormatType();};
	inline uint recordPackCopy(char* buf, uint maxlen)const{return m_tb->recordPackCopy(buf, maxlen);}
	int getFieldNumByPos(unsigned short pos)const
	{
		for (int i=0;i<m_tb->fields();i++)
		{
			char* start = m_tb->fieldPos(0);
			if (m_tb->fieldPos(i)-start == pos)
				return i;
		}
		return -1;
	}
	inline uint fieldSizeByte(int fieldNum){return m_tb->fieldSizeByte(fieldNum);}
};

/** If get all field then len = record length.
 *  
 */
struct resultField
{
	unsigned short len; 
	union
	{
		unsigned short pos;
		unsigned short fieldNum;
	};
};

struct extResultDef
{
	unsigned short maxRows;
	unsigned short fieldCount;
	resultField	   field[1]; //variable
};




inline position::position():m_tb(NULL),m_record(NULL){};
inline void position::setTable( engine::mysql::table* tb)
{
	m_tb = tb;
	m_record = (const char*)m_tb->record();	
}

inline char* position::fieldPtr(const resultField* rf) const
{
	return m_tb->fieldPos(rf->fieldNum);
}

inline bool position::isBlobField(const resultField* rf) const
{
	return db::engine::mysql::isBlobType(m_tb->fieldType(rf->fieldNum));
}


/** return data length as real rength.
 */
inline unsigned short position::packLen(const resultField* rf) const
{
	return m_tb->fieldDataLen(rf->fieldNum);
}

inline int compareUint24(const char* l, const char* r)
{
	unsigned int lv = *((unsigned int*)l) & 0xFFFFFF;
	unsigned int rv = *((unsigned int*)r) & 0xFFFFFF;
	if (lv < rv)
		return -1;
	if (lv > rv)
		return 1;
	return 0;
}

inline int compareInt24(const char* l, const char* r)
{
	int lv = ((*((int*)l) & 0xFFFFFF) << 8) / 0x100;
	int rv = ((*((int*)r) & 0xFFFFFF) << 8) / 0x100;
	 
	if (lv < rv)
		return -1;
	else if (lv > rv)
		return 1;
	return 0;
}

template <class T>
inline int compare(const char* l, const char* r)
{
	if (*((T*)l) < *((T*)r))
		return -1;
	else if (*((T*)l) > *((T*)r))
		return 1;
	return 0;
}

template <class T>
inline int compare(T l, T r)
{
	if (l < r)
		return -1;
	else if (l > r)
		return 1;
	return 0;
}

template <class T>
inline int compareVartype(const char* l, const char* r, bool bin, char logType)
{
	int llen = (*(T*)l);
	int rlen = (*(T*)r);
	int tmp = std::min(llen, rlen);
	if (logType & CMPLOGICAL_CASEINSENSITIVE)
		tmp = _strnicmp(l + sizeof(T), r + sizeof(T), tmp);
	else if (bin)
		tmp = memcmp(l + sizeof(T), r + sizeof(T), tmp);
	else
		tmp = strncmp(l + sizeof(T), r + sizeof(T), tmp);

	if (logType & CMPLOGICAL_VAR_COMP_ALL)
		return (tmp==0)?compare<int>(llen, rlen):tmp; //match complete 
	return (tmp==0 && (llen < rlen))? -1:tmp; //match a part
}

template <class T>
inline int compareWvartype(const char* l, const char* r, bool bin, char logType)
{
	int llen = (*(T*)l) / sizeof(char16_t);
	int rlen = (*(T*)r) / sizeof(char16_t);
	int tmp = std::min(llen, rlen);
	if (logType & CMPLOGICAL_CASEINSENSITIVE)
		tmp = wcsnicmp16((char16_t*)(l + sizeof(T)), (char16_t*)(r + sizeof(T)), tmp);
	else if (bin)
		tmp = memcmp((char16_t*)(l + sizeof(T)), (char16_t*)(r + sizeof(T)), tmp);
	else
		tmp = wcsncmp16((char16_t*)(l + sizeof(T)), (char16_t*)(r + sizeof(T)), tmp);
	if (logType & CMPLOGICAL_VAR_COMP_ALL)
		return (tmp==0)?compare<int>(llen, rlen):tmp; //match complete
	return (tmp==0 && (llen < rlen))? -1:tmp; //match a part
}

inline int compareBlobType(const char* l, const char* r, bool bin, char logType, int sizeByte)
{
	int llen = 0;
	int rlen = 0;
	memcpy(&llen, l, sizeByte);
	memcpy(&rlen, r, sizeByte);
	int tmp = std::min(llen, rlen);
	const char* lptr =  *((const char**)(l + sizeByte));
	const char* rptr = r + sizeByte;
	if (logType & CMPLOGICAL_CASEINSENSITIVE)
		tmp = _strnicmp(lptr, rptr, tmp);
	else if (bin)
		tmp = memcmp(lptr, rptr, tmp);
	else
		tmp = strncmp(lptr, rptr, tmp);

	if (logType & CMPLOGICAL_VAR_COMP_ALL)
		return (tmp==0)?compare<int>(llen, rlen):tmp; 
	return (tmp==0 && (llen < rlen))? -1:tmp; 
}

#define REC_MACTH			0
#define REC_NOMACTH			1
#define REC_NOMACTH_NOMORE  2

//#define COMP_USE_SWITCHCASE
#ifndef COMP_USE_SWITCHCASE
#define COMP_USE_FUNCTION_POINTER 
#endif

struct logicalField;
typedef int (logicalField::*compFunc)(const char* l, const char* r, int sizeByte) const;
struct logicalField
{
	
public:
	unsigned char	type;
	unsigned short	len;
	unsigned short	pos;
	char			logType;
	char			opr;
	union
	{
		unsigned short	offset;  
		unsigned char	ptr[2]; //variable
	};
	
	logicalField* next() const
	{
		return (logType & 64)?(logicalField*)(ptr + 2):(logicalField*)(ptr + len);
	}

public:	
#ifdef COMP_USE_SWITCHCASE
	int comp(const char* record, int sizeByte) const
	{	
		const char* r = (const char*) ptr;
		if  (logType & CMPLOGICAL_FIELD)
			r = record + offset;
		const char* l = record + pos;
		switch(type)
		{
		case ft_integer:
		case ft_autoinc:
		case ft_currency:
		{
			switch(len)
			{
			case 1:return compare<char>(l, r);
			case 2:return compare<short>(l, r);
			case 3:return compareInt24(l, r);
			case 4:return compare<int>(l, r);
			case 8:return compare<__int64>(l, r);
			}
		}
		case ft_mychar:
		case ft_string:
			if (logType & CMPLOGICAL_CASEINSENSITIVE)
				return _strnicmp(l, r, len);
			return memcmp(l, r, len);
		case ft_zstring:
		case ft_note:
			if (logType & CMPLOGICAL_CASEINSENSITIVE)
				return _strnicmp(l, r, len);
			return strncmp(l, r, len);
		case ft_logical:
		case ft_uinteger:
		case ft_autoIncUnsigned:
		case ft_date:
		case ft_time:
		case ft_timestamp:
		case ft_mydate:
		{
			switch(len)
			{
			case 1:return compare<unsigned char>(l, r);
			case 2:return compare<unsigned short>(l, r);
			case 3:return compareUint24(l, r);
			case 4:return compare<unsigned int>(l, r);
			case 8:return compare<unsigned __int64>(l, r);
			}
		}
		
		case ft_mytime:
		case ft_mydatetime:
		case ft_mytimestamp:
			return memcmp(l, r, len);
		case ft_float:
			switch(len)
			{
			case 4:return compare<float>(l, r);
			case 8:return compare<double>(l, r);
			}
		case ft_mywchar:
		case ft_wstring:
		case ft_wzstring:
			if (logType & CMPLOGICAL_CASEINSENSITIVE)
				return wcsnicmp16((char16_t*)l, (char16_t*)r, len);
			if ((type==ft_wstring)||(type==ft_mywchar))
				return memcmp(l, r, len);	
			return wcsncmp16((char16_t*)l, (char16_t*)r, len);
		case ft_lstring: 
		case ft_myvarchar:
		case ft_myvarbinary:
			if (sizeByte==1)
				return compareVartype<unsigned char>(l, r, type==ft_myvarbinary, logType);
			return compareVartype<unsigned short>(l, r, type==ft_myvarbinary, logType); 
		case ft_mywvarchar:
		case ft_mywvarbinary:
			if (sizeByte==1)
				return compareWvartype<unsigned char>(l, r, type==ft_mywvarbinary, logType);
			return compareWvartype<unsigned short>(l, r, type==ft_mywvarbinary, logType);
		case ft_mytext:
		case ft_myblob:
			return compareBlobType(l, r, type==ft_myblob, logType, sizeByte);
		}
		return 0;
	};
#else //COMP_USE_FUNCTION_POINTER

	template <class T>
	int compNumber(const char* l, const char* r, int sizeByte) const
	{	
		return compare<T>(l , r);
	}

	int compNumber24(const char* l, const char* r, int sizeByte) const
	{	
		return compareInt24(l , r);
	}

	int compNumberU24(const char* l, const char* r, int sizeByte) const
	{	
		return compareUint24(l , r);
	}

	int compMem(const char* l, const char* r, int sizeByte) const
	{	
		return memcmp(l , r, len);
	}

	int compString(const char* l, const char* r, int sizeByte) const
	{	
		return strncmp(l , r, len);
	}

	int compiString(const char* l, const char* r, int sizeByte) const
	{	
		return _strnicmp(l , r, len);
	}

	int compWString(const char* l, const char* r, int sizeByte) const
	{	
		return wcsncmp16((char16_t*)l , (char16_t*)r, len);
	}

	int compiWString(const char* l, const char* r, int sizeByte) const
	{	
		return wcsnicmp16((char16_t*)l , (char16_t*)r, len);
	}

	template <class T>
	int compVarString(const char* l, const char* r, int sizeByte) const
	{	
		return compareVartype<T>(l, r, type==ft_myvarbinary, logType); 
	}

	template <class T>
	int compWVarString(const char* l, const char* r, int sizeByte) const
	{	
		return compareWvartype<T>(l, r, type==ft_mywvarbinary, logType); 
	}

	int compBlob(const char* l, const char* r, int sizeByte) const
	{	
		return compareBlobType(l, r, type==ft_myblob, logType, sizeByte);
	}

	compFunc logicalField::getCompFunc(int sizeByte)
	{		
		switch(type)
		{
		case ft_integer:
		case ft_autoinc:
		case ft_currency:
		{
			switch(len)
			{
			case 1:return &logicalField::compNumber<char>;
			case 2:return &logicalField::compNumber<short>;
			case 3:return &logicalField::compNumber24;
			case 4:return &logicalField::compNumber<int>;
			case 8:return &logicalField::compNumber<__int64>;
			}
		}
		case ft_mychar:
		case ft_string:
			if (logType & CMPLOGICAL_CASEINSENSITIVE)
				return &logicalField::compiString;
			return &logicalField::compMem;
		case ft_zstring:
		case ft_note:
			if (logType & CMPLOGICAL_CASEINSENSITIVE)
				return &logicalField::compiString;
			return &logicalField::compString;
		case ft_logical:
		case ft_uinteger:
		case ft_autoIncUnsigned:
		case ft_date:
		case ft_time:
		case ft_timestamp:
		case ft_mydate:
		{
			switch(len)
			{
			case 1:return &logicalField::compNumber<unsigned char>;
			case 2:return &logicalField::compNumber<unsigned short>;
			case 3:return &logicalField::compNumberU24;
			case 4:return &logicalField::compNumber<unsigned int>;
			case 8:return &logicalField::compNumber<unsigned __int64>;
			}
		}
		case ft_mytime:
		case ft_mydatetime:
		case ft_mytimestamp:
			return &logicalField::compMem;
		case ft_float:
			switch(len)
			{
			case 4:return &logicalField::compNumber<float>;
			case 8:return &logicalField::compNumber<double>;
			}
		case ft_mywchar:
		case ft_wstring:
		case ft_wzstring:
			if (logType & CMPLOGICAL_CASEINSENSITIVE)
				return &logicalField::compiWString;
			if ((type==ft_wstring)||(type==ft_mywchar))
				return &logicalField::compMem;	
			return &logicalField::compWString;
		case ft_lstring: 
		case ft_myvarchar:
		case ft_myvarbinary:
			if (sizeByte==1)
				return &logicalField::compVarString<unsigned char>;
			return &logicalField::compVarString<unsigned short>; 
		case ft_mywvarchar:
		case ft_mywvarbinary:
			if (sizeByte==1)
				return &logicalField::compWVarString<unsigned char>;
			return &logicalField::compWVarString<unsigned short>;
		case ft_mytext:
		case ft_myblob:
			return &logicalField::compBlob;
		}
		return NULL;
	}
#endif
	extResultDef* resultDef() const
	{
		if (opr == 0)
			return (extResultDef*)next();
		return next()->resultDef();
	}
};

struct extRequest
{
	unsigned short	len;
	char			type[2];
	unsigned short	rejectCount;
	unsigned short	logicalCount;
	logicalField	field;

	extResultDef* resultDef()const 
	{
		if (logicalCount)
			return field.resultDef();
		return (extResultDef*)&field;
	}

};

#pragma option -a
pragma_pop

bool isMatch1(int v){return (v==0);}
bool isMatch2(int v){return (v>0);}
bool isMatch3(int v){return (v<0);}
bool isMatch4(int v){return (v!=0);}
bool isMatch5(int v){return (v>=0);}
bool isMatch6(int v){return (v<=0);}

class fields;
class fieldAdapter
{
	logicalField* m_fd;
	fieldAdapter* m_next;
	bool (*m_isMatchFunc)(int);
#ifdef COMP_USE_FUNCTION_POINTER
	compFunc m_compFunc;
#endif
	unsigned char m_keySeg;
	mutable bool m_judge;
	char m_judgeType;
	char m_sizeBytes;
	mutable bool m_matched;
public:
	friend class fields;
	fieldAdapter():	m_keySeg(0xff),m_judge(false)
			,m_judgeType(0),m_sizeBytes(0),m_matched(false){}

	int init(logicalField* fd, position& position, const KEY* key, bool forword)
	{
		m_fd = fd;
		int num = position.getFieldNumByPos(fd->pos);
		if (num == -1)
			return STATUS_INVALID_FIELD_OFFSET;
		m_sizeBytes = (char)position.fieldSizeByte(num);
#ifdef COMP_USE_FUNCTION_POINTER
		m_compFunc = fd->getCompFunc(m_sizeBytes);
#endif
		if (fd->opr == 2)
		{
			m_judgeType = 0;
			return 0;
		}
		if (key)
		{
			int segmentIndex = 0;
			int segments = std::min<uint>(MAX_KEY_SEGMENT, key->user_defined_key_parts);
			while (segmentIndex < segments)
			{
				if (key->key_part[segmentIndex].field->field_index == num)
				{
					eCompType comp = (eCompType)(fd->logType);
					bool gt = (comp == greater)||(comp == greaterEq);
					bool le = (comp == less)||(comp == lessEq);	
					bool valid = !(forword ? gt:le);	
					if (valid)
					{
						m_keySeg = (unsigned char)segmentIndex+1;
						m_judgeType =  (comp == equal) ? 2:1;
					}
					break;
				}
				++segmentIndex;
			}
		}
		return 0;
	}

	inline int checkNomore(bool typeNext, eCompType log)const
	{
		if (m_judge)
		{
			if ((log == equal) && m_matched)//==
				return REC_NOMACTH_NOMORE;
			else if (typeNext && (log == less || log==lessEq))
				return REC_NOMACTH_NOMORE;
			else if (!typeNext && (log == greater || log==greaterEq))
				return REC_NOMACTH_NOMORE;
		}
		return REC_NOMACTH;
	}
	
	int match(const char* record, bool typeNext) const
	{
#ifdef COMP_USE_SWITCHCASE
		int v = m_fd->comp(record, m_sizeBytes);
#else //COMP_USE_FUNCTION_POINTER
		const char* r = (const char*) m_fd->ptr;
		if  (m_fd->logType & CMPLOGICAL_FIELD)
			r = record + m_fd->offset;
		const char* l = record + m_fd->pos;
		int v = (m_fd->*m_compFunc)(l, r , m_sizeBytes);
#endif		
		bool ret = m_isMatchFunc(v);
		if (ret && m_judgeType)
		{
			m_matched = true;
			// check  is this logic range of max ? 
			// if max then set judge node to next logic 
			if ((m_fd->opr != 0) && m_judge && (v == 0) && m_next->m_judgeType)
				m_next->m_judge = true;
		}
		bool end = (m_fd->opr == 0)||(!ret && (m_fd->opr == 1))||(ret && (m_fd->opr == 2));
		if (!end)return m_next->match(record, typeNext);
		return ret ? REC_MACTH : checkNomore(typeNext, (eCompType)(m_fd->logType & 0xF)); 
	}
	
	bool operator<(const fieldAdapter& r)
	{
		if (m_judgeType != r.m_judgeType)
			return m_judgeType > r.m_judgeType;
		return m_keySeg < r.m_keySeg;
	}
};

class fields
{
	fieldAdapter*   m_fields;

public:
	fields():m_fields(NULL){};

	~fields()
	{
		delete [] m_fields;
	}

	void init(extRequest& req, position& position, const KEY* key, bool forword)
	{
		if (req.logicalCount==0) return ;

		logicalField* fd = &req.field;
		delete [] m_fields;
		m_fields = new fieldAdapter[req.logicalCount];
		int lastIndex = req.logicalCount;
		for (int i=0;i<req.logicalCount;++i)
		{
			fieldAdapter& fda = m_fields[i];
			fda.init(fd, position, key, forword);
			eCompType log = (eCompType)(fd->logType & 0xF);
			switch(log) 
			{
			case 1:fda.m_isMatchFunc = isMatch1;break;
			case 2:fda.m_isMatchFunc = isMatch2;break;
			case 3:fda.m_isMatchFunc = isMatch3;break;
			case 4:fda.m_isMatchFunc = isMatch4;break;
			case 5:fda.m_isMatchFunc = isMatch5;break;
			case 6:fda.m_isMatchFunc = isMatch6;break;
			}

			fd = fd->next();
			if (fda.m_fd->opr == 2) 
				lastIndex = i;
		}
		if (key)
		{
			fieldAdapter* begin = &m_fields[0], *cur = &m_fields[0], *end = &m_fields[lastIndex];
			char tmpOpr = (lastIndex != req.logicalCount) ? end->m_fd->opr: 0;
			std::sort(begin, end);
			bool flag = true;
			while (cur != end)
			{
				cur->m_fd->opr = 1;//and
				if (flag && cur->m_judgeType == 2)
					cur->m_judge = true;
				else
					flag = false;
				++cur;
			}

			//if first logic is first segmnet then first logic can judge.
			if ((begin->m_keySeg == 1) && begin->m_judgeType)
				begin->m_judge = true;

			if (lastIndex == req.logicalCount) --end;
			end->m_fd->opr = tmpOpr;
		}
		for (int i=0;i<req.logicalCount-1;++i)
			m_fields[i].m_next = &m_fields[i+1];
	}

	int match(const char* record, bool typeNext) const
	{
		return m_fields[0].match(record, typeNext);
	}
};

class resultWriter
{
	char* m_buf;
	extResultDef* m_def;
	unsigned short m_rowsPos;
	unsigned short m_maxLen;
	unsigned short m_resultLen;
	bool m_writeFirst;

	short writeFirst( position* pos, unsigned int bookmark)
	{
		m_rowsPos = m_resultLen;
		*((unsigned short*)(m_buf + m_resultLen)) = 0; 
		m_resultLen += sizeof(unsigned short);
		return doWrite(pos, bookmark);
	}

	short doWrite(position* pos, unsigned int bookmark)
	{
		// write rowCount	
		unsigned short* rows = (unsigned short*) (m_buf + m_rowsPos);
		++(*rows);

		//write recLength space;
		unsigned short recLen = 0;
		unsigned short recLenPos = m_resultLen;
		*((unsigned short*)(m_buf + m_resultLen)) = recLen; 
		m_resultLen += sizeof(unsigned short);

		//write bookmark
		*((unsigned int*)(m_buf + m_resultLen)) = bookmark; 
		m_resultLen += sizeof(unsigned int);
		
		//if pos ==NULL , that is not found record in a TD_KEY_SEEK_MULTI operation
		// and bookmark has error code also STATUS_NOT_FOUND_TI 
		// in the client, fieldCount > 0 buf recLen=0 then this pattern
		if (pos)
		{
			if ((m_def->fieldCount == 1) && (m_def->field[0].len >= pos->recordLenCl()))
			{	//write whole row
				int len = pos->recordLenCl();
				if (m_maxLen + RETBUF_EXT_RESERVE_SIZE >= m_resultLen + len)
				{
					int maxlen = m_maxLen + RETBUF_EXT_RESERVE_SIZE - m_resultLen;
					len =  pos->recordPackCopy(m_buf + m_resultLen, maxlen);
					if (len == 0)
						return STATUS_BUFFERTOOSMALL;
					m_resultLen += len;
					recLen += len;
				}else
					return STATUS_BUFFERTOOSMALL;
			}else
			{
				//write each fields by field num.
				for (int i=0;i<m_def->fieldCount;i++)
				{
					resultField& fd = m_def->field[i];
					if (m_maxLen+RETBUF_EXT_RESERVE_SIZE>= m_resultLen + fd.len)
					{
						memcpy(m_buf + m_resultLen, pos->fieldPtr(&fd),  fd.len);
						m_resultLen += fd.len;
						recLen += fd.len;
						if (pos->isBlobField(&fd))
							pos->addBlobBuffer(fd.fieldNum);
					}
					else
						return STATUS_BUFFERTOOSMALL;
				}
			}
		}
		//write recLength;
		*((unsigned short*)(m_buf + recLenPos)) = recLen;
		return 0;
	}
public:
	resultWriter(char* buf ,size_t offset,  extResultDef* def, unsigned short maxlen)
			:m_buf(buf),m_def(def),m_rowsPos(0),m_maxLen(maxlen),m_writeFirst(true)
	{
		m_resultLen = (unsigned short)offset;
	}

	short write(position* pos, unsigned int bookmark)
	{
		if (m_writeFirst)
		{
			m_writeFirst = false;
			return writeFirst(pos,  bookmark);
		}
		return doWrite(pos,  bookmark);
	}

	unsigned short resultLen(){return m_resultLen;};
	
	const char* resultBuffer(){return m_buf;}
};


class ReadRecordsHandler : public engine::mysql::IReadRecordsHandler
{
	boost::shared_ptr<resultWriter>	m_writer;
	extRequest*		m_req;
	extResultDef*	m_resultDef;
	position		m_position;
	fields			m_fields;

public:
	short begin(engine::mysql::table* tb, extRequest* req, bool fieldCache
		, char* buf,size_t offset, unsigned short maxlen, bool forword)
	{
		short ret = 0;
		m_position.setTable(tb);
		m_req = req;
		m_resultDef = m_req->resultDef();
		if (fieldCache)
		{
			const KEY* key = NULL;
			if (tb->keyNum() >= 0)
				key = &tb->keyDef(tb->keyNum());
			m_fields.init(*m_req, m_position, key, forword);
		}
		if (m_resultDef->fieldCount > 1)
			ret = convResultPosToFieldNum();
			
		m_writer.reset(new resultWriter(buf, offset, m_resultDef, maxlen));
		//DEBUG_RECORDS_BEGIN(m_resultDef, m_req)
		return ret;
	}

	//TODO This convert is move to client. but legacy app is need this 
	short convResultPosToFieldNum()
	{
		int blobs = 0;
		for (int i=0;i<m_resultDef->fieldCount;i++)
		{
			resultField& fd = m_resultDef->field[i];
			int num = m_position.getFieldNumByPos(fd.pos);
			if (num == -1)
				return STATUS_INVALID_FIELD_OFFSET;
			fd.fieldNum = num; 
			if (m_position.isBlobField(&fd)) 
				++blobs;
			
		}
		
		m_position.setBlobFieldCount(blobs);
		return 0;
	}

	unsigned int end()
	{
		unsigned int len = m_writer->resultLen();
		//DEBUG_RECORDS_END(m_writer.get())

		m_writer.reset();
		return len;
	}

	int match(bool typeNext)const
	{
		if (m_req->logicalCount)
			return m_fields.match(m_position.record(), typeNext);
		return REC_MACTH;
	}

	short write(const unsigned char* bmPtr, unsigned int bmlen, short stat=0)
	{
		unsigned int bookmark = 0;
		//if bmPtr ==NULL , that is not found record in a TD_KEY_SEEK_MULTI operation
		// and set error code to bookmark also STATUS_NOT_FOUND_TI 
		if (bmPtr == NULL)
		{
			bookmark = stat;
			return m_writer->write(NULL, bookmark);
		}
		else
		{
			switch(bmlen)
			{
			case 4:
				bookmark = *((unsigned int*)bmPtr);break;
			case 2:
				bookmark = *((unsigned short*)bmPtr);break;
			case 3:
				bookmark = *((unsigned int*)bmPtr) & 0x0FFFFFF;break;
			case 1:
				bookmark = *((unsigned short*)bmPtr) & 0x0FF;break;
			}
			return m_writer->write(&m_position, bookmark);
		}
		
	}
	unsigned short rejectCount(){return m_req->rejectCount;};
	unsigned short maxRows(){return m_resultDef->maxRows;};
};

}//namespace mysql
}//namespace protocol
}//namespace db
}//namespace tdap
}//namespace bzs

#endif //BZS_DB_PROTOCOL_TDAP_MYSQL_RECORDSETREADER_H
