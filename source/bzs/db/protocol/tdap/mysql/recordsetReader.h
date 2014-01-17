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
int compare(const char* l, const char* r)
{
	if (*((T*)l) < *((T*)r))
		return -1;
	else if (*((T*)l) > *((T*)r))
		return 1;
	return 0;
}

template <class T>
int compare(T l, T r)
{
	if (l < r)
		return -1;
	else if (l > r)
		return 1;
	return 0;
}

template <class T>
int compareVartype(const char* l, const char* r, bool bin, char logType)
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
int compareWvartype(const char* l, const char* r, bool bin, char logType)
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

#define MAX_ISINDEX_CACHE	8
#define REC_MACTH			0
#define REC_NOMACTH			1
#define REC_NOMACTH_NOMORE  2




struct extRequest;
class fieldInfoCache
{
	char m_sizeBytes[1000];
	bool m_isIndex[MAX_ISINDEX_CACHE];
	mutable int m_index;
	
public:
	inline fieldInfoCache();
	inline ~fieldInfoCache();
	inline short cache(extRequest& req, position& position, const KEY* key);
	inline int getPos()const;
	inline bool isIndex()const;
	inline void reset()const;

};


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
		return (logType & 64)?(logicalField*)(ptr + 2):(logicalField*)(ptr + len/*std::max((unsigned short)2,len)*/);
	}

private:	
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
	bool matchThis(const char* record, int sizeByte) const
	{
		
		int v = comp(record, sizeByte);
		switch(logType & 0xF) //16 or more are disregarded. 
		{
		case 1:return (v==0);	//==
		case 2:return (v>0);	//>
		case 3:return (v<0);	//<
		case 4:return (v!=0);	//!=
		case 5:return (v>=0);	//>=
		case 6:return (v<=0);	//<= 		
		}			
		return false;
	}
public:
	int checkNomore(bool matchResult, bool typeNext, const fieldInfoCache& sb)const
	{
		if (matchResult)
			return REC_MACTH;
		else if(sb.isIndex())
		{
			char log = logType & 0xF;
			if (log == 1)//==
				return REC_NOMACTH_NOMORE;
			else if (typeNext && (log == 3 || log==6))
				return REC_NOMACTH_NOMORE;
			else if (!typeNext && (log == 2 || log==5))
				return REC_NOMACTH_NOMORE;
		}
		return REC_NOMACTH;
	}
	int match(const char* record, bool typeNext, const fieldInfoCache& sb) const
	{
		bool ret = matchThis(record, sb.getPos());
		if (opr == 0) //this is last
			return checkNomore(ret, typeNext, sb); 
		if (!ret)
			return (opr == 1)?checkNomore(ret, typeNext, sb):next()->match(record, typeNext, sb); 
		else 
			return (opr == 1)?next()->match(record, typeNext, sb):checkNomore(ret, typeNext, sb); 
	}
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
	int match(const char* record, bool typeNext, const fieldInfoCache& sb)const
	{
		if (logicalCount)
			return field.match(record, typeNext, sb);
		return REC_MACTH;
	}
	extResultDef* resultDef()const 
	{
		if (logicalCount)
			return field.resultDef();
		return (extResultDef*)&field;
	}

};

#pragma option -a
pragma_pop

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
	
		memset(m_buf + m_resultLen, 0x00,  sizeof(unsigned short));
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
		memcpy(m_buf + m_resultLen, (const char*)&recLen,  sizeof(unsigned short));
		m_resultLen += sizeof(unsigned short);
		//write bookmark
		memcpy(m_buf + m_resultLen, (const char*)&bookmark,  sizeof(unsigned int));
		m_resultLen += sizeof(unsigned int);
		
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
					//memcpy(m_buf + m_resultLen, pos->record() + fd.pos,  fd.len);
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
		//write recLength;
		unsigned short* tmp = (unsigned short*) (m_buf + recLenPos);
		*tmp = recLen;
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

/* ---------------------------------------------------------------
 *   Implement fieldInfoCache
 * ---------------------------------------------------------------*/
inline fieldInfoCache::fieldInfoCache()
{
	memset(m_isIndex, 0, sizeof(bool) * MAX_ISINDEX_CACHE);	
}

inline fieldInfoCache::~fieldInfoCache()
{
	
}

inline short fieldInfoCache::cache(extRequest& req, position& position, const KEY* key)
{
	
	m_index = 0;
	logicalField* fd = &req.field;
	char* pos = m_sizeBytes;
	bool isCheckKeyseg = (key != NULL);
	unsigned short segmentIndex = 0;
	for (int i=0;i<req.logicalCount;i++)
	{
		
		int num = position.getFieldNumByPos(fd->pos);
		if (num == -1)
			return STATUS_INVALID_FIELD_OFFSET;
		*pos = (char)position.fieldSizeByte(num);

		/* Is target field  current keynum segnmnt 
		   For optimize match() return NOMATCH_NOMORE
		*/ 
		if (isCheckKeyseg && (i < MAX_ISINDEX_CACHE))
		{
			if (segmentIndex < key->user_defined_key_parts)
			{
				m_isIndex[i] = (key->key_part[segmentIndex].field->field_index == num);
				if (!m_isIndex[i] && (++segmentIndex < key->user_defined_key_parts))
					m_isIndex[i] = (key->key_part[segmentIndex].field->field_index == num);
			}
			isCheckKeyseg = m_isIndex[i];
			if (fd->opr == 2) isCheckKeyseg = false;
		}

		++pos;
		fd = fd->next();
	}
	return 0;
}

/* get value and inc index.
*/
inline int fieldInfoCache::getPos()const
{
	return m_sizeBytes[m_index++];
}

/* It certainly calls after getPos() */
inline bool fieldInfoCache::isIndex()const
{
	return m_isIndex[m_index-1];
}

/* reset for next record */
inline void fieldInfoCache::reset()const
{
	m_index = 0;
	
}

/* ---------------------------------------------------------------*/

class ReadRecordsHandler : public engine::mysql::IReadRecordsHandler
{
	boost::shared_ptr<resultWriter>	m_writer;
	extRequest*		m_req;
	extResultDef*	m_resultDef;
	position		m_position;
	fieldInfoCache  m_fieldInfoCache;
public:

	short begin(engine::mysql::table* tb, extRequest* req, char* buf,size_t offset, unsigned short maxlen)
	{
		short ret = 0;
		m_position.setTable(tb);
		m_req = req;
		const KEY* key = NULL;
		if (tb->keyNum() >= 0)
			key = &tb->keyDef(tb->keyNum());
		m_fieldInfoCache.cache(*m_req, m_position, key);

		m_resultDef = m_req->resultDef();
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
		m_fieldInfoCache.reset();
		return m_req->match(m_position.record(), typeNext, m_fieldInfoCache);
	}

	short write(const unsigned char* bmPtr, unsigned int bmlen)
	{
		unsigned int bookmark = 0;
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
	unsigned short rejectCount(){return m_req->rejectCount;};
	unsigned short maxRows(){return m_resultDef->maxRows;};
};

}//namespace mysql
}//namespace protocol
}//namespace db
}//namespace tdap
}//namespace bzs

#endif //BZS_DB_PROTOCOL_TDAP_MYSQL_RECORDSETREADER_H
