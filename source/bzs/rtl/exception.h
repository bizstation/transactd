#ifndef BZS_RTL_EXCEPTION_H
#define BZS_RTL_EXCEPTION_H
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

#include <exception>
#include <string>
#include <bzs/env/compiler.h>
#if ((defined(__x86_32__) || defined(__APPLE__)) && defined(__BORLANDC__))
#include <bzs/env/tstring.h>
#else
#include <boost/exception/info.hpp>
#include <boost/exception/all.hpp>

#endif

namespace bzs
{
namespace rtl
{

/** C++Builder XE 32bit  can not use boost exception*/
#if ((defined(__x86_32__) || defined(__APPLE__)) && defined(__BORLANDC__))

class exception : public std::exception
{
    int m_error;
    std::_tstring m_message;

public:
    exception() : std::exception(), m_error(0){};
    exception(int error, const std::_tstring& message)
        : std::exception(), m_error(error), m_message(message){};
    explicit exception(int error) : std::exception(), m_error(error){};
    explicit exception(const std::_tstring& message)
        : std::exception(), m_message(message){};
    const std::_tstring& getMessage() { return m_message; }
    const int& getErrorCode() { return m_error; }
    exception& operator<<(int error)
    {
        m_error = error;
        return *this;
    }
    exception& operator<<(const _TCHAR* msg)
    {
        m_message += msg;
        return *this;
    }
};

#define THROW_BZS_ERROR_WITH_CODE(code) throw bzs::rtl::exception(code)
#define THROW_BZS_ERROR_WITH_MSG(msg) throw bzs::rtl::exception(msg)
#define THROW_BZS_ERROR_WITH_CODEMSG(code, msg)                                \
    throw bzs::rtl::exception(code, msg)

inline const std::_tstring* getMsg(bzs::rtl::exception& e)
{
    return &(e.getMessage());
}
inline const int* getCode(bzs::rtl::exception& e)
{
    return &e.getErrorCode();
}

inline int errnoCode(int code)
{
    return code;
}
inline const _TCHAR* errMessage(const _TCHAR* msg)
{
    return msg;
}

#else // not c++builder 32bit

struct exception_base : virtual std::exception, virtual boost::exception
{
};
struct exception : virtual bzs::rtl::exception_base
{
};

#ifdef _UNICODE
typedef std::wstring message_type;
#else
typedef std::string message_type;
#endif

typedef boost::error_info<struct tagCode, int> errnoCode;
typedef boost::error_info<struct tagMsg, message_type> errMessage;

#define THROW_BZS_ERROR_WITH_CODE(code)                                        \
    throw bzs::rtl::exception() << bzs::rtl::errnoCode(code)
#define THROW_BZS_ERROR_WITH_MSG(msg)                                          \
    throw bzs::rtl::exception() << bzs::rtl::errMessage(msg)
#define THROW_BZS_ERROR_WITH_CODEMSG(code, msg)                                \
    throw bzs::rtl::exception() << bzs::rtl::errnoCode(code)                   \
                                << bzs::rtl::errMessage(msg)

inline const message_type* getMsg(bzs::rtl::exception& e)
{
    return boost::get_error_info<bzs::rtl::errMessage>(e);
}
inline const int* getCode(bzs::rtl::exception& e)
{
    return boost::get_error_info<bzs::rtl::errnoCode>(e);
}
#endif

} // namespace rtl
} // namespace bzs

#endif // BZS_RTL_EXCEPTION_H
