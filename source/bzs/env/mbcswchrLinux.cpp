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

#include "mbcswchrLinux.h"
#include <stdio.h>

namespace bzs
{
namespace env
{
cvt mbcscvt("UTF-16LE", MBC_CHARSETNAME);
cvt wchrcvt(MBC_CHARSETNAME, "UTF-16LE");
cvt u8mbcvt(MBC_CHARSETNAME, UTF8_CHARSETNAME);
cvt mbu8cvt(UTF8_CHARSETNAME, MBC_CHARSETNAME);
cvt u8wccvt("UTF-16LE", UTF8_CHARSETNAME);
cvt wcu8cvt(UTF8_CHARSETNAME, "UTF-16LE");

} // namespace env
} // namespace bzs
