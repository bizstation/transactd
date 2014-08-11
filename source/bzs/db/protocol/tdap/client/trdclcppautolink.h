#ifndef BZS_RTL_TRDCLCPPAUTOLINK_H
#define BZS_RTL_TRDCLCPPAUTOLINK_H
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

/* For windows client application programs
If you need auto link then you define TRDCL_AUTOLINK and assign
library path in project option.

*/

#ifdef TRDCL_AUTOLINK
#	include <bzs/env/compiler.h>
#	include <bzs/db/protocol/tdap/tdapcapi.h>
#	define TD_CPP_LIB_NAME LIB_PREFIX TD_CPP_LIB_PRE CPP_INTERFACE_VERSTR SHARED_LIB_EXTENTION
#	pragma comment(lib, TD_CPP_LIB_NAME)
#endif //TRDCL_AUTOLINK

#endif //BZS_RTL_TRDCLCPPAUTOLINK_H