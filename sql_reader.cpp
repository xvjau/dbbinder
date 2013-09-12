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

#include <vector>
#include <string>

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
            std::vector<std::string> tokens(stringTok(buffer));
            line++;

            if (tokens.size() > 2 && tokens[0] == "--!")
            {
                
                if (tokens[1] == "use")
                {
                    if (tokens.size() < 3)
                        FATAL(fileName << ':' << line << ": missing file name");
                    
                    std::string useFileName = tokens[2], ext;

                    std::string::size_type pos = useFileName.find(".");
                    
                    ext = useFileName.substr(pos + 1);

                    std::string path(getFilenameRelativeTo(fileName, useFileName));

                    if (ext == "yaml")
                    {
                        #ifdef WITH_YAML
                        parseYAML(path);
                        #else
                        FATAL(fileName << ':' << line << ": yaml support was not included");
                        #endif
                        break;
                    }
                    else if (ext == "xml")
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
        }

        AbstractGenerator *generator = AbstractGenerator::getGenerator();

        // TODO: Check for 'database' param optionally
        if ( !generator )
        {
            FATAL(fileName << ": could not find 'use' param.");
        }

        //TODO: Determine if this is a select, insert or update statement
        AbstractElements *elements = NULL;
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
            
            while ( file.good() )
            {
                file.getline(buffer, 4096);
                std::vector<std::string> tokens(stringTok(buffer));
                line++;

                #warning This should be sent to generator->db server to determine the 'correct' statement type.
                if (tokens.size())
                {
                    std::string firstToken = tokens[0];
                    
                    if (firstToken.size() == 0)
                        continue;
                    
                    switch(tolower(firstToken[0]))
                    {
                        case 's':
                            if (strcasecmp(firstToken.c_str(), "select") == 0)
                            {
                                elements = new SelectElements;
                                statementType = sstSelect;
                                break;
                            }
                            continue;
                        case 'u':
                            if (strcasecmp(firstToken.c_str(), "update") == 0)
                            {
                                elements = new UpdateElements;
                                statementType = sstUpdate;
                                break;
                            }
                            continue;
                        case 'i':
                            if (strcasecmp(firstToken.c_str(), "insert") == 0)
                            {
                                elements = new InsertElements;
                                statementType = sstInsert;
                                break;
                            }
                            continue;
                        case 'd':
                            if (strcasecmp(firstToken.c_str(), "delete") == 0)
                            {
                                elements = new DeleteElements;
                                statementType = sstDelete;
                                break;
                            }
                            continue;
                        case 'c':
                            if (strcasecmp(firstToken.c_str(), "call") == 0)
                            {
                                elements = new StoredProcedureElements;
                                statementType = sstStoredProcedure;
                                break;
                            }
                            continue;
                        default:
                            continue;
                    }

                    file.seekg(file.gcount() * -1, std::ios_base::cur);
                    size -= file.tellg();

                    char* str = static_cast<char*>(alloca(size+1));
                    file.read( str, size );
                    str[size] = '\0';

                    elements->sql = str;
                    elements->sql_location.file = _fileName;
                    elements->sql_location.line = line;
                    elements->sql_location.col = strstr(buffer, firstToken.c_str()) - &buffer[0];
                    
                    break;
                }
            }
        }

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
            std::vector<std::string> tokens(stringTok(buffer));
            line++;
            
            if (tokens.size() > 1 && tokens[0] == "--!" && tokens[1].size() > 1)
            {
                switch(tokens[1][0])
                {
                    case 'k':
                        if (tokens[1] == "key")
                        {
                            ListString params(tokens.begin() + 2, tokens.end());

                            if (statementType != sstSelect && statementType != sstStoredProcedure)
                                WARNING(fileName << ':' << line << ": ignoreing key param for non-select statement");

                            if ( params.size() == 0 )
                                FATAL(fileName << ':' << line << ": missing key name argument");

                            if ( params.size() > 1 )
                                WARNING(fileName << ':' << line << ": ignoreing extra params");

                            static_cast<SelectElements*>( elements )->keyFieldName = params.front();
                        }
                        break;
                    case 'n':
                        if (tokens[1] == "name")
                        {
                            ListString params(tokens.begin() + 2, tokens.end());

                            if ( params.size() == 0 )
                                FATAL(fileName << ':' << line << ": missing name argument");

                            if ( params.size() > 1 )
                                WARNING(fileName << ':' << line << ": ignoreing extra params");

                            elements->name = params.front();
                        }
                        break;
                    case 'p':
                        if (tokens[1] == "param")
                        {
                            ListString params(tokens.begin() + 2, tokens.end());

                            if ( params.size() == 0 )
                                FATAL(fileName << ':' << line << ": missing param arguments");

                            if ( params.size() == 1 )
                                FATAL(fileName << ':' << line << ": missing param type");

                            name.clear();
                            defaultValue.clear();
                            index = -1;
                            type = stUnknown;

                            int i = 0;
                            for(ListString::const_iterator it = params.begin(); it != params.end(); it++, i++)
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
            case sstStoredProcedure:
                generator->addStoredProcedure( *static_cast<StoredProcedureElements*>( elements ));
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
