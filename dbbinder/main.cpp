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
#include "abstractgenerator.h"
#include "options.h"

#include <libgen.h>

#include "sql_reader.h"
#include "xml_reader.h"
#ifdef WITH_YAML
#include "yaml_reader.h"
#endif

#define DEFAULT_TEMLPATE "c++,boost"

namespace DBBinder
{

String		appName;
String		optOutput;
const char*	optTemplate = 0;
ListString	optTemplateDirs;
const char*	optVersionMajor = "0";
const char*	optVersionMinor = "0";
bool		optListDepends = false;
ListString	optDepends;

static const char*	defaultTemplateDirs[] =
{
	"/usr/share/dbbinder/templates",
	"/usr/local/share/dbbinder/templates",
	"~/share/dbbinder/templates",
	0
};

String cescape(const String & _string)
{
	String result;
	foreach(char c, _string)
	{
		switch( c )
		{
			case '\\':
				result += "\\\\";
				break;
			case '"':
				result += "\\\"";
				break;
			case '\n':
				result += " \\\n";
				break;
			default:
				result += c;
		}
	}
	return result;
}

}

void printHelp()
{
	std::cout <<
			"Usage " << DBBinder::appName << " [options]\n"
			"Options:\n"
			"	-h, --help	print this help message\n"
			"	-i FILE		set the input file\n"
			"	-o FILE		set the output file name\n"
			"	--xml		set the input format to XML (default)\n"
#ifdef WITH_YAML
			"	--yaml		set the input format to YAML\n"
#endif
			"	--depends	do nothing, just list the dependencies for a target\n"
			"	-d DIR		add a template directory\n"
			"	-t FOO[,BAR]	set the template and optional sub-template (default: " << DEFAULT_TEMLPATE << ")\n"
			"	--database TYPE[,CONN0[,CONN1]] Database to connect and, optionally, connection params\n"
			"			example: --database MySQL,127.0.0.1,db,user,password\n"
			"	--sql COMMAND	SQL command\n"
			"			example: --sql 'select FIELD1, FIELD2 from TABLE where FIELD3 = ?'\n"
			"	--sql-param PARAM,TYPE[,DEFAULT]	PARAM for the SQL.\n"
			"			example: --param name,string,'no name' --param age,int\n"
			"	-V		version\n"
			"	--vmajor	major version\n"
			"	--vminor	minor version\n"
			<< std::endl;
}

