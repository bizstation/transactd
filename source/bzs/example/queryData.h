//---------------------------------------------------------------------------

#ifndef BZS_EXSAMPLE_QUERYDATA_H
#define BZS_EXSAMPLE_QUERYDATA_H
//---------------------------------------------------------------------------
#include <bzs/db/protocol/tdap/client/trdboostapi.h>

int prebuiltData(bzs::db::protocol::tdap::client::database_ptr db,
                 const bzs::db::protocol::tdap::client::connectParams& param,
                 bool foceCreate = false, int maxId = 20000);

bool createCacheTable(bzs::db::protocol::tdap::client::dbdef* def);
void fillBlobField(short fieldNum, int id, bzs::db::protocol::tdap::client::table* tb,
                    unsigned char* buf);
bool compBlobField(int id, bzs::db::protocol::tdap::client::field& fd);
const _TCHAR* name_field_str(_TCHAR* buf);

#endif // BZS_EXSAMPLE_QUERYDATA_H
