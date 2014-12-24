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
#include <pthread.h>
#include <bzs/env/crosscompile.h>
#include <assert.h>


#define CVT_COUNT 6

namespace bzs
{
namespace env
{

tls_key g_tls1=0;

void cleanupTls(void* p)
{
    delete [] ((cvt*)p);
}

void initCvtProcess()
{
    int ret = pthread_key_create(&g_tls1, cleanupTls);
    assert(ret == 0);
}

void deinitCvtProcess()
{
    pthread_key_delete(g_tls1);
}

cvt& getCvt(int index)
{
    cvt* p = (cvt*)pthread_getspecific(g_tls1);
    if (p == NULL)
    {
        p = new cvt[CVT_COUNT];
        p[0].setCharset("UTF-16LE", MBC_CHARSETNAME);
        p[1].setCharset(MBC_CHARSETNAME, "UTF-16LE");
        p[2].setCharset(MBC_CHARSETNAME, UTF8_CHARSETNAME);
        p[3].setCharset(UTF8_CHARSETNAME, MBC_CHARSETNAME);
        p[4].setCharset("UTF-16LE", UTF8_CHARSETNAME);
        p[5].setCharset(UTF8_CHARSETNAME, "UTF-16LE");
        pthread_setspecific(g_tls1, p);
    }
    return p[index];
}


} // namespace env
} // namespace bzs
