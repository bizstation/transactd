#ifndef BZS_ENV_TCHARMINGW_H
#define BZS_ENV_TCHARMINGW_H
/* =================================================================
 Copyright (C) 2000-2014 BizStation Corp All rights reserved.

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
#if __MINGW32__ && defined(__GNUC__)

#ifdef _UNICODE

	#define _tcsncpy_s	wcsncpy_s

#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ <= 5) // 4.5 and lesser
	#define _tcscpy_s	wcscpy_s
	#define _tcscat_s	wcscat_s
	typedef wchar_t		_TUCHAR;
#elseif __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 5) // 4.6 and grater
	#define _tcsstr wcsstr
	#define _tcscmp wcscmp
	#define _tcschr wcschr
	#define _tcsrchr wcsrchr
	#define _tcsncpy wcsncpy
	#define _tcsicmp _wcsicmp
#endif

#else // NOT _UNICODE

	#define _tcsncpy_s	strncpy_s

#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ <= 5) // 4.5 and lesser
	#define _tcscpy_s	strcpy_s
	#define _tcscat_s	strcat_s
	typedef char		_TUCHAR;
#elif __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 5) // 4.6 and grater
	#define _tcsstr strstr
	#define _tcscmp strcmp
	#define _tcschr strchr
	#define	_tcsrchr strrchr
	#define	_tcsncpy strncpy
	#define	_tcsicmp strcasecmp
#endif

#endif // _UNICODE
#endif // __MINGW32__ && defined(__GNUC__)
#endif //BZS_ENV_TCHARMINGW_H
