#ifndef BZS_RL_DEBUGLOG_H
#define BZS_RL_DEBUGLOG_H
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
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <stdio.h>
#include <bzs/env/crosscompile.h>
#include <time.h>
#include <boost/timer/timer.hpp>
#ifdef _WIN32
#include <windows.h>

#endif

namespace bzs
{
namespace rtl
{

class debuglog : private boost::noncopyable
{
    static debuglog* m_ptr;

protected:
    static boost::mutex m_mutex;
    static char logfilename[MAX_PATH];

public:
    virtual ~debuglog(){};
    virtual void stop() = 0;
    void write(const char* msg);
    void writeDump(const char* msg, const char* ptr, int size);
    static void regist(debuglog* ptr) { m_ptr = ptr; };
    static debuglog* get() { return m_ptr; }
    static void dump(FILE* fp, const char* p, int size, int limit);
    static void init();
    static const char* dateTime();
};

#ifdef DEBUG_LOG
#define DEBUG_WRITELOG(MSG) debuglog::get()->write(MSG);

#else // DEBUG_LOG
#define DEBUG_WRITELOG(MSG)
#endif // DEBUG_LOG

#if defined(DEBUG_PROFILE) || defined(DEBUG_LOG)
#define DEBUG_PROFILE_INIT() bzs::rtl::debuglog::get()->init();
#define DEBUG_PROFILE_DEINIT() bzs::rtl::debuglog::get()->stop();
#else
#define DEBUG_PROFILE_INIT()
#define DEBUG_PROFILE_DEINIT()
#endif

#ifdef DEBUG_PROFILE
extern boost::timer::cpu_timer g_t;

#define DEBUG_PROFILE_START(V)                                                 \
    if (V)                                                                     \
        bzs::rtl::g_t.start();

#define DEBUG_PROFILE_END(V, NAME)                                             \
    if (V)                                                                     \
    {                                                                          \
        bzs::rtl::g_t.stop();                                                  \
        char buf[256];                                                         \
        sprintf(buf, "%-20s %s\r\n", NAME, bzs::rtl::g_t.format().c_str());    \
        bzs::rtl::debuglog::get()->write(buf);                                 \
    };

#define DEBUG_PROFILE_END_OP(V, IV)                                            \
    if (V)                                                                     \
    {                                                                          \
        bzs::rtl::g_t.stop();                                                  \
        char buf[256];                                                         \
        sprintf(buf, "op = %-15ld %s\r\n", IV,                                 \
                bzs::rtl::g_t.format().c_str());                               \
        bzs::rtl::debuglog::get()->write(buf);                                 \
    };

#else // DEBUG_PROFILE

#define DEBUG_PROFILE_START(V)
#define DEBUG_PROFILE_END(V, NAME)
#define DEBUG_PROFILE_END_OP(V, IV)

#endif // DEBUG_PROFILE

} // namespace rtl
} // namespace bzs
#endif // BZS_RL_DEBUGLOG_H
