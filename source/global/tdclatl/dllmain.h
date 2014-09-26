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

class CtdclatlModule : public ATL::CAtlDllModuleT<CtdclatlModule>
{
public:
    DECLARE_LIBID(LIBID_transactd)
    DECLARE_REGISTRY_APPID_RESOURCEID(IDR_TDCLATL,
                                      "{0F94D9C4-FB96-4084-A939-EEC3992454B4}")
};

extern class CtdclatlModule _AtlModule;
