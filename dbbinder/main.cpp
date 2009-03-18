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

#ifdef WITH_YAML
#include <yaml.h>
#endif

#include <errno.h>

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
			case '\n':
				result += " \\\n";
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

#ifdef WITH_YAML
FILE *yamlFile = 0;

inline void getYAMLEvent(yaml_parser_t &_parser, yaml_event_t& _event)
{
	if (!yaml_parser_parse(&_parser, &_event))
	{
		const char *problem = _parser.problem;
		const char *context = _parser.context;
		yaml_mark_t mark = _parser.problem_mark;

		/* Destroy the Parser object. */
		yaml_parser_delete(&_parser);

		fclose(yamlFile);
		FATAL("YAML: " << optFileName << ": " << problem << " " << context << " at line " << mark.line << " at col " << mark.column);
	}
}

void parseYAMLDatabase(yaml_parser_t &parser, AbstractGenerator **_generator)
{
	yaml_event_t event;
	getYAMLEvent(parser, event);
	
	if ( event.type != YAML_MAPPING_START_EVENT )
		FATAL("YAML: Expected YAML_MAPPING_START_EVENT");
	
	getYAMLEvent(parser, event);
	if ( event.type != YAML_SCALAR_EVENT )
		FATAL("YAML: Expected YAML_SCALAR_EVENT");

	if ( strcasecmp(reinterpret_cast<const char*>(event.data.scalar.value), "type") != 0)
		FATAL("YAML: Expected database->type");

	getYAMLEvent(parser, event);
	if ( event.type != YAML_SCALAR_EVENT )
		FATAL("YAML: Expected YAML_SCALAR_EVENT");
	
	*_generator = AbstractGenerator::getGenerator( reinterpret_cast<const char*>(event.data.scalar.value) );
	
	bool done = false;

	String attr;
	
	while ( !done )
	{
		getYAMLEvent(parser, event);

		switch( event.type )
		{
			case YAML_MAPPING_START_EVENT: // A MAPPING-START event.
				break;
			case YAML_MAPPING_END_EVENT:
				done = true;
				break;
			case YAML_SCALAR_EVENT:
			{
				String value(reinterpret_cast<const char*>(event.data.scalar.value));

				if ( attr.empty() )
					attr = value;
				else
				{
					(*_generator)->setDBParam( attr, value );
					attr.clear();
				}
				break;
			}

			default:
				WARNING("YAML: " << optFileName << ": Unknown YAML event: " << event.type << " in database");
		}
	}
}

void getYAMLParams(yaml_parser_t &parser, AbstractElements* _elements)
{
	yaml_event_t event;

	getYAMLEvent(parser, event);
	if ( event.type != YAML_MAPPING_START_EVENT )
		FATAL("YAML: Expected YAML_MAPPING_START_EVENT");
	yaml_event_delete(&event);

	getYAMLEvent(parser, event);
	if ( event.type != YAML_SCALAR_EVENT )
		FATAL("YAML: Expected YAML_SCALAR_EVENT");

	if ( strcasecmp(reinterpret_cast<const char*>(event.data.scalar.value), "name") != 0)
		FATAL("YAML: Expected select->name");
	yaml_event_delete(&event);
	
	getYAMLEvent(parser, event);
	if ( event.type != YAML_SCALAR_EVENT )
		FATAL("YAML: Expected YAML_SCALAR_EVENT");

	_elements->name = reinterpret_cast<const char*>(event.data.scalar.value);
	yaml_event_delete(&event);
	
	bool done = false;

	String attr;

	SQLTypes type;
	String name, defaultValue, strType;
	int index;
	
	while ( !done )
	{
		getYAMLEvent(parser, event);

		switch( event.type )
		{
			case YAML_MAPPING_START_EVENT: // A MAPPING-START event.
				break;
			case YAML_MAPPING_END_EVENT:
				done = true;
				break;
			case YAML_SCALAR_EVENT:
			{
				String value(reinterpret_cast<const char*>(event.data.scalar.value));

				if ( attr.empty() )
					attr = value;
				else
				{
					if ( attr == "sql" )
						_elements->sql = value;
					else if ( attr == "param" )
					{
						attr = value;
						name.clear();
						strType.clear();
						defaultValue.clear();
						index = -1;

						bool paramDone = false;
						while( !paramDone )
						{
							getYAMLEvent(parser, event);

							if ( event.type == YAML_SCALAR_EVENT )
							{
								value = String(reinterpret_cast<const char*>(event.data.scalar.value));
								
								if ( attr.empty() )
									attr = value;
								else
								{
									if ( attr == "name" )
										name = value;
									else if ( attr == "type" )
										strType = value;
									else if ( attr == "default" )
										defaultValue = value;
									else if ( attr == "index" )
										index = atoi( value.c_str() );

									attr.clear();
								}
							}
							else if ( event.type == YAML_MAPPING_END_EVENT )
							{
								paramDone = true;
							}
							
							yaml_event_delete(&event);
						}

						type = typeNameToSQLType(strType);
						if ( type == stUnknown )
						{
							WARNING("unknown param type for: " << name);
							type = stText;
						}
						
						_elements->input.push_back( SQLElement( name, type, index, defaultValue ));
					}
					
					attr.clear();
				}
				break;
			}

			default:
				WARNING("YAML: " << optFileName << ": Unknown YAML event: " << event.type << " in params");
		}

		yaml_event_delete(&event);
	}
}

void parseYAMLSelect(yaml_parser_t &parser, AbstractGenerator **_generator)
{
	SelectElements elements;
	getYAMLParams( parser, &elements );
	(*_generator)->addSelect( elements );
}

