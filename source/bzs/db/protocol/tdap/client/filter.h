#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_FILTER_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_FILTER_H
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
#include "table.h"
#include "fields.h"
#include <assert.h>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>


namespace bzs
{
namespace db
{
namespace protocol
{
namespace tdap
{
namespace client
{

#define BOOKMARK_ALLOC_SIZE 40960
#define BOOKMARK_SIZE 4
#define DATASIZE_BYTE 2

#define BTRV_MAX_DATA_SIZE 57000
#define TDAP_MAX_DATA_SIZE 6291456 //3Mbyte



inline ushort_td varlenForFilter(const fielddef& fd)
{
	if (((fd.type >= ft_myvarchar) && (fd.type <= ft_mywvarbinary)) || fd.type == ft_lstring)
		return fd.len < 256 ? 1 : 2;
	else if ((fd.type == ft_myblob) || (fd.type == ft_mytext))
		return fd.len - 8;
	return 0;
}

/** Length of compare
 * if part of string or zstring then return strlen.
 */
inline uint_td compDataLen(const fielddef& fd, const uchar_td* ptr, bool part)
{
	uint_td length = fd.keyDataLen(ptr);
	if (part)
	{
		if ((fd.type == ft_string) || (fd.type == ft_zstring) || (fd.type == ft_note))
			length = (uint_td)strlen((const char*)ptr);
		else if ((fd.type == ft_wstring) || (fd.type == ft_wzstring))
			length = (uint_td)wcslen((const wchar_t*)ptr);
	}
	return length;
}

bool verType(uchar_td type)
{
	if (((type >= ft_myvarchar) && (type <= ft_mywvarbinary)) || type == ft_lstring)
		return true;
	return false;
}


#pragma option -a-
pragma_pack1

struct resultField
{
	unsigned short len;
	unsigned short pos;

	int setParam(table* tb, const _TCHAR* name)
	{
		short fieldNum = tb->fieldNumByName(name);
		if (fieldNum != -1)
		{
			fielddef* fd = &tb->tableDef()->fieldDefs[fieldNum];
			len = fd->len;
			pos = fd->pos;
			return fieldNum;
		}
		return -1;
	}
	unsigned char* writeBuffer(unsigned char* p, bool estimate)
	{
		int n = sizeof(resultField);
		if (!estimate) memcpy(p, this, n);
		return p+n;
	}
};

struct resultDef
{
	resultDef()
	{
		reset();
	}
	void reset()
	{
		maxRows = 0;
		fieldCount = 0;
	}
	unsigned short maxRows;
	unsigned short fieldCount;
	unsigned char* writeBuffer(unsigned char* p, bool estimate)
	{
		int n = sizeof(resultDef);
		if (!estimate) memcpy(p, this, n);
		return p + n;
	}
	friend class filter;
};

struct seek
{
	unsigned short	len;
	unsigned char*  data;

public:
	size_t getLength()
	{
		return sizeof(len) + len;
	}

	//setParam from keyValue
	bool setParam(uchar_td* buf, ushort_td keylen)
	{
		len = keylen;
		data = buf;
		return true;
	}

	unsigned char* writeBuffer(unsigned char* p, bool estimate, bool end, bool isTransactd) const
	{
		int n = sizeof(len);
		if (!estimate)
		{
			if (isTransactd)
				memcpy(p, &len, n);
			else
				n = 0;
			memcpy(p + n, data, len);
		}else if (!isTransactd)
         	n = 0;

		return p  + n + len;
	}

};

struct logic
{
	unsigned char	type;
	unsigned short	len;
	unsigned short	pos;
	unsigned char	logType;
	char			opr;
	unsigned char*  data;

public:
	logic():data(NULL){}

	~logic()
	{
 		delete [] data;
	}

	size_t getLength()
	{
		return sizeof(logic) - sizeof(unsigned char*) + getDatalen();
	}

	void setFieldParam(fielddef* fd )
	{
		type = fd->type;
		len = fd->len;
		pos = fd->pos;
	}

