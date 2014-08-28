#ifndef %includegurde%_MAPH
#define %includegurde%_MAPH

#include "%className%.h"
#include <bzs/db/protocol/tdap/client/trdboostapi.h>

%nameSpaceMapBegin%

class %extern% %className%_fdi
{
public:
%fdiMemba%

	void init(td::client::table* tb);
};

%className%_fdi* createFdi(%className%_fdi *);
void destroyFdi(%className%_fdi * p);
void initFdi(%className%_fdi * fdi, td::client::table* tb);

class %extern%%className%_orm
{
	const %className%_fdi& m_fdi;
	
	//noncopyable
	%className%_orm(const %className%_orm&);
	%className%_orm& operator=(const %className%_orm&);
public:
	enum key
	{
	%keyEnum%	
	};
	%className%_orm(const %className%_fdi& fdi):m_fdi(fdi){}
	bool compKeyValue(%className%& l, %className%& r, int keyNum) const;
	void setKeyValues(%className%& m, const td::client::fields& fds, int keyNum);
	void writeMap(%className%& m, const td::client::fields& fds, int optipn);
	void readMap(%className%& m, const td::client::fields& fds, int optipn);
	void readAuntoincValue(%className%& m, const td::client::fields& fds, int optipn);
	const _TCHAR* getTableName(){return _T("%tableName%");}
	
	/*  must define mdl_typename */
	typedef %className% mdl_typename; 

	/*  must define  fdi_typename
		If fdi is not used, It is considered as default "typedef fdibase fdi_typename" 
		If using different FDI(s),In this case, the class of fdi is specified like 
		 "activeTable<group, group_fdi>" at the instance creation of activeTable. */
	typedef %className%_fdi fdi_typename;

	/* if use not vector collection, please spcecify your_collection_type*/
	//typedef mdlsHandler<%className%_orm, your_collection_type>   collection_orm_typename;
};

%nameSpaceMapEnd%

#endif //%includegurde%_ORMH