Transactd
===============================================================================
Transactdは、MySQL/MariaDB内部のデータベースアクセスメソッドを直接操作すること
で、柔軟で高速なデータベース処理を提供します。

一般的にNoSQLにはトランザクションが使えないなどの制限がありますが、
TransactdはSQLと同じようにトランザクション処理を行うことができます。

SQLでの操作は、サーバー内でSQL構文を解析して行単位でのアクセスメソッドに分解し、
処理を行って結果を返します。

Transactdは、サーバー内部で行われる行単位の処理をクライアント側でコントロール
できるようにしたものです。APIはとてもシンプルです。

例えば、`users`テーブルのすべてのレコードを読み取り`id`と`name`を表示するコードは
以下のようになります。

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

この他にも便利なAPIが用意されています。[チュートリアル](
http://www.bizstation.jp/ja/transactd/documents/tutorial.html)で主な機能を概観
することができます。



インストール
-------------------------------------------------------------------------------
Transactdはプラグインとして動作するサーバー側モジュールと、クライアント側で動作
するクライアント側モジュールから構成されます。
サーバー側をTransactd Plugin、クライアント側をTransactdクライアントと呼びます。

ソースコードからビルドすることも、ビルド済みバイナリを使用することもできます。
ビルド・ダウンロードおよびインストールの方法は、[Transactd インストールガイド](
http://www.bizstation.jp/ja/transactd/documents/install_guide.html)を参照して
ください。



ドキュメント等
-------------------------------------------------------------------------------
* [Transactd リリースノート](
  https://github.com/bizstation/transactd/blob/master/RELEASE_NOTE-JA.md)
* [Transactd プロダクトページ](http://www.bizstation.jp/ja/transactd/)
* [Transactd ドキュメント](http://www.bizstation.jp/ja/transactd/documents/)
* [Transactd Utilities ガイド](
  http://www.bizstation.jp/ja/transactd/documents/utilities_guide.html)
* [チュートリアル](http://www.bizstation.jp/ja/transactd/documents/tutorial.html)
* [リファレンス](http://www.bizstation.jp/ja/transactd/client/sdk/doc/)
* リポジトリ内の`source/bzs/example`に簡単なサンプルコードがあります。



ライセンス
-------------------------------------------------------------------------------
MySQL/MariaDBのGPLv2ライセンスにより、Transactd PluginはGeneral Public License
 (GPLv2)のもとで利用可能です。ライセンス情報はCOPYINGをご覧ください。

TransactdクライアントのライセンスはGeneral Public License (GPLv2)と商用ライセ
ンスのいずれかから選ぶことができます。

GPLv2以外のオープンソースプロジェクト向けの[TransactdクライアントOSS例外規定](
http://www.bizstation.jp/ja/transactd/support/ossex.html)も参照してください。

* [Transactd ライセンス](http://www.bizstation.jp/ja/transactd/support/license.html)
* [Transactd ライセンスFAQ](
  http://www.bizstation.jp/ja/transactd/support/license_faq.html)



バグ報告・要望・質問など
-------------------------------------------------------------------------------
バグ報告・要望・質問などは、[github上のIssueトラッカー](
https://github.com/bizstation/transactd/issues)にお寄せください。
