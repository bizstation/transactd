/*=================================================================
   Copyright (C) 2013 BizStation Corp All rights reserved.

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
var DRY_RUN = false;
var LOG_OFF = false;
var IGNORE_ERROR = false;
var VERSION_SEPARATOR = '.';
var TIMEOUT = 30 * 1000; // copy timeout = 30 sec
var fso = new ActiveXObject('Scripting.FileSystemObject');

// replace '/' to '\'
function toNativePath(path)
{
    return path.replace(/\//g, '\\').replace(/^\s*(.*?)\s*$/, "$1");
}

// replace '/' to '\' and convert to absolute path
function toNativeAbsolutePath(path)
{
    return fso.GetAbsolutePathName(toNativePath(path));
}

// get parent folder native absolute path
function getParentPath(path)
{
    path = toNativeAbsolutePath(path);
    var name   = fso.GetFileName(path);
    return stripPath(path.substring(0, path.length - name.length));
}

// delete end of '\'
function stripPath(path)
{
    return path.replace(/\\$/, '');
}

// create folder recursive -- return true if succeed
function createFolder(path)
{
    var fullpath = toNativeAbsolutePath(path);
    if (fso.FolderExists(fullpath))
        return true;
    var myName = fso.GetFileName(fullpath);
    var parentPath = fullpath.substring(0, fullpath.length - myName.length);
    parentPath = toNativeAbsolutePath(parentPath);
    if (fullpath == parentPath)
        return false;
    var parentOK = fso.FolderExists(parentPath) || createFolder(parentPath);
    if (!parentOK)
        return false;
    try
    {
        fso.CreateFolder(fullpath);
        return true;
    } catch (e) {}
    return false;
}

// get version array
function getVersion(path)
{
    var str = fso.FileExists(path) ? fso.GetFileVersion(path) : '';
    var ver = str.split(VERSION_SEPARATOR);
    var maxstrlen = 0;
    for(var i = 0; i < ver.length; i++)
    {
        ver[i] = ver[i].replace(/^\s*(.*?)\s*$/, "$1");
        if (ver[i].length > maxstrlen)
            maxstrlen = ver[i].length;
    }
    if (maxstrlen == 0)
        ver = [];
    return ver;
}

// hasVersion -- return true if both of srcpath-version and destpath-version has version info
function haveVersion(srcpath, destpath)
{
    var srcver  = getVersion(srcpath);
    var destver = getVersion(destpath);
    return ((srcver.length > 0) && (destver.length > 0));
}

// greater -- return true if srcpath-version is greater than destpath-version
function greater(srcpath, destpath)
{
    var srcver  = getVersion(srcpath);
    var destver = getVersion(destpath);
    if((srcver.length <= 0) || (destver.length <= 0))
        return false;
    var i = 0;
    while(i < srcver.length && i < destver.length)
    {
        src = srcver[i];
        dest = destver[i];
        if (!isNaN(src) && !isNaN(dest))
        {
            src = parseInt(src, 10);
            dest = parseInt(dest, 10);
        }
        if(src > dest)
            return true;
        i++;
    }
    return false;
}

// copy -- copy srcpath to destpath (overwrite)
//         return true if succeed
function copy(srcpath, destpath)
{
    if (!fso.FileExists(srcpath))
        return false;
    if (!createFolder(getParentPath(destpath)))
        return false;
    try
    {
        if (! DRY_RUN)
        {
            fso.CopyFile(srcpath, destpath, true);
            var timer = 0;
            while(!fso.fileExists(destpath) && (timer < TIMEOUT))
            {
                WScript.Sleep(100);
                timer += 100;
            }
            return (timer < TIMEOUT);
        }
    }
    catch (e)
    {
        return false;
    }
    return true;
}

/*
  copyIfGreater
    copy srcpath to destpath if srcpath is greater than destpath
    return code:
      COPYRESULT_COPIED     (0): file copied
      COPYRESULT_NOTCOPIED  (1): file not copied because it is not greater than dest
      COPYRESULT_CANNOTREAD (2): file not copied because can not read version number
      COPYRESULT_EXITERROR  (3): file not copied because some error occured
*/
var COPYRESULT_COPIED     = 0;
var COPYRESULT_NOTCOPIED  = 1;
var COPYRESULT_CANNOTREAD = 2;
var COPYRESULT_EXITERROR  = 3;
function copyIfGreater(srcpath, destpath)
{
    // 1. check srcpath exists (not folder!)
    if (!fso.FileExists(srcpath))
        return COPYRESULT_EXITERROR;
    // 2. check destpath (not folder!) and its parent
    if (fso.FolderExists(destpath))
        return COPYRESULT_EXITERROR;
    if (!createFolder(getParentPath(destpath)))
        return COPYRESULT_EXITERROR;
    // 3. if destpath not exists then copy file
    if (!fso.FileExists(destpath))
    {
        if (copy(srcpath, destpath))
            return COPYRESULT_COPIED;
        return COPYRESULT_EXITERROR;
    }
    // 4. check both of srcpath and destpath have version info
    if (!haveVersion(srcpath, destpath))
        return COPYRESULT_CANNOTREAD;
    // 5. if this is greater than destFileInfo then copy file
    if (greater(srcpath, destpath))
    {
        if (copy(srcpath, destpath))
            return COPYRESULT_COPIED;
        return COPYRESULT_EXITERROR;
    }
    // 6. other do nothing
    return COPYRESULT_NOTCOPIED;
}

