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

class haNotify
{
public:
    virtual ~haNotify() {};
    virtual void onUpdateStaus(int status, const _TCHAR* msg) = 0;
    virtual void setHostName(const _TCHAR* host) = 0;
};

#define HA_NF_ROLE_SLAVE       1
#define HA_NF_ROLE_MASTER      2
#define HA_NF_CANNELNAME       3
#define HA_SLAVE_START         4
#define HA_SLAVE_STOP          5
#define HA_SLAVE_STOP_ALL      6
#define HA_CHANGE_MASTER       7
#define HA_SWITCH_MASTER       8
#define HA_NF_WAIT_TRX_START   9
#define HA_NF_WAIT_TRX_COMP   10 
#define HA_NF_SLAVE_LIST      11
#define HA_NF_PROMOTE_MASTER  12 
#define HA_NF_PROMOTE_CHANNEL 13
#define HA_NF_WAIT_POS_START  14
#define HA_NF_WAIT_POS_COMP   15
#define HA_SET_READ_ONLY      16

void failOrver(const failOverParam& pm, haNotify* nf=NULL);
void switchOrver(const failOverParam& pm, haNotify* nf=NULL);
void demoteToSlave(const failOverParam& pm, haNotify* nf=NULL);
void setEnableFailOver(const failOverParam& pm, bool v);
void setServerRole(const failOverParam& pm, int v);


#endif // GLOBAL_REPLICATION_HACOMMANDH
