#include <bzs/db/protocol/tdap/client/trdboostapi.h>
#include <iostream>
#include <vector>

using namespace bzs::db::protocol::tdap::client;
using namespace bzs::db::protocol::tdap;

/**
@brief Read records and manual O/R mapping example

This program read records of the "user" table where group = 3.
And O/R mapping to the user class.
This program use the filter operation on server side.

Please execute "create database" , "change schema" and "insert records" example
	before execute this example.

*/

static const short fieldnum_id = 0;
static const short fieldnum_name = 1;
static const short fieldnum_group = 2;
static const short fieldnum_tel = 3;
static const char_td keynum_group = 1;

/** User class */
class user
{
public:
	int id;
	std::string name;
	int group;
	std::string tel;

};

typedef boost::shared_ptr<user> user_ptr;

/** dump user to screen */
void dumpUser(const user_ptr& user)
{
	std::cout << " id           " << user->id    << std::endl;
	std::cout << " name         " << user->name  << std::endl;
	std::cout << " group        " << user->group << std::endl;
	std::cout << " tel          " << user->tel   << std::endl << std::endl;

}

/** OR mapping functional object*/
class userMappper
{
	std::vector<user_ptr>& m_users;
public:
	userMappper(std::vector<user_ptr>& users):m_users(users){}
	void operator()(const fields& fds)
	{
	   user_ptr u(new user());
	   u->id      = fds[fieldnum_id].i();
	   u->name    = fds[fieldnum_name].a_str(); //get by ansi string
	   u->group   = fds[fieldnum_group].i();
	   u->tel     = fds[fieldnum_tel].a_str();  //get by ansi string
	   m_users.push_back(u);
	}
};

void readUsers(table_ptr tb, std::vector<user_ptr>& users)
{

	// Get filterIterator. This filter is execute on server side.
	filterParams fp( _T("group = 3"), 1 , 0);
	findIterator itsf = find(tb, keynum_group, fp, 3);

	// O/R mapping functional object
	userMappper ormap(users);

	//loop each group=1 records
	for_each(itsf, ormap);

}

#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
	database_ptr db = createDatadaseObject();
	try
	{
		connectParams param(_T("tdap"), _T("localhost"), _T("test"), _T("test"));
		openDatabase(db, param);

		table_ptr tb = openTable(db, _T("user"));
		std::vector<user_ptr> users;
		readUsers(tb, users);

		std::cout << "Read records success.\nRecord count = " << users.size() << std::endl;

		//dump users to screen.
		std::for_each(users.begin(), users.end(), dumpUser);

		return 0;
	}

	catch(bzs::rtl::exception& e)
	{
		std::tcout << _T("[ERROR] ") << *bzs::rtl::getMsg(e) << std::endl;
	}
	return 1;
}
