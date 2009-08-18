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

#include <libgen.h>

#include "main.h"
#include "abstractgenerator.h"
#include "sql_reader.h"
#include "xml_reader.h"
#include "yaml_reader.h"

namespace DBBinder
{

static String fileName;

void parseSQL(const String& _fileName)
{
	fileName = _fileName;

	std::ifstream file(fileName.c_str());
	if ( file.good() )
	{
		char buffer[4096];

		int line = 0;

		// Find USE param FIRST! to load generator
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

				String path( getFilenameRelativeTo(fileName, start) );

				if ( strcasecmp(ext, "yaml") == 0 )
				{
					parseYAML(path);
					break;
				}
				else if ( strcasecmp(ext, "xml") == 0 )
				{
					parseXML(path);
					break;
				}
				else
				{
					FATAL(fileName << ':' << line << ": unknown file extension: " << ext);
				}
			}
		}

		AbstractGenerator *generator = AbstractGenerator::getGenerator();

		// TODO: Check for 'database' param optionally
		if ( !generator )
		{
			FATAL(fileName << ": could not find 'use' param.");
		}

		//TODO: Determine if this is a select, insert or update statement
		SelectElements elements;

		file.clear();
		file.seekg(0);
		line = 0;

		SQLTypes type;
		String name, defaultValue, strType;
		int index;

		// Load all other params now that we got a generator
		while ( file.good() )
		{
			file.getline(buffer, 4096);
			line++;

			switch( buffer[4] )
			{
				case 'n':
					if ( strncmp(buffer, "--! name", 8 ) == 0 && isblank(buffer[8]) )
					{
						ListString list = stringTok( &buffer[9] );

						if ( list.size() == 0 )
							FATAL(fileName << ':' << line << ": missing name argument");

						if ( list.size() > 1 )
							WARNING(fileName << ':' << line << ": ignoreing extra params");

						elements.name = list.front();
					}
					break;
				case 'p':
					if ( strncmp(buffer, "--! param", 9 ) == 0 && isblank(buffer[9]) )
					{
						ListString list = stringTok( &buffer[9] );

						if ( list.size() == 0 )
							FATAL(fileName << ':' << line << ": missing param arguments");

						if ( list.size() == 1 )
							FATAL(fileName << ':' << line << ": missing param type");

						name.clear();
						defaultValue.clear();
						index = -1;
						type = stUnknown;

						int i = 0;
						for(ListString::const_iterator it = list.begin(); it != list.end(); it++, i++)
						{
							switch( i )
							{
								case 0: name = *it; break;
								case 1: type = typeNameToSQLType(*it); break;
								case 2: defaultValue = *it; break;
								case 3: index = atoi( it->c_str() ); break;
							}
						}

						if ( type == stUnknown )
							FATAL(fileName << ':' << line << ": illegal param type");

						elements.input.push_back( SQLElement( name, type, index, defaultValue ));
					}
					break;
			}
		}

		// Load the SQL statement
		// We need to 'jump' all the starting lines so engines like SQLite don't get confused with comments and
		// blank lines.
		{
			file.clear();
			file.seekg(0, std::ios_base::end);
			size_t size = file.tellg();
			file.seekg(0);

			while ( file.good() )
			{
				file.getline(buffer, 4096);
				if ( strncmp(buffer, "--", 2 ) != 0 )
				{
					char *c = buffer;
					while( *c )
					{
						if ( !isblank( *c ) )
						{
							file.seekg(file.gcount() * -1, std::ios_base::cur);
							size -= file.tellg();

							char* str = new char[size];
							file.read( str, size );
							elements.sql = str;
							delete str;

							goto END_READ_SQL;
						}
						else
							c++;
					}
				}
			}
		}

		END_READ_SQL:

		generator->addSelect( elements );
	}
	else
	{
		FATAL(fileName << ": unable to open file.");
	}
}

}
