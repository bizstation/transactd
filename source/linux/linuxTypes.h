#ifndef	LINUX_TYPES_H
#define LINUX_TYPES_H
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



#ifndef WCHAR
#	if (!(__APPLE__ && (__BORLANDC__ || __clang__)))
		typedef unsigned short char16_t;
#	endif
	typedef char16_t WCHAR;

#endif


#endif	//LINUX_TYPES_H