	int getDatalen() const
	{
		if (logType & CMPLOGICAL_FIELD)
			return 2;
		return len;
	}

	bool setCompFiled(table* tb, short index, const _TCHAR* name)
	{
		 short tmp = tb->fieldNumByName(name);
		 if (tmp !=-1)
		 {
			allocBuffer(2);
			fielddef& fd = tb->tableDef()->fieldDefs[tmp];
			memcpy(data, &(fd.pos), 2);
			logType |= CMPLOGICAL_FIELD;
			return true;
		 }
		 return false;
	}

	void allocBuffer(int size)
	{
		 if (data)
			delete [] data;
		 data = new unsigned char[size + 2];
		 memset(data, 0, size + 2);
	}

	void copyToBuffer(table* tb, short index, bool part)
	{
		fielddef* fd = &tb->tableDef()->fieldDefs[index];
		const uchar_td* ptr = (const uchar_td*)tb->fieldPtr(index);
		int varlen = varlenForFilter(*fd);
		int copylen = compDataLen(*fd, ptr, part);
		len = varlen + copylen;
		allocBuffer(len);
		uchar_td* to = (uchar_td*)data;
		if (varlen)
			memcpy(to, ptr, varlen);
		memcpy(to + varlen, fd->keyData(ptr), copylen);

		if (!part && (fd->varLenBytes() || fd->blobLenBytes()))
			logType |= CMPLOGICAL_VAR_COMP_ALL; //match complate
	}

	bool setParam(table* tb, const _TCHAR* name
						, const _TCHAR* type, const _TCHAR* value, char combine, bool compField = false)
	{
		logType = getFilterLogicTypeCode(type);
		opr = combine;
		short fieldNum = tb->fieldNumByName(name);
		if ((logType!=255) && (fieldNum != -1))
		{
			bool ret = true;
			fielddef* fd = &tb->tableDef()->fieldDefs[fieldNum];
			setFieldParam(fd);

			if (compField)
				ret = setCompFiled(tb, fieldNum, value);// value is field name
			else
			{
				fields fds(*tb);
				field fd =  fds[fieldNum];
				fd = value;
				bool part = fd.isCompPartAndMakeValue();
				copyToBuffer(tb, fieldNum, part);
			}
			return ret;
		}
		return false;
	}

	unsigned char* writeBuffer(unsigned char* p, bool estimate, bool end) const
	{
		int n = sizeof(logic) - sizeof(unsigned char*);
		if (!estimate)
		{
			memcpy(p, this, n);
			if (end)
				*(p+n-1) = eCend;
		}
		p += n;

		n = getDatalen();
		if (!estimate) memcpy(p, data, n);
		return p + n;
	}

	bool canJoin(bool after)
	{
		bool flag = true;
		if (after)
			flag = (opr == 1);
		return (flag
				&& (logType == 1)
				&& (type != ft_zstring)
				&& !verType(type));
	}

	bool isNextFiled(logic* src)
	{
		return ((pos + len) ==  src->pos);
	}

	void joinAfter(logic* src)
	{
		assert(src);
		assert(src->data);
		unsigned char* tmp = data;
		data = new unsigned char[len + src->len + 2];
		memcpy(data, tmp, len);
		memcpy(data + len, src->data, src->len);
		len += src->len;
		type = ft_string; //compare by memcmp
		opr = src->opr;
		delete [] tmp;
	}
};

struct header
{
private:
	union
	{
		struct
		{
			unsigned short	len;
			char			type[2];
		};
		struct 
		{
			int ilen  :28;
			int itype : 4;
		};
	};
public:
	unsigned short	rejectCount;
	unsigned short	logicalCount;
	header():rejectCount(1),logicalCount(0),len(0)
	{
		type[0] = 0x00;
		type[1] = 0x00;
	}
	
	void reset()
	{
		rejectCount = 1;
		logicalCount = 0;
		len = 0;
		type[0] = 0x00;
		type[1] = 0x00;
	}

