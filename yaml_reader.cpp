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
#include "abstractgenerator.h"
#include "yaml_reader.h"

namespace DBBinder
{

static std::string fileName;
static FILE *yamlFile = 0;

void sendYAMLError(yaml_parser_t &_parser, const char * const _err = NULL)
{
    if ( _err )
    {
        /* Destroy the Parser object. */
        yaml_parser_delete(&_parser);

        fclose(yamlFile);
        FATAL(fileName << ": error: " << _err);
    }
    else
    {
        const char *problem = _parser.problem;
        const char *context = _parser.context;
        yaml_mark_t mark = _parser.problem_mark;

        /* Destroy the Parser object. */
        yaml_parser_delete(&_parser);

        fclose(yamlFile);
        FATAL(fileName << ":" << mark.line + 1 << ":" << mark.column << ": error: " << problem << " " << context);
    }
}

inline void getYAMLEvent(yaml_parser_t &_parser, yaml_event_t& _event)
{
    if (!yaml_parser_parse(&_parser, &_event))
        sendYAMLError(_parser);
}

static void parseYAMLDatabase(std::string rootDir, yaml_parser_t &_parser)
{
    yaml_event_t event;
    getYAMLEvent(_parser, event);

    if ( event.type != YAML_MAPPING_START_EVENT )
        sendYAMLError(_parser, "Expected YAML_MAPPING_START_EVENT");

    getYAMLEvent(_parser, event);
    if ( event.type != YAML_SCALAR_EVENT )
        sendYAMLError(_parser, "Expected YAML_SCALAR_EVENT");

    if ( strcasecmp(reinterpret_cast<const char*>(event.data.scalar.value), "type") != 0)
        sendYAMLError(_parser, "Expected database->type");

    getYAMLEvent(_parser, event);
    if ( event.type != YAML_SCALAR_EVENT )
        sendYAMLError(_parser, "Expected YAML_SCALAR_EVENT");

    AbstractGenerator *generator = AbstractGenerator::getGenerator( reinterpret_cast<const char*>(event.data.scalar.value) );

    generator->setDBParam( "rootDir", rootDir );

    bool done = false;

    std::string attr;

    while ( !done )
    {
        getYAMLEvent(_parser, event);

        switch( event.type )
        {
            case YAML_MAPPING_START_EVENT: // A MAPPING-START event.
                break;
            case YAML_MAPPING_END_EVENT:
                done = true;
                break;
            case YAML_SCALAR_EVENT:
            {
                std::string value(reinterpret_cast<const char*>(event.data.scalar.value));

                if ( attr.empty() )
                    attr = value;
                else
                {
                    generator->setDBParam( attr, value );
                    attr.clear();
                }
                break;
            }

            default:
            {
                WARNING(fileName << ": warning: Unknown YAML event: " << event.type << " in params");
            }
        }
    }
}

static void getYAMLParams(yaml_parser_t &_parser, AbstractElements* _elements)
{
    yaml_event_t event;

    getYAMLEvent(_parser, event);
    if ( event.type != YAML_MAPPING_START_EVENT )
        sendYAMLError(_parser, "Expected YAML_MAPPING_START_EVENT");
    yaml_event_delete(&event);

    getYAMLEvent(_parser, event);
    if ( event.type != YAML_SCALAR_EVENT )
        sendYAMLError(_parser, "Expected YAML_SCALAR_EVENT");

    if ( strcasecmp(reinterpret_cast<const char*>(event.data.scalar.value), "name") != 0)
        sendYAMLError(_parser, "Expected select->name");

    yaml_event_delete(&event);

    getYAMLEvent(_parser, event);
    if ( event.type != YAML_SCALAR_EVENT )
        sendYAMLError(_parser, "Expected YAML_SCALAR_EVENT");

    _elements->name = reinterpret_cast<const char*>(event.data.scalar.value);
    yaml_event_delete(&event);

    bool done = false;

    std::string attr;

    SQLTypes type;
    std::string name, defaultValue, strType;
    int index;

    while ( !done )
    {
        getYAMLEvent(_parser, event);

        switch( event.type )
        {
            case YAML_MAPPING_START_EVENT: // A MAPPING-START event.
                break;
            case YAML_MAPPING_END_EVENT:
                done = true;
                break;
            case YAML_SCALAR_EVENT:
            {
                std::string value(reinterpret_cast<const char*>(event.data.scalar.value));

                if ( attr.empty() )
                    attr = value;
                else
                {
                    if ( attr == "sql" )
                        _elements->sql = value;
                    else if ( attr == "include" )
                    {
                        std::string path( getFilenameRelativeTo(fileName, value) );

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
                            sendYAMLError(_parser, (std::string("include file not found: ") + value).c_str());
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
                            getYAMLEvent(_parser, event);

                            if ( event.type == YAML_SCALAR_EVENT )
                            {
                                value = std::string(reinterpret_cast<const char*>(event.data.scalar.value));

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
                            WARNING(fileName << ": warning: Unknown YAML event: " << event.type << " in params");
                            type = stText;
                        }

                        _elements->input.push_back( SQLElement( name, type, index, defaultValue ));
                    }

                    attr.clear();
                }
                break;
            }

            default:
                WARNING(fileName << ": warning: Unknown YAML event: " << event.type << " in params");
        }

        yaml_event_delete(&event);
    }
}

static void parseYAMLSelect(yaml_parser_t &_parser)
{
    SelectElements elements;
    getYAMLParams( _parser, &elements );
    AbstractGenerator::getGenerator()->addSelect( elements );
}

static void parseYAMLInsert(yaml_parser_t &_parser)
{
    InsertElements elements;
    getYAMLParams( _parser, &elements );
    AbstractGenerator::getGenerator()->addInsert( elements );
}

static void parseYAMLUpdate(yaml_parser_t &_parser)
{
    UpdateElements elements;
    getYAMLParams( _parser, &elements );
    AbstractGenerator::getGenerator()->addUpdate( elements );
}

static void parseYAMLExtra(yaml_parser_t &_parser)
{
    yaml_event_t event;

    getYAMLEvent(_parser, event);
    if ( event.type != YAML_MAPPING_START_EVENT )
        sendYAMLError(_parser, "Expected YAML_MAPPING_START_EVENT");
    yaml_event_delete(&event);

    std::string sequence;
    AbstractGenerator *generator = AbstractGenerator::getGenerator();

    bool done = false;
    while( !done )
    {
        getYAMLEvent(_parser, event);

        switch( event.type )
        {
            case YAML_MAPPING_END_EVENT:
                done = true;
                break;

            case YAML_SCALAR_EVENT:
            {
                std::string value(reinterpret_cast<const char*>(event.data.scalar.value));

                if ( value == "types" )
                {
                    getYAMLEvent(_parser, event);
                    if ( event.type != YAML_MAPPING_START_EVENT )
                        sendYAMLError(_parser, "Expected YAML_MAPPING_START_EVENT");
                    yaml_event_delete(&event);

                    std::string attr;

                    bool typesDone = false;
                    while( !typesDone )
                    {
                        getYAMLEvent(_parser, event);

                        if( event.type == YAML_SCALAR_EVENT )
                        {
                            value = std::string(reinterpret_cast<const char*>(event.data.scalar.value));

                            if ( attr.empty() )
                                attr = value;
                            else
                            {
                                SQLTypes type = typeNameToSQLType( attr );

                                if ( type == stUnknown )
                                {
                                    WARNING(fileName << ": warning: unknown type for: " << attr);
                                }
                                else
                                    generator->setType( type, value );

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
                    WARNING(fileName << ": warning: Unknown extra: " << value);
                break;
            }
            case YAML_SEQUENCE_START_EVENT:
            {
                bool sequenceDone = false;
                while( !sequenceDone )
                {
                    getYAMLEvent(_parser, event);

                    if( event.type == YAML_SCALAR_EVENT )
                    {
                        std::string value(reinterpret_cast<const char*>(event.data.scalar.value));

                        if ( sequence == "namespaces")
                            generator->addNamespace( value );
                        else if ( sequence == "headers")
                            generator->addHeader( value );
                        else
                            WARNING(fileName << ": warning: Unknown sequence: " << sequence);
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
                WARNING(fileName << ": warning: Unknown YAML event: " << event.type << " in extras");
        }

        yaml_event_delete(&event);
    }
}

static void parseYAMLHeaders(yaml_parser_t &_parser)
{
    yaml_event_t event;

    getYAMLEvent(_parser, event);
    if ( event.type != YAML_SEQUENCE_START_EVENT )
        sendYAMLError(_parser, "Expected YAML_MAPPING_START_EVENT");
    yaml_event_delete(&event);

    std::string sequence;
    AbstractGenerator *generator = AbstractGenerator::getGenerator();

    bool done = false;
    while( !done )
    {
        getYAMLEvent(_parser, event);

        switch( event.type )
        {
            case YAML_SEQUENCE_END_EVENT:
                done = true;
                break;

            case YAML_SCALAR_EVENT:
            {
                std::string value(reinterpret_cast<const char*>(event.data.scalar.value));
                generator->addHeader( value );
                break;
            }
            default:
                WARNING(fileName << ": warning: Unknown YAML event: " << event.type << " in header");
        }

        yaml_event_delete(&event);
    }
}

void parseYAML(const std::string& _fileName)
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

    DBBinder::optDepends.push_back( fileName );

    yaml_parser_set_input_file(&parser, yamlFile);

    std::string tagName;

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
                std::string value(reinterpret_cast<const char*>(event.data.scalar.value));

                switch( value[0] )
                {
                    case 'd':
                        if ( value == "database" )
                            parseYAMLDatabase(getFilenameRelativeTo(fileName, ""), parser);
                        break;

                    case 'e':
                        if ( value == "extra" )
                            parseYAMLExtra(parser);
                        break;

                    case 'h':
                        if ( value == "headers" )
                            parseYAMLHeaders(parser);
                        break;

                    case 'i':
                        if ( value == "insert" )
                            parseYAMLInsert(parser);
                        break;

                    case 's':
                        if ( value == "select" )
                            parseYAMLSelect(parser);
                        break;

                    case 'u':
                        if ( value == "update" )
                            parseYAMLUpdate(parser);
                        break;
                    default:
                        WARNING(fileName << ": warning: Unknown value: " << value);
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
