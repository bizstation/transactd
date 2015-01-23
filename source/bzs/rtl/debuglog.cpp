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

#include "debuglog.h"
#include <bzs/env/fileopen.h>
#include <algorithm>
#include <time.h>

namespace bzs
{
namespace rtl
{

#ifdef DEBUG_PROFILE
boost::timer::cpu_timer g_t;
#endif

static char datebuf[30];

debuglog* debuglog::m_ptr = NULL;
boost::mutex debuglog::m_mutex;
char debuglog::logfilename[MAX_PATH];

const char* dateTimeStr(char* buf, unsigned int bufsize)
{
    struct tm* date;
    time_t now;
    time(&now);
    struct tm tmp;
    date = &tmp;
    localtime_x(date, &now);
    sprintf_s(buf, bufsize, "%04d/%02d/%02d %02d:%02d:%02d",
              date->tm_year + 1900, date->tm_mon + 1, date->tm_mday,
              date->tm_hour, date->tm_min, date->tm_sec);
    return buf;
}

const char* debuglog::dateTime()
{
    return dateTimeStr(datebuf, 30);
}

void debuglog::write(const char* msg)
{
    boost::mutex::scoped_lock lck(m_mutex);
    FILE* fp = fileOpen(logfilename, "a+");
    if (fp)
    {
        fputs(dateTime(), fp);
        fputs(" ", fp);
        fputs(msg, fp);
        fclose(fp);
    }
}

void debuglog::dump(FILE* fp, const char* p, int size, int limit)
{
    size = std::min<int>(size, limit);
    for (int i = 0; i < size; i += 16)
    {
        const char* pp = p + i;
        int max = std::min<int>(size - i, 16);
        for (int j = 0; j < max; j++)
            fprintf(fp, "%02X ", *((unsigned char*)(pp + j)));
        
        if (max != 16)
        {
            for (int j = 0; j < 16 - max; j++)
                fprintf(fp, "   ");
        }
        
        fprintf(fp, " ");
        for (int j = 0; j < max; j++)
        {
            const char* p1 = pp + j;
            if (*p1 >= ' ' && *p1 <= '~')
                fprintf(fp, "%c", *p1);
            else
                fprintf(fp, " ");
        }
        fprintf(fp, "\n");
    }
    fprintf(fp, "\n");
}

void debuglog::writeDump(const char* msg, const char* ptr, int size)
{
    boost::mutex::scoped_lock lck(m_mutex);
    FILE* fp = fileOpen(logfilename, "a+");
    if (fp)
    {
        fputs(dateTime(), fp);
        fputs(" ", fp);
        fputs(msg, fp);
        fputs("\n", fp);
        dump(fp, ptr, size, INT_MAX);

        fclose(fp);
    }
}

} // namespace rtl
} // namespace bzs