	void setPositionType(bool incCurrent, bool withBookmark, bool isTransactd)
	{
		if (isTransactd)
		{
			itype = incCurrent ? FILTER_CURRENT_TYPE_INC : FILTER_CURRENT_TYPE_NOTINC;
			if (!withBookmark)
				itype |= FILTER_CURRENT_TYPE_NOBOOKMARK;
		}
		else
		{
			if (incCurrent)
			{
				type[0] = 'U';
				type[1] = 'C';
			}
			else
			{
				type[0] = 'E';
				type[1] = 'G';
			}
		}
		
	}

	int bookmarkSize(bool isTransactd) const
	{
		if (isTransactd)
			return (itype & FILTER_CURRENT_TYPE_NOBOOKMARK) ? 0: BOOKMARK_SIZE;
		assert(type[0]);
		//if (type[1] == 'N') return 0;
		return BOOKMARK_SIZE;
	}

	bool positionTypeNext(bool isTransactd) const
	{
		if (isTransactd)
			return !(itype & FILTER_CURRENT_TYPE_INC);
		return (type[0] == 'E');
	}
	
	void setLen(int size, bool isTransactd)
	{
		if (isTransactd) 
			ilen = size; 
		else
			len = size;
	}

	unsigned char* writeBuffer(unsigned char* p, bool estimate) const
	{
		int n = sizeof(header);
		if (!estimate) memcpy(p, this, n);
		return p + n;
	}
};
#pragma option -a
pragma_pop


class recordBackup
{
	char* m_buf;
	table* m_tb;
public:
	recordBackup(table* tb):m_tb(tb)
	{
		m_buf = new char[m_tb->buflen()];
		memcpy(m_buf, m_tb->fieldPtr(0), m_tb->buflen());
	}

	~recordBackup()
	{
		memcpy(m_tb->fieldPtr(0), m_buf, m_tb->buflen());
		delete [] m_buf;
	}
};

class filter
{

	table* m_tb;
	header m_hd;
	resultDef m_ret;
	std::vector<resultField*> m_fields;
	std::vector<short> m_selectFieldIndexes;
	std::vector<logic> m_logics;
	std::vector<seek>  m_seeks;
	uchar_td* m_seeksDataBuffer;

	int m_extendBuflen;
	bool m_ignoreFields;
	bool m_seeksMode;
	bool m_useOptimize;
	bool m_withBookmark;
	size_t m_seeksWritedCount;
	size_t m_logicalLimitCount;
	table::eFindType m_direction;
	bool m_isTransactd;
	inline int maxDataBuffer()
	{
		return m_isTransactd ? TDAP_MAX_DATA_SIZE: BTRV_MAX_DATA_SIZE;
	}

	void addAllFields()
	{
		resultField* r = new resultField();
		r->len = (ushort_td) m_tb->tableDef()->maxRecordLen;
		r->pos = 0;
		m_fields.push_back(r);
	}

	bool addSelect(const _TCHAR* name)
	{
		resultField* r = new resultField();
		int fieldNum = r->setParam(m_tb, name);
		if (fieldNum != -1)
		{
			m_fields.push_back(r);
			m_selectFieldIndexes.push_back(fieldNum);
			return true;
		}
		delete r;
		return false;
	}

	bool setSelect(const std::vector<std::_tstring>& selects)
	{
		for (size_t i=0;i < selects.size();++i)
		{
			if (!addSelect(selects[i].c_str()))
				return false;
		}
		return true;
	}

	bool setWhere(const std::vector<std::_tstring>& where)
	{
		if (where.size() == 0) return true;
		if (where.size() < 3) return false;
		m_logics.resize(m_logics.size() + (where.size() + 1)/4);

		int index = 0;
		for (size_t i=0;i<where.size();i+=4)
		{
			if (i+2 >= where.size())
				return false;
			char combine = eCend;
			std::_tstring value = where[i+2];
			bool compField = (value.size() && (value[0] == _T('[')));
			if (compField)
			{
				value.erase(value.begin());
				value.erase(value.end() - 1);
			}
			if (i+3 < where.size())
			{
				std::_tstring s = where[i+3];
				boost::algorithm::to_lower(s);
				if (s == _T("or")) combine = eCor;
				else if (s == _T("and"))
					combine = eCand;
				else
					return false;
			}
			if (!m_logics[index++].setParam(m_tb, where[i].c_str(), where[i+1].c_str()
									, value.c_str(), combine, compField))
				return false;
		  
		}
		return true;

	}

