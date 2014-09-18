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
#ifndef PERCENTAGEKEY_H
#define PERCENTAGEKEY_H

#include "mysqlInternal.h"

namespace bzs
{
namespace db
{
namespace engine
{
namespace mysql
{


/** return value of setKeyValueByPer function
 *
 */
#define KEY_COPY_SUCCESS		0
#define KEY_ALL_SGMENTS_SAME	1
#define KEY_NEED_SGMENT_COPY	2

class percentageKey
{
	uchar* m_first;
	uchar* m_last;
	uchar* m_current;
	const KEY& m_key;
	double m_period; 
	int m_lastCompSize;
	int m_lastCompPos;
	unsigned short* m_varlenPtr;
	const KEY_PART_INFO* m_curseg;
	
	const unsigned char* rev(const unsigned char* p, unsigned char* buf, int size)
	{
		for (int i=0;i<size;i++)
			buf[i] = p[size - i - 1];
		return buf;
	}
	
	template <typename T>
	bool setKeyValueByPerPos(unsigned short per)
	{
		m_lastCompSize = sizeof(T);
		double k = (*((T*)m_last) - *((T*)m_first))/(double)10000;
		T v =  (T)((per * k) + *((T*)m_first) + m_period);
		
		memcpy(m_current, (unsigned char*)&v, m_lastCompSize); 
		return ((*((T*)m_last) - *((T*)m_first)) == 1);
	}
	
	template <class T>
	bool setKeyValueByPerPosRev(unsigned short per)
	{
		uchar buf1[10];
		uchar buf2[10];
		m_lastCompSize = sizeof(T);
		const unsigned char* fr = rev(m_first, buf1, m_lastCompSize);
		const unsigned char* lr = rev(m_last, buf2, m_lastCompSize);
		double k = (*((T*)lr) - *((T*)fr))/(double)10000;
		T v =  (T)((per * k) + *((T*)fr) + m_period);
		
		memcpy(m_current, rev((const unsigned char*)&v, buf1, m_lastCompSize), m_lastCompSize);
		
		return  ((*((T*)fr) - *((T*)lr))==1);
	}
	
	void movePosition(int size)
	{
		m_first += size;
		m_last += size;
		m_current += size;
		m_lastCompPos += size;
	}
	
	/** if all segment is same return false
	 *
	 */
	bool seekDifferentKey()
	{
		int pos = 0;
		for (int i=0;i<(int)m_key.user_defined_key_parts;i++)
		{
			m_curseg = &m_key.key_part[i];
			if (memcmp(m_first + pos, m_last + pos, m_curseg->store_length))
				break;
			pos +=  m_curseg->store_length; 
		}
		if (pos)
		{
			memcpy(m_current, m_first, pos); //A key value is the same value so far.
			movePosition(pos);
		}
		return (pos != m_key.key_length);
	}
	
public:
	percentageKey(const KEY& key, uchar* first, uchar* last, uchar* current)
		:m_first(first),m_last(last),m_current(current),m_key(key),m_curseg(&key.key_part[0])
	{

	}
	
	void reset(uchar* first, uchar* last, uchar* current)
	{
		m_first = first;
		m_last = last;
		m_current = current;
		m_curseg = &m_key.key_part[0];
		m_lastCompPos = 0;
	}

	/**  
	 *	 @return 0 succes, 1 allsegmnet are same, 2 need segment copy
	 */
	int setKeyValueByPer(unsigned short per, bool forwoard)
	{
		m_period = (forwoard) ? 0.5f:-0.5f;
		if(!seekDifferentKey())
			return KEY_ALL_SGMENTS_SAME;

		int len = m_curseg->store_length;
		if (m_curseg->null_bit)
		{
			--len;
			movePosition(1);
		}
		
		bool needCopy;	
		switch(m_curseg->field->key_type())
		{
		case HA_KEYTYPE_FLOAT:
		case HA_KEYTYPE_DOUBLE:
		{
			if (len ==4)
				needCopy = setKeyValueByPerPos<float>(per);
			else if (len ==8)
				needCopy = setKeyValueByPerPos<double>(per);
			break;
		}
		case HA_KEYTYPE_USHORT_INT:
		case HA_KEYTYPE_ULONG_INT:
		case HA_KEYTYPE_ULONGLONG:
		{
			switch(len)
			{
			case 1:needCopy = setKeyValueByPerPos<unsigned char>(per);break;
			case 2:needCopy = setKeyValueByPerPos<unsigned short>(per);break;
			case 4:needCopy = setKeyValueByPerPos<unsigned int>(per);break;
			case 8:needCopy = setKeyValueByPerPos<unsigned __int64>(per);break;
			}
			break;
		}
		case HA_KEYTYPE_INT8:
		case HA_KEYTYPE_SHORT_INT:
		case HA_KEYTYPE_LONG_INT:
		case HA_KEYTYPE_LONGLONG:
		{
			switch(len)
			{
			case 1:needCopy = setKeyValueByPerPos<char>(per);break;
			case 2:needCopy = setKeyValueByPerPos<short>(per);break;
			case 4:needCopy = setKeyValueByPerPos<int>(per);break;
			case 8:needCopy = setKeyValueByPerPos<__int64>(per);break;
			}
			break;
		}
		case HA_KEYTYPE_VARTEXT1:
		case HA_KEYTYPE_VARBINARY1:
		case HA_KEYTYPE_VARTEXT2:
		case HA_KEYTYPE_VARBINARY2:
			
			len = m_curseg->length;
			m_varlenPtr = (unsigned short*)m_current;
			*m_varlenPtr = len;
			movePosition(2);
		case HA_KEYTYPE_TEXT:
		case HA_KEYTYPE_BIT:
		case HA_KEYTYPE_BINARY:
		{
	
			//It copies until every 1 byte of values differ, 
			//and the following 2 bytes are made into the order of reverse,
			//and it processes as unsigned short. 
			for (int i=0;i<len;i++)
			{
				if (*m_first == *m_last)
				{
					*m_current = *m_first;
					movePosition(sizeof(char));
					
				}
				else
				{
					int size = (len - i);
					
					if (size >= 8)
						needCopy = setKeyValueByPerPosRev<unsigned __int64>(per);
					else if (size >= 4)
						needCopy = setKeyValueByPerPosRev<unsigned int>(per);
					else if (size >= 2)
						needCopy = setKeyValueByPerPosRev<unsigned short>(per);
					else
						needCopy = setKeyValueByPerPos<unsigned char>(per);
					break;
				}
			}
			
			break;
		}		
		case HA_KEYTYPE_INT24:
		case HA_KEYTYPE_UINT24:
		case HA_KEYTYPE_NUM:
		case HA_KEYTYPE_END:
			//no support
			break;
		}
		if (!forwoard && needCopy)
			return KEY_NEED_SGMENT_COPY; 
		return KEY_COPY_SUCCESS;
	}

	void copyFirstDeferentSegment()
	{
		
		memcpy(m_last, m_first, m_lastCompSize);
		//After area that pad by ff;
		memset(m_last + m_lastCompSize, 0xff, m_key.key_length - m_lastCompSize - m_lastCompPos);

	}

};

}//namespace mysql
}//namespace engine
}//namespace db
}//namespace bzs


#endif //PERCENTAGEKEY_H

