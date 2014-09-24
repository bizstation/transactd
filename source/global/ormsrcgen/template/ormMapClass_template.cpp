
#pragma hdrstop
#include "%className%_map.h"
#include <bzs/env/tstring.h>
using namespace std;
using namespace bzs::db::protocol::tdap::client;

% nameSpaceMapBegin %

    /*  class %className%_fdi */
    void % className % _fdi::init(table* tb){ % fdiResolver % }

    % className % _fdi* createFdi(% className % _fdi*)
{
    return new % className % _fdi();
}
void destroyFdi(% className % _fdi * p) { delete p; }
void initFdi(% className % _fdi * fdi, td::client::table * tb)
{
    fdi->init(tb);
}

/*  class %className%_orm */
bool % className %
    _orm::compKeyValue(% className % &l, % className % &r, int keyNum) const
{
    % keyComp %
}

void % className %
    _orm::setKeyValues(% className % &m, const fields& fds, int keyNum)
{
    % writeKeyValue %
}

void % className %
    _orm::writeMap(% className % &m, const fields& fds, int optipn)
{
    % write %
}

void % className %
    _orm::readMap(% className % &m, const fields& fds, int optipn)
{
    % read %
}

void % className % _orm::readAuntoincValue(% className % &m, const fields& fds,
                                           int optipn){ % autoinc % }

% nameSpaceMapEnd %