	inline void setSeekValue(short filedNum, const std::_tstring& s)
	{
		 m_tb->setFV(filedNum, s.c_str());
	}

	inline void setSeekValue(short filedNum, const void* v)
	{
		fielddef& fd = m_tb->tableDef()->fieldDefs[filedNum];
		memcpy(m_tb->fieldPtr(filedNum), v, fd.len);
	}

	template <class vector_type>
	bool setSeeks(const vector_type& keyValues)
	{
		//Check key values
		keydef* kd = &m_tb->tableDef()->keyDefs[m_tb->keyNum()];
		if (keyValues.size() % kd->segmentCount)
			return false;
		//Check uniqe key
		if (kd->segments[0].flags.bit0)
			return false;
		m_seeks.resize(keyValues.size()/kd->segmentCount);
		int maxKeylen = 0;
		for (int j=0;j<kd->segmentCount;++j)
			maxKeylen += m_tb->tableDef()->fieldDefs[kd->segments[j].fieldNum].len + 2;

		// alloc databuffer
		if (m_seeksDataBuffer)
			delete [] m_seeksDataBuffer;

		m_seeksDataBuffer = new uchar_td[maxKeylen*m_seeks.size()];
		memset(m_seeksDataBuffer, 0, maxKeylen*m_seeks.size());
		uchar_td* dataBuf = m_seeksDataBuffer;

		for (size_t i=0;i<keyValues.size();i+= kd->segmentCount)
		{
			for (int j=0;j<kd->segmentCount;++j)
				setSeekValue(kd->segments[j].fieldNum, keyValues[i+j]);
			seek& l = m_seeks[i];
			ushort_td len = m_tb->writeKeyDataTo(dataBuf);
			if (!l.setParam(dataBuf, len))
				return false;
			dataBuf += len;
		}
		m_seeksMode = true;
		m_seeksWritedCount = 0;
		return true;
	}

	bool doSetFilter(const queryBase* q)
	{
		cleanup();
		setRejectCount(q->getReject());
		setMaxRows(q->getLimit());
		m_direction = q->getDirection();
		m_useOptimize = ((q->getOptimize() & queryBase::joinWhereFields) == queryBase::joinWhereFields);
		m_withBookmark = q->isBookmarkAlso();
		recordBackup recb(m_tb);

		if (q->isAll())
			addAllFields();
		else
		{
			if (q->getSelects().size() == 0)
				addAllFields();
			else if (!setSelect(q->getSelects()))
				return false;

			//seeks or where
			if (q->getSeekKeyValues().size() && q->getWheres().size())
				return false;

			if (q->getSeekKeyValues().size())
			{
				//m_withBookmark = true;
				return setSeeks(q->getSeekKeyValues());
			}
			else if (q->getSeekValuesPtr().size())
			{
				//m_withBookmark = true;
				return setSeeks(q->getSeekValuesPtr());
			}
			else if (q->getWheres().size())
				return setWhere(q->getWheres());
		}
		return true;
	}

	int resultRowSize(bool ignoreFields) const
	{

		int recordLen = m_hd.bookmarkSize(m_isTransactd) + DATASIZE_BYTE;
		if (!ignoreFields)
		{
			for (size_t i=0;i< m_fields.size();++i)
				recordLen += m_fields[i]->len;
		}
		return recordLen;
	}

	int calcMaxRows()
	{
		return maxDataBuffer() / resultRowSize(m_ignoreFields);
	}

	int resultBufferNeedSize()
	{
		return (m_ret.maxRows * resultRowSize(m_ignoreFields)) + DATASIZE_BYTE;
	}

