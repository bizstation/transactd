//---------------------------------------------------------------------------

#ifndef BZS_EXSAMPLE_QUERYDATA_H
#define BZS_EXSAMPLE_QUERYDATA_H
//---------------------------------------------------------------------------
#include <bzs/db/protocol/tdap/client/trdboostapi.h>


int prebuiltData( bzs::db::protocol::tdap::client::database_ptr db
		, const bzs::db::protocol::tdap::client::connectParams& param
		, bool foceCreate=false, int maxId=20000);

#endif //BZS_EXSAMPLE_QUERYDATA_H
