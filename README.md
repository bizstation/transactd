Transactd
===============================================================================
Transactd manipulates internal database access method to MySQL/MariaDB directly, 
and provides the flexible high-speed database access.

In general, NoSQL has some limitations such as can not use the transaction.
But Transactd will can use transaction in the same way as SQL.

Operation with SQL is that: parse SQL syntax in the server, decompose to row 
basis access methods, processing, and returns the result.

Transactd make it possible to control these row basis processing in the client 
side. And the API is very simple.

For example, following code shows how to print `id` and `name` of all records in 
`users` table.

```ruby
db = Transactd::Database.new()
db.open('tdap://servername/dbname?dbfile=transactd_schema')

tb = db.openTable('users', Transactd::TD_OPEN_NORMAL)

tb.seekFirst()
while (tb.stat() == 0) do
  puts('id = ' + tb.getFVint('id').to_s() + ', name = ' + tb.getFVstr('name'))
  tb.seekNext()
end

tb.close()
db.close()
```

There are more APIs. See [tutorial](
http://www.bizstation.jp/en/transactd/documents/tutorial.html) to overview 
main features of Transactd.



Install
-------------------------------------------------------------------------------
Transactd consists of two modules, server plugin and client libraries.
Server plugin is called "Transactd plugin", client libraries are called 
"Transactd clients".

You can build Transactd from source code. Or you can use pre-built binaries.
See [Transactd Install guide](
http://www.bizstation.jp/en/transactd/documents/install_guide.html) to building 
or installing Transactd.



Documents
-------------------------------------------------------------------------------
* [Transactd Release Note](
  https://github.com/bizstation/transactd/blob/master/RELEASE_NOTE.md)
* [Transactd product page](http://www.bizstation.jp/en/transactd/)
* [Transactd documents](http://www.bizstation.jp/en/transactd/documents/)
* [Transactd Utilities guide](
  http://www.bizstation.jp/en/transactd/documents/utilities_guide.html)
* [Tutorial](http://www.bizstation.jp/en/transactd/documents/tutorial.html)
* [Reference](http://www.bizstation.jp/ja/transactd/client/sdk/doc/) (Japanese)
* Some sample codes are in `source/bzs/example`.



License
-------------------------------------------------------------------------------
According to the license of MySQL/MariaDB, Transactd Plugin can be used under
General Public License (GPLv2). The license information is in COPYING.

You can select the license of Transactd clients from General Public License
(GPLv2) or commercial license.

See also [Transactd clients OSS Exception](
http://www.bizstation.jp/en/transactd/support/ossex.html) for OSS projects which
is not subject to GPLv2.

* [Transactd license](http://www.bizstation.jp/en/transactd/support/license.html)
* [Transactd license FAQ](
  http://www.bizstation.jp/en/transactd/support/license_faq.html)



Bug reporting, requests and questions
-------------------------------------------------------------------------------
If you have any bug-reporting, requests or questions, please send it to
[Issues tracker on github](https://github.com/bizstation/transactd/issues).
