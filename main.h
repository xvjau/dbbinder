/*
    Copyright 2008 Gianni Rossi

    This file is part of DBBinder++.

    DBBinder++ is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    DBBinder++ is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with DBBinder++.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __DBBINDER_MAIN_H
#define __DBBINDER_MAIN_H

#include <template.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

namespace DBBinder
{

/*
    Since Google decided to change the namespace between releases,
    these typedefs will put everything we need in the DBBinder namespace
*/
#ifdef CTEMPLATE_TEMPLATE_H_
typedef ctemplate::TemplateDictionary TemplateDictionary;
typedef ctemplate::Template Template;
#define DO_NOT_STRIP ctemplate::DO_NOT_STRIP
#else
typedef google::TemplateDictionary TemplateDictionary;
typedef google::Template Template;
#define DO_NOT_STRIP google::DO_NOT_STRIP
#endif

typedef std::vector<std::string> ListString;

enum FileType
{
    ftNULL,
    ftYAML,
    ftXML,
    ftSQL,
};

// Options set by command line
extern std::string  appName;
extern std::string  optOutput;
extern std::string  optTemplate;
extern ListString   optTemplateDirs;
extern const char*  optVersionMajor;
extern const char*  optVersionMinor;
extern bool         optListDepends;
extern ListString   optDepends;
extern bool         optExtras;
extern ListString   optIncludeFiles;

// Utility funcs and defines
inline std::string stringToLower(const std::string& _string)
{
    std::string::size_type sz = _string.length();

    std::string result;
    result.resize( sz );

    for(std::string::size_type i = 0; i < sz; ++i)
        result[i] += tolower(_string[i]);

    return result;
}

inline std::string stringToUpper(const std::string& _string)
{
    std::string::size_type sz = _string.length();

    std::string result;
    result.resize( sz );

    for(std::string::size_type i = 0; i < sz; ++i)
        result[i] += toupper(_string[i]);

    return result;
}

inline std::string extractFileName(const std::string& _string)
{
    std::string::size_type pos = _string.rfind( '/' );
    if ( pos != std::string::npos )
        return _string.substr( pos + 1 );
    else
        return _string;
}
std::string cescape(const std::string& _string);

inline ListString stringTok(const std::string &_string, const char _sep)
{
    ListString result;

    const char *p = _string.c_str();
    const char *s = p;

    while ( *++p )
    {
        if ( *p == _sep )
        {
            result.push_back( std::string( s, p - s ));
            ++p;
            s = p;
        }
    }

    if ( *s )
        result.push_back( std::string( s, p - s ));

    return result;
}


inline ListString stringTok(const std::string &_string)
{
    ListString result;

    const char *p = _string.c_str();
    const char *s;

    while ( *p )
    {
        if ( !isspace( *p ))
        {
            s = p;
            while( *s && !isspace( *s ))
                ++s;

            result.push_back( std::string( p, s - p ));

            p = s;
        }
        else
            p++;
    }

    return result;
}

inline std::string getFilenameRelativeTo(const std::string& _relFileName, const std::string& _fileName)
{
    std::string result;

    std::string::size_type pos = _relFileName.rfind('/');
    if ( pos && pos != std::string::npos )
    {
        result = _relFileName.substr(0, pos);
        result += '/';
    }

    result += _fileName;

    return result;
}

inline bool fileExists(const std::string& fileName)
{
    struct stat fs;
    return stat(fileName.c_str(), &fs) == 0;
}

enum FileCheckType
{
    fctRegularFile = S_IFREG,
    fctDirectory = S_IFDIR
};

enum FileCheck
{
    fcOK,
    fcDoesNotExist,
    fcIsNotExpectedType
};

inline FileCheck checkFileExistsAndType(const std::string& fileName, FileCheckType type)
{
    struct stat fs;
    if ( stat(fileName.c_str(), &fs) )
        return fcDoesNotExist;
    else
    {
        if (( fs.st_mode & S_IFMT ) != type )
            return fcIsNotExpectedType;
    }
    return fcOK;
}

}

#ifdef NDEBUG
#define FATAL(MSG) { std::cerr << MSG << std::endl; abort(); }
#define ASSERT(COND, MSG)
#else
#define FATAL(MSG) { std::cerr << MSG << std::endl; assert(false); }
#define ASSERT(COND, MSG) { if ( !COND ) std::cerr << DBBinder::appName << ": " << MSG << std::endl; assert(COND); }
#endif

#define WARNING(MSG) { std::cerr << DBBinder::appName << ": " << MSG << std::endl; }

#define GET_TEXT_OR_ATTR( str, elem, attr ) { str = elem->GetText(false); if ( str.empty() ) elem->GetAttribute(attr, &str, false); }

#include <boost/function.hpp>
#include <boost/foreach.hpp>
#define foreach(X, Y) BOOST_FOREACH(X, Y)

#define UNUSED(VAR) (void)(VAR);

#endif // __DBBINDER_MAIN_H
