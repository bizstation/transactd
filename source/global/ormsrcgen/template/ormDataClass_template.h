#ifndef % includegurde % H
#define % includegurde % H

#include <bzs/env/tstring.h>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <bzs/db/protocol/tdap/btrDate.h>
namespace td = bzs::db::protocol::tdap;

% nameSpaceBegin %

    class % extern % % className %
{
  protected:
    struct imple* m_impl;

  public:
    % className % (void* owner);
    % className % (const % className % &rt);
    ~ % className % ();
    % className % &operator=(const % className % &rt);
    % dataClassMembaFuncDec % static % className % *create(void* owner);
};

typedef boost::shared_ptr< % className % > % className % _ptr;
typedef std::vector< % className % _ptr> % className % _ptr_list;
typedef std::vector< % className % *> % className % _list;
typedef boost::shared_ptr< % className % _list> % className % _list_ptr;

% className % *create(% className % _ptr_list & mdls, int);

% nameSpaceEnd %

#endif //%includegurde%H