int main(int argc, char *argv[])
{
	DBBinder::appName = DBBinder::extractFileName( argv[0] );

	char* fileName = 0;
	DBBinder::FileType fileType = DBBinder::ftNULL;

	if ( argc <= 1 )
	{
		WARNING("Missing options");
		printHelp();
		exit(-1);
	}

	const commandOption *option;
	int i = 0;

	while(++i < argc)
	{
		option = Perfect_Hash::isValid( argv[i], strlen(argv[i]) );
		if ( option )
		{
			switch( option->optionCode )
			{
				case commandOptionCode::HELPVERBOSE:
				{
					printHelp();
					exit(0);
				}
				case commandOptionCode::INPUT_FILE:
				{
					if ( ++i < argc )
					{
						struct stat fs;
						if ( stat(argv[i], &fs) )
						{
							FATAL( argv[i] << ": No such file");
						}
						else
						{
							if (( fs.st_mode & S_IFMT ) != S_IFREG )
								FATAL( argv[i] << ": must be a regular file");
						}

						fileName = argv[i];
					}
					else
						FATAL("missing input file name");

					break;
				}
				case commandOptionCode::XML:
				{
					if ( fileType != DBBinder::ftNULL )
						FATAL("cannot set more than one file type flag");

					fileType = DBBinder::ftXML;
					break;
				}

				case commandOptionCode::YAML:
				{
#ifdef WITH_YAML
					if ( fileType != DBBinder::ftNULL )
						FATAL("cannot set more than one file type flag");

					fileType = DBBinder::ftYAML;
					break;
#else
					FATAL("no yaml support");
#endif
				}

				case commandOptionCode::DEPENDS:
				{
					DBBinder::optListDepends = true;
					break;
				}

				case commandOptionCode::TEMPLATE:
				{
					DBBinder::optTemplate = argv[i];
					break;
				}
				case commandOptionCode::TEMPLATE_DIR:
				{
					if ( ++i < argc )
					{
						struct stat fs;
						if ( stat(argv[i], &fs) )
						{
							FATAL( argv[i] << ": No such directory");
						}
						else
						{
							if (( fs.st_mode & S_IFMT ) != S_IFDIR )
							{
								FATAL( argv[i] << ": must be a directory");
							}
						}

						DBBinder::optTemplateDirs.push_back( argv[i] );
					}
					else
						FATAL("missing directory name");

					break;
				}
				case commandOptionCode::OUTPUT_FILE:
				{
					if ( ++i < argc )
					{
						DBBinder::optOutput = argv[i];
					}
					else
						FATAL("missing output file name");

					break;
				}
				case commandOptionCode::VERSION:
				{
					std::cout << DBBinder::optVersionMajor << '.' << DBBinder::optVersionMinor << std::endl;
					exit(0);
				}
				case commandOptionCode::VERSION_MAJOR:
				{
					std::cout << DBBinder::optVersionMajor << std::endl;
					exit(0);
				}
				case commandOptionCode::VERSION_MINOR:
				{
					std::cout << DBBinder::optVersionMinor << std::endl;
					exit(0);
				}
				default:
					FATAL("illegal option: " << argv[i]);
			}
		}
		else
		{
			FATAL("illegal option: " << argv[i]);
		}
	}

	if ( !fileName )
		FATAL("missing input file name");

	if ( !DBBinder::optTemplate )
		DBBinder::optTemplate = DEFAULT_TEMLPATE;

	if ( DBBinder::optOutput.empty() )
	{
		using namespace DBBinder;
		optOutput = fileName;
		String::size_type pos = optOutput.rfind('.');
		if ( pos == String::npos )
		{
			optOutput += "_out";
		}
		else
		{
			optOutput = optOutput.substr(0, pos);
		}
	}


	//Add the program's own path
	{
		char buffer[4096];
		strcpy(buffer, argv[0]);
		dirname(buffer);
		strcpy(&buffer[strlen(buffer)], "/templates");

		struct stat fs;
		if (( stat(buffer, &fs) == 0 ) && S_ISDIR( fs.st_mode & S_IFMT ))
			DBBinder::optTemplateDirs.push_back( buffer );
	}
	i = -1;
	while ( DBBinder::defaultTemplateDirs[++i] )
	{
		// Check to see if default dirs exists before adding them
		struct stat fs;
		if (( stat(DBBinder::defaultTemplateDirs[i], &fs) == 0 ) && S_ISDIR( fs.st_mode & S_IFMT ))
			DBBinder::optTemplateDirs.push_back( DBBinder::defaultTemplateDirs[i] );
	}

	// If not explicitly selected by the user, deduce the type fromt the file's extension
	if ( fileType == DBBinder::ftNULL )
	{
		const char *c = fileName;
		c += strlen( fileName ) - 3;

		while( c > fileName && *c != '.')
			--c;

		switch( *(c+1) )
		{
			case 's':
				if ( strcasecmp(c, ".sql") == 0 )
				{
					fileType = DBBinder::ftSQL;
					break;
				}
			case 'x':
				if ( strcasecmp(c, ".xml") == 0 )
				{
					fileType = DBBinder::ftXML;
					break;
				}
				// no break
			case 'y':
				if ( strcasecmp(c, ".yaml") == 0 )
				{
					fileType = DBBinder::ftYAML;
					break;
				}
				// no break
			default:
				FATAL(fileName << ": unknown file extension - " << c);
		}
	}

	switch ( fileType )
	{
		case DBBinder::ftXML:
		{
			DBBinder::parseXML(fileName);
			break;
		}
		case DBBinder::ftYAML:
		{
			#ifdef WITH_YAML
			DBBinder::parseYAML(fileName);
			#else
			FATAL("no yaml support");
			#endif
			break;
		}
		case DBBinder::ftSQL:
		{
			DBBinder::parseSQL(fileName);
			break;
		}
		default:
			FATAL("Unknown file type.");
	}

	DBBinder::AbstractGenerator::getGenerator()->generate();

	if ( DBBinder::optListDepends )
	{
		using namespace DBBinder;

		std::cout << "Depends:\n";

		ListString::iterator it;
		for(it = optDepends.begin(); it != optDepends.end(); ++it)
			std::cout << *it << "\n";

		std::cout << std::flush;
	}
	return 0;
}
