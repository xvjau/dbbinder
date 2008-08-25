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

#define DEFAULT_TEMLPATE "c++,boost"

namespace DBBinder
{

String		appName;
const char*	optFileName = 0;
String		optOutput;
const char*	optTemplate = 0;
ListString	optTemplateDirs;
bool		optXML = false;
bool		optYAML = false;

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
			case '"':
				result += "\\\"";
				break;
			default:
				result += c;
		}
	}
	return result;
}

void getXMLParams(XMLElementPtr _elem, AbstractElements* _elements)
{
	{
		XMLElementPtr sql = _elem->FirstChildElement("sql");
		_elements->sql = sql->GetText();
	}

	SQLTypes type;
	String name, defaultValue, strType;
	int index;
	
	XMLElementPtr param;
	XMLNodePtr node = 0;
	while( node = _elem->IterateChildren( "param", node ))
	{
		param = node->ToElement();
				
		param->GetAttribute( "name", &name );
		param->GetAttributeOrDefault( "type", &strType, "" );
		param->GetAttributeOrDefault( "default", &defaultValue, "" );
		param->GetAttributeOrDefault( "index", &index, -1 );

		type = typeNameToSQLType(strType);
		if ( type == stUnknown )
		{
			WARNING("unknown param type for: " << name);
			type = stText;
		}
		
		_elements->input.push_back( SQLElement( name, type, index, defaultValue ));
	}
}

void parseXML()
{
	try
	{
		XMLDocument xmlFile( optFileName );
		xmlFile.LoadFile();
		
		XMLElementPtr xml = xmlFile.FirstChildElement("xml");

		XMLElementPtr db = xml->FirstChildElement("database");
		AbstractGenerator *generator = AbstractGenerator::getGenerator( db->FirstChildElement("type")->GetText() );

		XMLElementPtr elem;
		XMLNodePtr node, subnode, valnode;

		node = 0;
		while( node = db->IterateChildren( node ))
		{
			elem = node->ToElement();

			String str;
			elem->GetAttribute( "type", &str, false );

			if ( stringToLower(str) == "int" )
			{
				generator->setDBParam( elem->Value(), atoi( elem->GetText().c_str() ));
			}
			else
				generator->setDBParam( elem->Value(), elem->GetText() );
		}

		node = 0;
		while( node = xml->IterateChildren( "select", node ))
		{
			elem = node->ToElement();
			
			SelectElements elements;
			elem->GetAttribute( "name", &elements.name );
			getXMLParams( elem, &elements );
			
			generator->addSelect( elements );
		}

		node = 0;
		while( node = xml->IterateChildren( "update", node ))
		{
			elem = node->ToElement();
			
			UpdateElements elements;
			elem->GetAttribute( "name", &elements.name );
			getXMLParams( elem, &elements );
			
			generator->addUpdate( elements );
		}

		node = 0;
		while( node = xml->IterateChildren( "insert", node ))
		{
			elem = node->ToElement();
			
			InsertElements elements;
			elem->GetAttribute( "name", &elements.name );
			getXMLParams( elem, &elements );
			
			generator->addInsert( elements );
		}

		node = 0;
		while( node = xml->IterateChildren( "extra", node ))
		{
			subnode = 0;
			while( subnode = node->IterateChildren( "types", subnode ))
			{
				valnode = 0;
				while( valnode = subnode->IterateChildren( valnode ))
				{
					elem = valnode->ToElement();

					String str;
					elem->GetText( &str, false );
					if ( str.empty() )
						elem->GetAttribute( "type", &str, false );

					if ( str.length() )
					{
						SQLTypes type = typeNameToSQLType( elem->Value() );
						
						if ( type == stUnknown )
						{
							WARNING("unknown type for: " << elem->Value());
						}
						else
							generator->setType( type, str );
					}
				}
			}

			subnode = 0;
			while( subnode = node->IterateChildren( "namespaces", subnode ))
			{
				valnode = 0;
				while( valnode = subnode->IterateChildren( "namespace", valnode ))
				{
					elem = valnode->ToElement();

					String str;
					elem->GetText( &str, false );
					if ( str.empty() )
						elem->GetAttribute( "name", &str, false );

					if ( !str.empty() )
						generator->addNamespace( str );
				}
			}

			subnode = 0;
			while( subnode = node->IterateChildren( "headers", subnode ))
			{
				valnode = 0;
				while( valnode = subnode->IterateChildren( "header", valnode ))
				{
					elem = valnode->ToElement();

					String str;
					elem->GetText( &str, false );
					if ( str.empty() )
						elem->GetAttribute( "value", &str, false );

					if ( !str.empty() )
						generator->addHeader( str );
				}
			}
		}
		
		generator->generate();
	}
	catch( ticpp::Exception &e )
	{
		FATAL("XML: " << optFileName << ": " << e.what());
	}
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
			"	--yaml		set the input format to YAML\n"
			"	-d DIR		add a template directory\n"
			"	-t FOO[,BAR]	set the template and optional sub-template (default: " << DEFAULT_TEMLPATE << ")\n"
			<< std::endl;
}

int main(int argc, char *argv[])
{
	DBBinder::appName = DBBinder::extractFileName( argv[0] );

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
						
						DBBinder::optFileName = argv[i];
					}
					else
						FATAL("missing input file name");
					
					break;
				}
				case commandOptionCode::XML:
				{
					if ( DBBinder::optYAML )
						FATAL("cannot set both XML and YAML flags");
					
					DBBinder::optXML = true;
					break;
				}
				case commandOptionCode::YAML:
				{
					if ( DBBinder::optXML )
						FATAL("cannot set both XML and YAML flags");

					DBBinder::optYAML = true;
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
			}
		}
		else
		{
			FATAL("illegal option: " << argv[i]);
		}
	}
	
	if ( !DBBinder::optFileName )
		FATAL("missing input file name");

	if ( !DBBinder::optTemplate )
		DBBinder::optTemplate = DEFAULT_TEMLPATE;

	if ( DBBinder::optOutput.empty() )
	{
		using namespace DBBinder;
		optOutput = optFileName;
		size_t pos = optOutput.find_last_of('.');
		if ( pos == String::npos )
		{
			optOutput += "_out";
		}
		else
		{
			optOutput = optOutput.substr(0, pos);
		}
	}
	
	i = -1;
	while ( DBBinder::defaultTemplateDirs[++i] )
	{
		// Check to see if default dirs exists before adding them
		struct stat fs;
		if (( stat(DBBinder::defaultTemplateDirs[i], &fs) == 0 ) && (( fs.st_mode & S_IFMT ) != S_IFDIR ))
			DBBinder::optTemplateDirs.push_back( DBBinder::defaultTemplateDirs[i] );
	}

	if ( DBBinder::optXML || !DBBinder::optYAML )
		DBBinder::parseXML();
	else
		FATAL("YAML parser not implemented yet. sorry.");
	
	return 0;
}

