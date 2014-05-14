#ifndef BZS_ENV_TSTRING_H
#define BZS_ENV_TSTRING_H
/*=================================================================
   Copyright (C) 2006 2013 BizStation Corp All rights reserved.

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

#ifndef __TSTRING_H //old tstring.h is used

#include <string.h>
#include <string>
#ifdef _WIN32
#include <mbstring.h>
#endif

#include <tchar.h>
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef LINUX
#include <bzs/env/mbcswchrLinux.h>
#endif


namespace std {
#ifdef _UNICODE
	typedef wstring _tstring;

#else
	typedef string _tstring;

#endif
} //std


#ifdef _UNICODE
	#define tPos Pos
	#define tcout wcout
#else
	#define tPos AnsiPos
	#define tcout cout
#endif


#ifdef _UNICODE
	#define _tcsmcmp _tcscmp
	#define _tcsmclen _tcsclen
	#define _tcsmnextc _tcsnextc
	#define _tcsmrchr _tcsrchr
	#define _tcsmstr _tcsstr
	#define _tcsmupr _tcsupr

	typedef char _NTCHAR;
#else
	#define _tcsmcmp _mbscmp
	#define _tcsmclen _mbslen
	#define _tcsmnextc _mbsnextc
	#define _tcsmrchr _mbsrchr
	#define _tcsmstr _mbsstr
	#define _tcsmupr _mbsupr
	typedef wchar_t _NTCHAR;
#endif

#define __BEGIN_NO_TCHAR_CONVERT__
#define __END_NO_TCHAR_CONVERT__


inline const char* toChar(char* buf, const _TCHAR* w, int size)
{   //If w becomes in Ansi, a pointer will be returned without doing anything.
	#ifdef _UNICODE
		WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, w, -1, buf, size, NULL, NULL);
		return buf;
	#else
		return w;
	#endif
}

inline const char* wtoa(char* buf, const WCHAR* w, int size)
{
	WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, w, -1, buf, size, NULL, NULL);
	return buf;
}

inline const WCHAR* toWChar(WCHAR* buf, const _TCHAR* w, int size)
{//If w becomes in WCHAR, a pointer will be returned without doing anything.
	#ifdef _UNICODE
		return w;
	#else
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, w, -1, buf, size);
		return buf;
	#endif
}


#ifdef _UNICODE
inline const WCHAR* toWChar(WCHAR* buf, const char* a, int size)
{
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, a, -1, buf, size);
	return buf;
}
#endif

inline const WCHAR* toWChar_n(WCHAR* buf, const char* a, int size)
{
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, a, size, buf, size);
	return buf;
}

inline const _TCHAR* toTChar(_TCHAR* t, const WCHAR* w, int size)
{
	 #ifdef _UNICODE
		return w;
	 #else
		WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, w, -1, t, size, NULL, NULL);
		return t;
	 #endif
}

inline const _TCHAR* toTCharCopy(_TCHAR* t, const WCHAR* w, int size)
{ //It returns, after certainly copying, even if w is _TCHAR.
	 #ifdef _UNICODE
		_tcscpy_s(t, size, w);
		return t;
	 #else
		WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, w, -1, t, size, NULL, NULL);
		return t;

	 #endif
}

#pragma warning(disable:4996)
inline const char* toCharCpy(char* buf, const _TCHAR* w, int size)
{   
	 #ifdef _UNICODE
		WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, w, -1, buf, size, NULL, NULL);
		return buf;
	 #else
		strncpy(buf, w, size);
		return buf;
	 #endif
}
#pragma warning(default:4996)

inline const _TCHAR* toTChar(_TCHAR* t, const char* a, int size)
{
	 #ifdef _UNICODE
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, a, -1, t, size);
		return t;
	 #else
		return a;
	 #endif
}

#pragma warning(disable:4996)
inline const _TCHAR* toTCharCopy(_TCHAR* t, const char* a, int size)
{
	 #ifdef _UNICODE
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, a, -1, t, size);
		return t;
	 #else
		_tcsncpy(t, a, size);
		return t;
	 #endif
}
#pragma warning(default:4996)


#endif //__TSTRING_H
#endif //BZS_ENV_TSTRING_H
