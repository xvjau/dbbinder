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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <map>

#include <google/template.h>

namespace DBBinder
{

typedef std::string String;
typedef std::list<String> ListString;

// Options set by command line
extern String		appName;
extern const char*	optFileName;
extern String		optOutput;
extern const char*	optTemplate;
extern ListString	optTemplateDirs;
extern bool		optXML;
extern bool		optYAML;

// Utility funcs and defines
inline String stringToLower(const String& _string)
{
	String result;

	for(uint i = 0; i < _string.length(); ++i)
		result += tolower(_string[i]);

	return result;
}

inline String stringToUpper(const String& _string)
{
	String result;

	for(uint i = 0; i < _string.length(); ++i)
		result += toupper(_string[i]);

	return result;
}

inline String extractFileName(const String& _string)
{
	size_t pos = _string.find_last_of( '/' );
	if ( pos != String::npos )
		return _string.substr( pos + 1 );
	else
		return _string;
}
String cescape(const String& _string);

inline ListString stringTok(const String &_string, const char _sep)
{
	ListString result;

	const char *p = _string.c_str();
	const char *s = p;

	while ( *++p )
	{
		if ( *p == _sep )
		{
			result.push_back( String( s, p - s ));
			++p;
			s = p;
		}
	}

	if ( *s )
		result.push_back( String( s, p - s ));

	return result;
}

}

#define FATAL(MSG) { std::cerr << DBBinder::appName << ": " << MSG << std::endl; exit( 1 ); }
#define WARNING(MSG) { std::cerr << DBBinder::appName << ": " << MSG << std::endl; }

#define GET_TEXT_OR_ATTR( str, elem, attr ) { str = elem->GetText(false); if ( str.empty() ) elem->GetAttribute(attr, &str, false); }

#include <boost/function.hpp>
#include <boost/foreach.hpp>
#define foreach(X, Y) BOOST_FOREACH(X, Y)

#endif // __DBBINDER_MAIN_H
