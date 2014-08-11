#ifndef	CHARSET_CONVERT_H
#define CHARSET_CONVERT_H

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

#include <iconv.h>
#include <string.h>
#include <wchar.h>
#include <linuxTypes.h>

inline size_t strlen16(const char16_t* src)
{
	const char16_t* p = src;
	while(*p)
		++p;
	return p - src; 
}
	
#define ICONV_NO_INIT   -1
#define ICONV_SUCCESS   0
#define ICONV_OPENRRROR 1
class cvt
{
	iconv_t m_cd;
	int m_stat;
	bool m_isOpend;
	
public:
	cvt():m_stat(ICONV_NO_INIT),m_isOpend(false)
	{
		
	}
	
	cvt(const char* to, const char* from)
		:m_stat(ICONV_NO_INIT),m_isOpend(false)
	{
		 setCharset(to, from);	
	}
	
	~cvt()
	{
		if (m_isOpend)
			iconv_close(m_cd);
	}
	
	int setCharset(const char* to, const char* from)
	{
		m_cd = iconv_open(to, from);
		if (m_cd == (iconv_t)-1) 
			m_stat = ICONV_OPENRRROR;
		else
			m_stat = ICONV_SUCCESS;
		m_isOpend = (m_stat == ICONV_SUCCESS);
		return m_stat;
	}
	
	size_t conv(const char* src, size_t inszie, char* outbuf, size_t outbufsize)
	{
		if ((int)inszie == -1)
			inszie = strlen(src);
		char* pout = outbuf;
		
		size_t len = iconv(m_cd, (char**)&src, &inszie, &pout, &outbufsize);
	   	if (outbufsize)
	   		*pout = 0x00;
	   	return pout - outbuf;
	}
	
	size_t conv(const char* src, size_t inszie, char16_t* outbuf, size_t outbufsize)
	{
		if ((int)inszie == -1)
			inszie = strlen(src);
		char* pout = (char*)outbuf;
		size_t len = iconv(m_cd, (char**)&src, &inszie, &pout, &outbufsize);
	   	
	   	if (outbufsize)
	   		*pout = 0x00;
	   	return (char16_t*)pout - outbuf;
	}
	
	size_t conv(const char16_t* src, size_t inszie, char* outbuf, size_t outbufsize)
	{
		if ((int)inszie == -1)
			inszie = strlen16(src)*sizeof(char16_t);
		char* pout = outbuf;
		size_t len = iconv(m_cd, (char**)&src, &inszie, &pout, &outbufsize);
	   	if (outbufsize)
	   		*pout = 0x00;
	   	return pout - outbuf;
	}

};

#endif	//CHARSET_CONVERT_H

