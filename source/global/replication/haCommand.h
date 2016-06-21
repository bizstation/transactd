#ifndef GLOBAL_REPLICATION_HACOMMANDH
#define GLOBAL_REPLICATION_HACOMMANDH
/*=================================================================
   Copyright (C) 2016 BizStation Corp All rights reserved.

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
#include "replCommand.h"

#define OPT_SO_AUTO_SLVAE_LIST      1
#define OPT_FO_FAILOVER             2
#define OPT_DISABLE_OLD_TO_SLAVE    4
#define OPT_READONLY_CONTROL        8

struct failOverParam
{
    mutable int option;
    mutable std::_tstring slaves;
    masterNode newMaster;
    node master;
    std::string portMap;
    failOverParam() : option(0){} 
    bool isFailOver() const {return (option & OPT_FO_FAILOVER) != 0;}
    bool isSwitchOver() const {return !isFailOver();}
    bool isDisableOldMasterToSalve() const 
        {return (option & OPT_DISABLE_OLD_TO_SLAVE) != 0;}
};

void failOrver(const failOverParam& pm);
void switchOrver(const failOverParam& pm);
void masterToSlave(const failOverParam& pm);
void setEnableFailOver(const failOverParam& pm, bool v);
void setServerRole(const failOverParam& pm, int v);


#endif // GLOBAL_REPLICATION_HACOMMANDH