void parseYAMLInsert(yaml_parser_t &parser, AbstractGenerator **_generator)
{
	InsertElements elements;
	getYAMLParams( parser, &elements );
	(*_generator)->addInsert( elements );
}

void parseYAMLUpdate(yaml_parser_t &parser, AbstractGenerator **_generator)
{
	UpdateElements elements;
	getYAMLParams( parser, &elements );
	(*_generator)->addUpdate( elements );
}

void parseYAMLExtra(yaml_parser_t &parser, AbstractGenerator **_generator)
{
	yaml_event_t event;

	getYAMLEvent(parser, event);
	if ( event.type != YAML_MAPPING_START_EVENT )
		FATAL("YAML: Expected YAML_MAPPING_START_EVENT");
	yaml_event_delete(&event);

	String sequence;
	
	bool done = false;
	while( !done )
	{
		getYAMLEvent(parser, event);

		switch( event.type )
		{
			case YAML_MAPPING_END_EVENT:
				done = true;
				break;

			case YAML_SCALAR_EVENT:
			{
				String value(reinterpret_cast<const char*>(event.data.scalar.value));

				if ( value == "types" )
				{
					getYAMLEvent(parser, event);
					if ( event.type != YAML_MAPPING_START_EVENT )
						FATAL("YAML: Expected YAML_MAPPING_START_EVENT");
					yaml_event_delete(&event);

					String attr;
					
					bool typesDone = false;
					while( !typesDone )
					{
						getYAMLEvent(parser, event);
						
						if( event.type == YAML_SCALAR_EVENT )
						{
							value = String(reinterpret_cast<const char*>(event.data.scalar.value));
							
							if ( attr.empty() )
								attr = value;
							else
							{
								SQLTypes type = typeNameToSQLType( value );

								if ( type == stUnknown )
								{
									WARNING("unknown type for: " << value);
								}
								else
									(*_generator)->setType( type, value );
								
								attr.clear();
							}
						}
						else if( event.type == YAML_MAPPING_END_EVENT )
						{
							typesDone = true;
						}
						
						yaml_event_delete(&event);
					}
				}
				else if (( value == "namespaces" ) || ( value == "headers" ))
					sequence = value;
				else
					WARNING("YAML: Unknown extra: " << value);
				break;
			}
			case YAML_SEQUENCE_START_EVENT:
			{
				bool sequenceDone = false;
				while( !sequenceDone )
				{
					getYAMLEvent(parser, event);

					if( event.type == YAML_SCALAR_EVENT )
					{
						String value(reinterpret_cast<const char*>(event.data.scalar.value));

						if ( sequence == "namespaces")
							(*_generator)->addNamespace( value );
						else if ( sequence == "headers")
							(*_generator)->addHeader( value );
						else
							WARNING("YAML: Unknown sequence: " << sequence);
					}
					else if( event.type == YAML_SEQUENCE_END_EVENT )
					{
						sequenceDone = true;
					}
					
					yaml_event_delete(&event);
				}
				
				break;
			}
			default:
				WARNING("YAML: " << optFileName << ": Unknown YAML event: " << event.type << " in extras");
		}
		
		yaml_event_delete(&event);
	}
}

void parseYAML()
{
	yaml_parser_t parser;
	yaml_event_t event;

	/* Create the Parser object. */
	yaml_parser_initialize(&parser);

	/* Set a file input. */
	yamlFile = fopen(optFileName, "rb");
	if ( !yamlFile )
	{
		FATAL("YAML: " << optFileName << ": " << strerror(errno));
	}

	yaml_parser_set_input_file(&parser, yamlFile);

	AbstractGenerator *generator = 0;
	
	String tagName;
	
	/* Read the event sequence. */
	bool done = false;
	while (!done)
	{
		/* Get the next event. */
		getYAMLEvent(parser, event);

		switch( event.type )
		{
			case YAML_STREAM_END_EVENT: // A STREAM-END event.
				done = true;
				break;
				
			case YAML_SCALAR_EVENT: // A SCALAR event.
			{
				String value(reinterpret_cast<const char*>(event.data.scalar.value));
				
				switch( value[0] )
				{
					case 'd':
						if ( value == "database" )
							parseYAMLDatabase(parser, &generator);
						break;
						
					case 'e':
						if ( value == "extra" )
							parseYAMLExtra(parser, &generator);
						break;

					case 'i':
						if ( value == "insert" )
							parseYAMLInsert(parser, &generator);
						break;
					
					case 's':
						if ( value == "select" )
							parseYAMLSelect(parser, &generator);
						break;

					case 'u':
						if ( value == "update" )
							parseYAMLUpdate(parser, &generator);
						break;
					default:
						WARNING("YAML: Unknown value: " << value);
				}
				
				break;
			}
			default:
				break;
		}

		/* The application is responsible for destroying the event object. */
		yaml_event_delete(&event);
	}

	/* Destroy the Parser object. */
	yaml_parser_delete(&parser);

	fclose(yamlFile);

	generator->generate();
}
#endif

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

	// If not explicitly selected by the user, deduce the type fromt the file's extension
	if ( !DBBinder::optXML && !DBBinder::optYAML )
	{
		const char *c = DBBinder::optFileName;
		c += strlen( DBBinder::optFileName ) - 4;
		
		if ( strcasecmp(c, ".xml") == 0 )
		{
			DBBinder::optXML = true;
		}
		else
		{
			if ( strcasecmp(--c, ".yaml") == 0 )
				DBBinder::optYAML = true;
		}
	}
	
	if ( DBBinder::optXML || !DBBinder::optYAML )
		DBBinder::parseXML();
	else
	{
#ifdef WITH_YAML
		DBBinder::parseYAML();
#else
		FATAL("YAML not enabled in this build.");
#endif
	}
	
	return 0;
}