// parse arguments to file list
function printArg(args){
  WScript.Echo('source:' + args['source'] + "\ndest:" +args['dest'] + "\n");
}
function parseArgs(args)
{
    if (!(args instanceof Array) || (args.length < 2))
        return false;
    // 1. last arg is dest
    var dest = toNativePath(args[args.length - 1])
    // 2. is dest folder? -- end with '\\' or multi source then dest is folder
    var destIsFolder = (/\\$/.test(dest) || args.length > 2 || fso.FolderExists(dest))
      //WScript.Echo(destIsFolder ? 'destIsFolder' : 'destIs not Folder')
    // 3. if dest is file then only 1 file to copy
    if (!destIsFolder)
    {
        var src = stripPath(toNativePath(args[0]));
        if (src.length <= 0)
            return { 'source': new Array(), 'dest': new Array() };
        var dst = stripPath(toNativePath(args[1]));
        return { 'source': new Array(src), 'dest': new Array(dst) };
    }
    // 4. if dest is folder, make dest array
    var ret = { source: new Array(), dest: new Array() };
    var tmp_srcs = args.slice(0, args.length - 1);
    for(var i = 0; i < tmp_srcs.length; i++)
    {
        var path = toNativePath(tmp_srcs[i]);
        if (path.length > 0)
        {
            var name = fso.GetFileName(path);
            ret['source'].push(path);
            ret['dest'].push(fso.BuildPath(dest, name));
        }
    }
    return ret;
}


function main()
{
    var args = [];
    for(var i = 0; i < WScript.Arguments.length; i++)
    {
        var argument = WScript.Arguments(i).replace(/^\s*"?\s*(.*?)\s*"?\s*$/, "$1");
        var subargs = argument.split(';');
        for(var j = 0; j < subargs.length; j++)
        {
            var arg = subargs[j].replace(/^\s*(.*?)\s*$/, "$1");
            switch (arg) {
                case '--dry-run': DRY_RUN = true; break;
                case '--log-off': LOG_OFF = true; break;
                case '--ignore-error': IGNORE_ERROR = true; break;
                default:
                  if (arg.length > 0);
                      args.push(arg);
                  break;
              }
        }
    }
    var files = parseArgs(args);
    if (!files)
        WScript.Quit(1);
    var sources = files['source'];
    var dests   = files['dest'];
    var exitcode = 0;
    for(var i = 0; i < sources.length; i++)
    {
        var ret = copyIfGreater(sources[i], dests[i]);
        switch (ret) {
            case COPYRESULT_COPIED:
                if (!LOG_OFF)
                    WScript.Echo('Installing-if-grater: ' + dests[i]);
                break;
            case COPYRESULT_NOTCOPIED:
                if (!LOG_OFF)
                    WScript.Echo('Up-to-date-if-grater: ' + dests[i]);
                break;
            case COPYRESULT_CANNOTREAD:
                if (!LOG_OFF)
                    WScript.StdErr.WriteLine('Install-warning: ' + sources[i] + ' -> ' + dests[i]);
                break;
            case COPYRESULT_EXITERROR:
            default:
                if (!LOG_OFF)
                    WScript.StdErr.WriteLine('Install-error-' + ret + ': ' + sources[i] + ' -> ' + dests[i]);
                if (!IGNORE_ERROR)
                    WScript.Quit(1);
                exitcode = 1;
                break;
        }
    }
    WScript.Quit(exitcode);
}
main()
