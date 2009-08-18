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

#include "main.h"
#include "sql_reader.h"
#include "xml_reader.h"
#include "yaml_reader.h"

namespace DBBinder
{

static const char* fileName = 0;

void parseSQL(const char* _fileName, AbstractGenerator **_generator)
{
	fileName = _fileName;

	std::ifstream file(_fileName);
	if ( file.good() )
	{
		char buffer[4096];

		int line = 0;
		while ( file.good() )
		{
			file.getline(buffer, 4096);
			line++;

			if ( strncmp(buffer, "--! use", 7 ) == 0 && isblank(buffer[7]) )
			{
				char *start = &buffer[8];
				char *end = start + strlen(start);
				char *c, *ext;

				if ( end <= start)
					FATAL(fileName << ':' << line << ": missing file name");

				c = end - 1;

				while( isblank(*c) )
					c--;

				if ( c <= start )
					FATAL(fileName << ':' << line << ": missing file name");

				if ( isblank(*(c + 1)) )
					*(c + 1) = '\0';

				end = c;
				c = start;

				while( isblank(*c) )
					c++;

				start = c;

				ext = end;
				while( ext > start && *(ext - 1) != '.')
					ext--;

				if ( strcasecmp(ext, "yaml") == 0 )
				{
					parseYAML(start, _generator);
					break;
				}
				else if ( strcasecmp(ext, "xml") == 0 )
				{
					parseXML(start, _generator);
					break;
				}
				else
				{
					FATAL(fileName << ':' << line << ": unknown file extension: " << ext);
				}
			}
		}
	}
	else
	{
		FATAL(fileName << ": unable to open file.");
	}

	if ( !*_generator )
		FATAL(fileName << ": unable to determine database type.");


}

}