	void joinLogic()
	{
		if (m_seeksMode || !m_useOptimize) return;

		for (int i= (int)m_logics.size()-2;i>=0;--i)
		{
		   
			logic& la = m_logics[i+1];
			logic& lb = m_logics[i];
			if (la.canJoin(false) && lb.canJoin(true) && lb.isNextFiled(&la))
			{
				lb.joinAfter(&la);
				//delete la;
				m_logics.erase(m_logics.begin()+i+1);
			}
		}
	}

	int doWriteBuffer(bool estimate)
	{
		unsigned char* p = (unsigned char*)m_tb->dataBak();
		unsigned char* start = p;

		m_hd.logicalCount = (ushort_td)m_logicalLimitCount;
		if (m_ignoreFields)
			m_ret.fieldCount = 0;
		else
			m_ret.fieldCount = (ushort_td)m_fields.size();

		size_t first = 0, last = m_logicalLimitCount;
		if (m_seeksMode)
		{
			first = m_seeksWritedCount;
			last = std::min<size_t>(calcMaxRows() + m_seeksWritedCount, m_logicalLimitCount);
			m_hd.rejectCount = 0;
			m_ret.maxRows = m_hd.logicalCount = (ushort_td)(last - first);
		}
		if (m_ret.maxRows == 0)
			m_ret.maxRows = (unsigned short)std::min<int>(calcMaxRows(), USHRT_MAX);

		p =  m_hd.writeBuffer(p, estimate);
		if (m_seeksMode)
		{
			for (size_t i=first;i< last;++i)
				p = m_seeks[i].writeBuffer(p, estimate, (i==(last-1)), true);
			if (!estimate)
				m_seeksWritedCount += m_hd.logicalCount;

		}else
		{
			for (size_t i=first;i< last;++i)
				p = m_logics[i].writeBuffer(p, estimate, (i==(last-1)));
		}

		p =  m_ret.writeBuffer(p, estimate);

		if (!m_ignoreFields)
		{
			for (size_t i=0;i< m_fields.size();++i)
				p = m_fields[i]->writeBuffer(p, estimate);
		}

		//write total length
		int len = (int)(p - start);
		if (!estimate)
		{
			m_hd.setLen(len, m_isTransactd);
			m_hd.writeBuffer(start, false);
		}
		return len;
	}

	//use seeksMode only
	int calcLogicalCutsize(int oversize)
	{
		int cutsize = 0;
		for (size_t i=m_hd.logicalCount-1;i!=0;--i)
		{
			cutsize += (int)m_seeks[i+m_seeksWritedCount].getLength();
			if (oversize - cutsize < 0)
			{
				m_logicalLimitCount = i;
				return  cutsize;
			}
		}
		return 0;
	}

	bool allocDataBuffer()
	{
		joinLogic();
		m_logicalLimitCount = m_seeksMode ? m_seeks.size() : m_logics.size();
		int len = doWriteBuffer(true);
		if (len > maxDataBuffer())
		{
			if (m_seeksMode)
				len -= calcLogicalCutsize(len - maxDataBuffer() + 1);
			else
				return false;
		}
		//m_hd.len = len;//lost 2byte data at transactd
		int resultLen = (int)resultBufferNeedSize();
		if (resultLen > maxDataBuffer())
		{
			/* change the max rows fit to a max buffer size */
			m_ret.maxRows = calcMaxRows();
			resultLen = resultBufferNeedSize();
		}

		m_extendBuflen = std::max<int>((int)len, resultLen);
		m_extendBuflen = std::max<int>(m_extendBuflen, m_tb->tableDef()->maxRecordLen);
		if (fieldSelected() || m_tb->valiableFormatType())
			m_extendBuflen += m_tb->buflen();

		if ((int)m_tb->buflen() < m_extendBuflen)
		{
			m_tb->setDataBak((void*) realloc(m_tb->dataBak(), m_extendBuflen));
			m_tb->setData(m_tb->dataBak());
		}
		return true;
	}

public:
	filter(table* tb):m_tb(tb),m_ignoreFields(false)
		,m_seeksMode(false),m_seeksWritedCount(0)
		,m_useOptimize(true),m_withBookmark(true)
		,m_seeksDataBuffer(NULL)
	{
		m_isTransactd = m_tb->isUseTransactd();
	}

