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

#pragma hdrstop
#include "benchmark.h"
#include <tchar.h>
#include <stdio.h>

#define BOOST_ERROR_CODE_HEADER_ONLY
#define BOOST_CHRONO_INLINED


#pragma package(smart_init)

namespace bzs
{
namespace rtl
{

boost_timer benchmark::t;
#ifdef BOOST_HIGH_RESOL_TIMER_ENABLE
boost_timer::time_point  benchmark::m_start;
#endif
void benchmark::showTimeSec(bool result, const char* name)
{
	if (result == true)
	{
		#ifdef BOOST_CPUTIMER_ENABLE
		boost::timer::cpu_times elapsed = t.elapsed();
		printf("%d msec %s\n", (int)(elapsed.wall/1000000), name);
		printf("%s\r\n", boost::timer::format(elapsed).c_str());
		#else
		#ifdef BOOST_HIGH_RESOL_TIMER_ENABLE
			boost_timer::time_point p = boost_timer::now();
			boost::chrono::nanoseconds ns  = p - m_start;
			printf("%d msec %s\n", (int)(ns.count()/1000000) , name);
		#else
			printf("%d msec %s\n", (int)(benchmark::t.elapsed()*1000), name);
		#endif
		#endif
	}
	else
		printf("Erorr %s\n", name);
}

int  benchmark::stop()
{
	#ifdef BOOST_CPUTIMER_ENABLE
	boost::timer::cpu_times elapsed = t.elapsed();
	return (int)(elapsed.wall/1000000);
	#else
		#ifdef BOOST_HIGH_RESOL_TIMER_ENABLE
			boost_timer::time_point p = boost_timer::now();
			boost::chrono::nanoseconds ns  = p - m_start;
			return (int)(ns.count()/1000000;
		#else
			return  (int)(t.elapsed()*1000);
		#endif
	#endif
}

void benchmark::showTimes(int result, const char* name)
{
	printf("------------------------------\n");
	printf("%s%.4f(sec)\n", name, ((double)result)/1000 );
}


bool benchmark::report(boost::function<bool()> func, const char* name)
{
	start();

	bool ret = func();
	showTimeSec(ret, name);
	return ret;
}

void benchmark::report2(boost::function<void()> func, const char* name)
{
  
	start();
	func();
	showTimeSec(1, name);

}

void benchmark::start()
{
   
	#ifdef BOOST_CPUTIMER_ENABLE
	t.stop();
	t.start();
	#else
		#ifdef BOOST_HIGH_RESOL_TIMER_ENABLE
		m_start = boost_timer::now();
		#else
		t.restart();
		#endif
	#endif
}


void benchmarkMt::start()
{

	#ifdef BOOST_CPUTIMER_ENABLE
	t.stop();
	t.start();
	#else
		#ifdef BOOST_HIGH_RESOL_TIMER_ENABLE
		m_start = boost_timer::now();
		#else
		t.restart();
		#endif
	#endif
}


int benchmarkMt::end()
{
	#ifdef BOOST_CPUTIMER_ENABLE
	boost::timer::cpu_times elapsed = t.elapsed();
	return (int)(elapsed.wall/1000);
	#else
		#ifdef BOOST_HIGH_RESOL_TIMER_ENABLE
			boost_timer::time_point p = boost_timer::now();
			boost::chrono::nanoseconds ns  = p - m_start;
			return (int)(ns.count()/1000;
		#else
			return  (int)(t.elapsed()*1000000);
		#endif
	#endif
}

}//namespace rtl
}//namespace bzs
