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

static std::string fileName;

void parseSQL(const std::string& _fileName)
{
    fileName = _fileName;

    std::ifstream file(fileName.c_str());
    if ( file.good() )
    {
        DBBinder::optDepends.push_back( fileName );

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

                std::string path( getFilenameRelativeTo(fileName, start) );

                if ( strcasecmp(ext, "yaml") == 0 )
                {
                    #ifdef WITH_YAML
                    parseYAML(path);
                    #else
                    FATAL(fileName << ':' << line << ": yaml support was not included");
                    #endif
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
        AbstractElements *elements = 0;
        SQLStatementTypes statementType = sstUnknown;

        // Load the SQL statement
        // We need to 'jump' all the starting lines so engines like SQLite don't get confused with comments and
        // blank lines.
        {
            file.clear();
            file.seekg(0, std::ios_base::end);
            std::string::size_type size = file.tellg();
            file.seekg(0);

            line = 0;
            
            bool inComments = false;

            while ( file.good() )
            {
                file.getline(buffer, 4096);
                line++;

                #warning This should be sent to generator->db server to determine the 'correct' statement type.
                if (strncmp(buffer, "--", 2 ) != 0)
                {
                    char *c = buffer;
                    while( *c )
                    {
                        if ( !isblank( *c ) )
                        {
                            if ( strncasecmp( c, "select", 6 ) == 0 && ( !c[6] || isspace( c[6] )))
                            {
                                elements = new SelectElements;
                                statementType = sstSelect;
                            }
                            else if ( strncasecmp( c, "update", 6 ) == 0 && ( !c[6] || isspace( c[6] )))
                            {
                                elements = new UpdateElements;
                                statementType = sstUpdate;
                            }
                            else if ( strncasecmp( c, "insert", 6 ) == 0 && ( !c[6] || isspace( c[6] )))
                            {
                                elements = new InsertElements;
                                statementType = sstInsert;
                            }
                            else if ( strncasecmp( c, "delete", 6 ) == 0 && ( !c[6] || isspace( c[6] )))
                            {
                                elements = new DeleteElements;
                                statementType = sstDelete;
                            }
                            else
                            {
                                goto NEXT_LINE;
                            }

                            file.seekg(file.gcount() * -1, std::ios_base::cur);
                            size -= file.tellg();

                            char* str = static_cast<char*>(alloca(size+1));
                            file.read( str, size );
                            str[size] = '\0';

                            elements->sql = str;
                            elements->sql_location.file = _fileName;
                            elements->sql_location.line = line;
                            elements->sql_location.col = c - &buffer[0];

                            goto END_READ_SQL;
                        }
                        else
                            c++;
                    }
                }
                
                NEXT_LINE: {}
            }
        }

        END_READ_SQL:

        if (statementType == sstUnknown)
        {
            FATAL(fileName << ": unknown statement type.");
        }
        
        file.clear();
        file.seekg(0);
        line = 0;

        SQLTypes type;
        std::string name, defaultValue, strType;
        int index;

        // Load all other params now that we got a generator
        while ( file.good() )
        {
            file.getline(buffer, 4096);

            if ( file.good() )
            {
                line++;
                switch( buffer[4] )
                {
                    case 'k':
                        if ( strncmp(buffer, "--! key", 7 ) == 0 && isblank(buffer[7]) )
                        {
                            ListString list = stringTok( &buffer[8] );

                            if (statementType != sstSelect)
                                WARNING(fileName << ':' << line << ": ignoreing key param for non-select statement");

                            if ( list.size() == 0 )
                                FATAL(fileName << ':' << line << ": missing key name argument");

                            if ( list.size() > 1 )
                                WARNING(fileName << ':' << line << ": ignoreing extra params");

                            static_cast<SelectElements*>( elements )->keyFieldName = list.front();
                        }
                        break;
                    case 'n':
                        if ( strncmp(buffer, "--! name", 8 ) == 0 && isblank(buffer[8]) )
                        {
                            ListString list = stringTok( &buffer[9] );

                            if ( list.size() == 0 )
                                FATAL(fileName << ':' << line << ": missing name argument");

                            if ( list.size() > 1 )
                                WARNING(fileName << ':' << line << ": ignoreing extra params");

                            elements->name = list.front();
                        }
                        break;
                    case 'p':
                        if ( strncmp(buffer, "--! param", 9 ) == 0 && isblank(buffer[9]) )
                        {
                            ListString list = stringTok( &buffer[10] );

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

                            elements->input.push_back( SQLElement( name, type, index, defaultValue ));
                        }
                        break;
                }
            }
        }

        switch( statementType )
        {
            case sstSelect:
                generator->addSelect( *static_cast<SelectElements*>( elements ));
                break;
            case sstInsert:
                generator->addInsert( *static_cast<InsertElements*>( elements ));
                break;
            case sstUpdate:
                generator->addUpdate( *static_cast<UpdateElements*>( elements ));
                break;
            case sstDelete:
                generator->addDelete( *static_cast<DeleteElements*>( elements ));
                break;
            default:
                FATAL("Unknwon statement type.");
        }

    }
    else
    {
        FATAL(fileName << ": unable to open file.");
    }
}

}