	~filter()
	{
		cleanup();
		
	}

	void cleanup()
	{
		for (size_t i=0;i < m_fields.size();++i)
			delete m_fields[i];
		m_selectFieldIndexes.clear();
		m_fields.clear();
		m_logics.clear();
		m_seeks.clear();
		m_hd.reset();
		m_ret.reset();
		m_ignoreFields = false;
		m_seeksMode = false;
		m_seeksWritedCount = 0;
		m_useOptimize = true;
		delete [] m_seeksDataBuffer;
		m_seeksDataBuffer = NULL;
	}

	bool setQuery(const queryBase* q)
	{
		 bool ret = doSetFilter(q);
		if (!ret)
			cleanup();
		return ret;
	}

	bool isWriteComleted() const
	{
		if (!m_seeksMode) return true;
		return (m_seeksWritedCount == m_seeks.size());
	}
	void resetSeeksWrited(){m_seeksWritedCount = 0;}

	void setPositionType(bool incCurrent)
	{
		m_hd.setPositionType(incCurrent, m_withBookmark, m_isTransactd);
	}

	bool positionTypeNext() const{return m_hd.positionTypeNext(m_isTransactd);}

	void setRejectCount(ushort_td v){m_hd.rejectCount = v;}
	ushort_td rejectCount()const {return m_hd.rejectCount;}

	void setMaxRows(ushort_td v){m_ret.maxRows = v;}
	ushort_td maxRows()const {return m_ret.maxRows;}

	ushort_td recordCount()const {return maxRows();}

	void setPosTypeNext(bool v){setPositionType(!v);}

	uint_td exDataBufLen() const
	{
		if (fieldSelected() || m_tb->valiableFormatType())
			return m_extendBuflen - m_tb->buflen();
		return m_extendBuflen;
	}

	void init(table* pBao){};

	ushort_td fieldCount() const {return m_ret.fieldCount;}

	void setFieldCount(ushort_td v){m_ret.fieldCount = v;}

	ushort_td fieldLen(int index) const
	{
		assert(index < (int)m_fields.size());
		return m_fields[index]->len;
	}

	ushort_td totalFieldLen() const
	{
		return resultRowSize(false) - m_hd.bookmarkSize(m_isTransactd) - DATASIZE_BYTE;
	}

	ushort_td totalSelectFieldLen() const
	{
		ushort_td recordLen = 0;
		for (size_t i=0;i< m_fields.size();++i)
			recordLen += m_fields[i]->len;
		return recordLen;
	}

	ushort_td fieldOffset(int index) const
	{
		assert(index < (int)m_fields.size());
		return m_fields[index]->pos;
	}

	bool writeBuffer()
	{
		if (allocDataBuffer())
			return (doWriteBuffer(false) > 0);
		return false;
	}

	ushort_td extendBuflen() const{return m_extendBuflen;}

	bool fieldSelected() const
	{
		return !((m_fields.size() == 1) 
			&& (m_fields[0]->pos == 0) && (m_fields[0]->len == (ushort_td) m_tb->tableDef()->maxRecordLen));
	}

	bool ignoreFields() const {return m_ignoreFields;}

	int bookmarkSize() const {return m_hd.bookmarkSize(m_isTransactd);}

	/* The Ignore fields option don't use with multi seek operation.
	   because if a server are not found a record then a server return
	   error code in a bookmark field.
	*/
	void setIgnoreFields(bool v){m_ignoreFields = v;}
	bool isSeeksMode()const {return m_seeksMode;}
	table::eFindType direction() const{return m_direction;}
	void setDirection(table::eFindType v) {m_direction = v;}
	const std::vector<short>& selectFieldIndexes(){return m_selectFieldIndexes;}
	const std::vector<seek>& seeks() const {return m_seeks;}
};





}// namespace client
}// namespace tdap
}// namespace protocol
}// namespace db
}// namespace bzs
#endif//BZS_DB_PROTOCOL_TDAP_CLIENT_FILTER_H
