#ifndef BZS_DB_PROTOCOL_TDAP_CLIENT_SHAREDDATA_H
#define BZS_DB_PROTOCOL_TDAP_CLIENT_SHAREDDATA_H
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
#include <bzs/env/compiler.h>

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


#define MAX_BTRENGIN  50

class nsdatabase;
typedef nsdatabase** (*EnginsFunc)();

#ifdef ARBTREGN_PKG
	PACKAGE void registEnginsPtr(EnginsFunc func);//If use shared dll then you need Call this function. Implemant at trdclengne
#else
	PACKAGE_IMPORT void registEnginsPtr(EnginsFunc func);
	#ifdef ARBTREGN_SHARED_DLL
		PACKAGE nsdatabase** enginsShared();
	#else
		PACKAGE_IMPORT nsdatabase** enginsShared(); //Implemant at ARBtEgn_Shared.dll
    #endif
#endif

}//namespace client
}//namespace tdap
}//namespace protocol
}//namespace db
}//namespace bzs
#endif//BZS_DB_PROTOCOL_TDAP_CLIENT_SHAREDDATA_H
