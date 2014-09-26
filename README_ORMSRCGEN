ormsrcgen C ++ O/R mapping source code generator
===============================================================================
ormsrcgen (32|64) is a source code generator for C++ O/R mapping.

C++ does not allow to define classes dynamically. So we need to generate model
classes with the source code generator. You can use O/R mapping fast and easy
in C++ with it.



Introduction
-------------------------------------------------------------------------------
Related files of the generator is in `source/global/ormsrcgen/template`.
The descriptions of each files are:

  * ormDataClass_template.cpp : Template of C ++ model class.
  * ormDataClass_template.h   : Template of C ++ model class.
  * ormMapClass_template.cpp  : Template of C ++ map class.
  * ormMapClass_template.h    : Template of C ++ map class.
  * template.cnf              : Template of the configuration file
  * fieldNameList_sample.txt  : Sample file of field name alias.

Edit `fieldNameList_sample.txt` and `template.cnf` as needed.



Command-line options
-------------------------------------------------------------------------------
Command line options are:

```
command line option:
  -d [ --database_uri ] database uri ex:tdap://hostname/dbname?dbfile=trnasctd_schema
  -t [ --table_name ] table name.
  -c [ --class_name ] class name.
  -f [ --conf_name ] configuration file.

-d :Specify the database URI.
-t :Specify the table name to generate the model class.
    The class is generated for each table.
-c :Specify the name of the model class.
-f :Specify the path of configuration file.
```

In many cases, you can use same configuration file even if tables are different.
Template of configuration file is `source/global/ormsrcgen/template/template.cnf`.
You can copy and edit it.



Descriptions of the configuration file
-------------------------------------------------------------------------------
* `lang = (language)`
  
  Specify the language of the code to be generated. Only `C++` is supported in
  current version.

* `files = (number of file)`
  
  Specify the number of template files required for a model. In the case of 
  C++, the number is `2`.

* `file1 = (file path) file2 = (file path) ... fileN = (file path)`
  
  Specify the path of the template file, with the number that was specified in
  `files=`.
  Default values are `file1 = ormDataClass_template` `file2 = ormMapClass_template`,
  but please specify the absolute path in actually.

* `saveDir = (output dir)`
  
  Specify the output directory to save generated source code files.

* `setPrefix = (setter prefix)`
  
  Specify the prefix of the setter method of member variables in the class.
  If `setPrefix = set`is specified, the setter function for `name` field is:
  
  ```
  setName(const char* v)
  ```

* `getPrefix = (getter prefix)`
  
  Specify the prefix of the getter method as well as setPrefix.

* `externWord = (extern keyword)`
  
  Add export keyword in class declaration when make shared libraries
  such as `.SO` or `.DLL`.
  
  ```
  class $externWord someModel
  {
  ...
  };
  ```

* `fieldRenameList = (alias list file)`
  
  Normally, the member variable name of the class is same as the name of
  field in the table. If you want to change the name, specify fieldRenameList
  which contains alias list.
  The fieldRenameList format is:
  
  ```
  originalName1=alias1
  originalName2=alias2
  ```

* `name_space = (namespace of model class)`
  
  Specify the namespace of the model class.

* `name_space_map = (namespace of map class)`
  
  Specify the namespace of the map class.
  The map class maps the members of the model class and the fields in the table.
