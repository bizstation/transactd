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
#pragma hdrstop
#include "sharedData.h"

#pragma package(smart_init)

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

static nsdatabase* g_engins[MAX_BTRENGIN] = { 0 };

nsdatabase** enginsInternal()
{
    return g_engins;
}

#ifdef ARBTREGN_SHARED_DLL
nsdatabase** enginsShared()
{
    return g_engins;
}
#endif

EnginsFunc engins = enginsInternal;

} // namespace client
} // namespace tdap
} // namespace protocol
} // namespace db
} // namespace bzs
