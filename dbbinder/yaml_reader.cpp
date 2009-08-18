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

#include <errno.h>
#include <yaml.h>

#include "main.h"
#include "yaml_reader.h"

namespace DBBinder
{

static String fileName;
static FILE *yamlFile = 0;

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
		FATAL("YAML: " << fileName << ": " << problem << " " << context << " at line " << mark.line << " at col " << mark.column);
	}
}

static void parseYAMLDatabase(yaml_parser_t &parser, AbstractGenerator **_generator)
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

	if ( !*_generator )
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
				WARNING("YAML: " << fileName << ": Unknown YAML event: " << event.type << " in database");
		}
	}
}

static void getYAMLParams(yaml_parser_t &parser, AbstractElements* _elements)
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
					else if ( attr == "include" )
					{
						String path( getFilenameRelativeTo(fileName, value) );

						std::ifstream sql(path.c_str());
						if ( sql.good() )
						{
							sql.seekg(0, std::ios_base::end);
							int size = sql.tellg();
							sql.seekg(0);

							char *buffer = static_cast<char*>( malloc( size + 1 ) );

							sql.read(buffer, size);
							buffer[size] = '\0';

							_elements->sql = buffer;

							free( buffer );
						}
						else
						{
							FATAL(DBBinder::fileName << ": include file not found: " << value);
						}
					}
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
				WARNING("YAML: " << fileName << ": Unknown YAML event: " << event.type << " in params");
		}

		yaml_event_delete(&event);
	}
}

static void parseYAMLSelect(yaml_parser_t &parser, AbstractGenerator **_generator)
{
	SelectElements elements;
	getYAMLParams( parser, &elements );
	(*_generator)->addSelect( elements );
}

static void parseYAMLInsert(yaml_parser_t &parser, AbstractGenerator **_generator)
{
	InsertElements elements;
	getYAMLParams( parser, &elements );
	(*_generator)->addInsert( elements );
}

static void parseYAMLUpdate(yaml_parser_t &parser, AbstractGenerator **_generator)
{
	UpdateElements elements;
	getYAMLParams( parser, &elements );
	(*_generator)->addUpdate( elements );
}

static void parseYAMLExtra(yaml_parser_t &parser, AbstractGenerator **_generator)
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
								SQLTypes type = typeNameToSQLType( attr );

								if ( type == stUnknown )
								{
									WARNING("unknown type for: " << attr);
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
				WARNING("YAML: " << fileName << ": Unknown YAML event: " << event.type << " in extras");
		}

		yaml_event_delete(&event);
	}
}

void parseYAML(const String& _fileName, AbstractGenerator **_generator)
{
	fileName = _fileName;

	yaml_parser_t parser;
	yaml_event_t event;

	/* Create the Parser object. */
	yaml_parser_initialize(&parser);

	/* Set a file input. */
	yamlFile = fopen(fileName.c_str(), "rb");
	if ( !yamlFile )
	{
		FATAL(fileName << ": " << strerror(errno));
	}

	yaml_parser_set_input_file(&parser, yamlFile);

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
							parseYAMLDatabase(parser, _generator);
						break;

					case 'e':
						if ( value == "extra" )
							parseYAMLExtra(parser, _generator);
						break;

					case 'i':
						if ( value == "insert" )
							parseYAMLInsert(parser, _generator);
						break;

					case 's':
						if ( value == "select" )
							parseYAMLSelect(parser, _generator);
						break;

					case 'u':
						if ( value == "update" )
							parseYAMLUpdate(parser, _generator);
						break;
					default:
						WARNING(fileName << ": Unknown value: " << value);
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
}

}
