#ifndef BZS_DB_PROTOCOL_TDAP_FIELD_COMP_H
#define BZS_DB_PROTOCOL_TDAP_FIELD_COMP_H
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
#include <bzs/db/protocol/tdap/tdapcapi.h>
#include <bzs/env/crosscompile.h>
#include <algorithm>
#include <string.h>


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
	int tmp = std::min<int>(llen, rlen);
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
	int tmp = std::min<int>(llen, rlen);
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
	int tmp = std::min<int>(llen, rlen);
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


#endif
