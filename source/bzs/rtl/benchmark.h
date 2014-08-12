#ifndef benchmarkH
#define benchmarkH
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
#include <boost/function.hpp>
#include <boost/version.hpp>

//#define BOOST_HIGH_RESOL_TIMER_ENABLE

#if BOOST_VERSION > 104801
	
	#ifdef BOOST_HIGH_RESOL_TIMER_ENABLE
		#include <boost/chrono/system_clocks.hpp>
		#include <boost/chrono.hpp>
		typedef boost::chrono::high_resolution_clock boost_timer;
	#else
		#define BOOST_CPUTIMER_ENABLE
		#include <boost/timer/timer.hpp>
		typedef boost::timer::cpu_timer boost_timer;
	#endif

#else
	#ifdef __APPLE__
		#include <mach/mach_time.h>
		#include <limits>
		namespace boost
		{
			#define NANOSEC 1000000000
			class timer
			{
			public:
				timer()
				{
					mach_timebase_info(&_base);
					_start_time = mach_absolute_time();
				}
				void restart() { _start_time = mach_absolute_time(); }
				double elapsed() const
				{
					uint64_t e = mach_absolute_time() - _start_time;
					int unit =   _base.numer / _base.denom;
					return  double(e * unit)/NANOSEC; //seconds
				}
				double elapsed_max() const
				{
					return (double((std::numeric_limits<std::clock_t>::max)())
					- double(_start_time)) / (double(_base.denom/_base.numer)/NANOSEC);
				}

				double elapsed_min() const
				{ return double(1)/double(double(_base.denom/_base.numer)/NANOSEC); }

			private:
				  uint64_t _start_time;
				  mach_timebase_info_data_t _base;
			}; // timer

		}
	#else
		#include <boost/timer.hpp>
	#endif
		typedef boost::timer boost_timer;

#endif

namespace bzs
{
namespace rtl
{
// single thread only
class benchmark
{
	static boost_timer t;
	#ifdef BOOST_HIGH_RESOL_TIMER_ENABLE
	static boost_timer::time_point  m_start;
	#endif
public:
	static bool report(boost::function<bool()> func, const char* name);
	static void report2(boost::function<void()> func, const char* name);
	static void start();
	static int  stop();
	static void showTimes(int result, const char* name);
	static void showTimeSec(bool result, const char* name);

};


//Multi thread version. Self measurement.
class benchmarkMt
{
	boost_timer t;
	#ifdef BOOST_HIGH_RESOL_TIMER_ENABLE
	boost_timer::time_point  m_start;
	#endif

public:

	void start();
	int end();
};


}//namespace rtl
}//namespace bzs
#endif
