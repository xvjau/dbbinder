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

#include "sqlitegenerator.h"

#define SQLFATAL(STR) FATAL( "SQLite3: " << STR << sqlite3_errmsg(m_db) )
#define SQLCHECK(STR) { if ( ret != SQLITE_OK ) FATAL( STR << sqlite3_errmsg(m_db) ); }

namespace DBBinder
{

SQLiteGenerator::SQLiteGenerator(): AbstractGenerator(),
        m_db( 0 )
{
    m_dbengine = "sqlite3";

    std::cout << "Using SQLite version: " << sqlite3_libversion() << std::endl;
}

SQLiteGenerator::~SQLiteGenerator()
{
    if ( m_db )
        sqlite3_close( m_db );
}

bool SQLiteGenerator::checkConnection()
{
    if ( !m_connected )
    {
        std::string dbName = m_dbParams["file"].value;

        if ( dbName.empty() )
            FATAL( "SQLite3: 'file' db parameter is empty." );

        int ret = sqlite3_open( dbName.c_str(), &m_db );

        if ( ret )
        {
            SQLFATAL( "Can't open database: " );
            sqlite3_close( m_db );
        }
        else
            m_connected = true;
    }

    return m_connected;
}

std::string SQLiteGenerator::getBind( SQLStatementTypes _type, const ListElements::iterator & _item, int _index )
{
    // TODO Abstract this
    std::stringstream str;
    str << "SQLCHECK( sqlite3_bind_";

    std::string typeName;
    switch( _type )
    {
        case sstSelect: typeName = "select"; break;
        case sstInsert: typeName = "insert"; break;
        case sstUpdate: typeName = "update"; break;
        case sstDelete: typeName = "delete"; break;
        default:
            FATAL("Uknown type!");
    }

    switch ( _item->type )
    {
        case stUnknown:
        {
            FATAL( "BUG BUG BUG! " << __FILE__ << __LINE__ );
        }

        case stInt:
        case stUInt:
        case stInt64:
        case stUInt64:
        {
            str << "int";
            break;
        }

        case stFloat:
        case stUFloat:
        case stDouble:
        case stUDouble:
        {
            str << "double";
            break;
        }

        case stTimeStamp:
        case stTime:
        case stDate:
        case stText:
        {
            str << "text";
            str << "(m_" << typeName << "Stmt, " << _index + 1 << ", _" << _item->name << ", strlen(_" << _item->name << "), ";

            /*
                In selects, it is unknown how long SQLite might wish to hold the string.
                In other cases, we know that the statement will be reset with the caller function so no need to
                keep the string.
            */
            if ( _type == sstSelect )
                str << "SQLITE_TRANSIENT";
            else
                str << "SQLITE_STATIC";

            str << " ));";

            return str.str();
        }

    }

    str << "(m_" << typeName << "Stmt, " << _index + 1 << ", _" << _item->name << "));";

    return str.str();
}

std::string SQLiteGenerator::getReadValue( SQLStatementTypes _type, const ListElements::iterator & _item, int _index )
{
    // TODO Abstract this
    std::string typeName;
    switch( _type )
    {
        case sstSelect: typeName = "select"; break;
        case sstInsert: typeName = "insert"; break;
        case sstUpdate: typeName = "update"; break;
        case sstDelete: typeName = "delete"; break;
        default:
            FATAL("Uknown type!");
    }

    std::stringstream str;

    switch ( _item->type )
    {
        case stUnknown:
        {
            FATAL( "BUG BUG BUG! " << __FILE__ << __LINE__ );
        }

        case stInt:
        case stUInt:
        case stInt64:
        case stUInt64:
        {
            str << "m_" << _item->name << " = sqlite3_column_int(_parent->m_" << typeName << "Stmt, " << _index << ");";
            break;
        }

        case stFloat:
        case stUFloat:
        case stDouble:
        case stUDouble:
        {
            str << "m_" << _item->name << " = sqlite3_column_double(_parent->m_" << typeName << "Stmt, " << _index << ");";
            break;
        }

        case stTimeStamp:
        case stTime:
        case stDate:
        case stText:
        {
            str << "m_" << _item->name << " = reinterpret_cast<const char*>( sqlite3_column_text(_parent->m_" << typeName << "Stmt, " << _index << ") );";
            break;
        }
    }

    return str.str();
}

std::string SQLiteGenerator::getIsNull(SQLStatementTypes _type, const ListElements::iterator& _item, int _index)
{
    // TODO Abstract this
    std::stringstream str;

    str << "( sqlite3_column_type( _parent->m_selectStmt, " << _index << " ) == SQLITE_NULL );";

    return str.str();
}

sqlite3_stmt *SQLiteGenerator::execSQL( AbstractElements &_elements )
{
    checkConnection();

    const char *tail = 0;
    sqlite3_stmt *stmt = 0;

    int ret = sqlite3_prepare( m_db, _elements.sql.c_str(), _elements.sql.length(), &stmt,  &tail );

    if ( ret != SQLITE_OK )
    {
        int line = _elements.sql_location.line;
        int col = std::max( _elements.sql_location.col, 1 );

        std::string err = sqlite3_errmsg( m_db );

        //This try is to avoid unnecessary exception when looking for the error message.
        try
        {
            if ( err.find( "near" ) != std::string::npos )
            {
                std::string::size_type pos = err.rfind( '"' );
                if ( pos != std::string::npos )
                {
                    err.erase( 0, err.find( '"' ) + 2 );
                    err.erase( pos - 1, std::string::npos );
                }
            }
            else
            {
                err.erase( 0, err.find( ':' ) + 2 );
            }

            std::string::size_type pos = _elements.sql.find( err );

            const char *s = _elements.sql.c_str();
            const char *e = s + pos;

            while ( s < e )
            {
                if ( *s++ == '\n' )
                {
                    col = 1;
                    line++;
                }
                else
                    col++;
            }
        }
        catch(std::logic_error &err)
        {
        }

        FATAL( _elements.sql_location.file << ':' << line << ':' << col << ": error " << sqlite3_errmsg( m_db ) );
    }

    return stmt;
}

void SQLiteGenerator::addSelect( SelectElements _elements )
{
    sqlite3_stmt *stmt = execSQL( _elements );

    int ret, i = 1;

    for ( ListElements::iterator it = _elements.input.begin();
            it != _elements.input.end(); ++it, ++i
        )
    {
        if ( !it->defaultValue.empty() )
        {
            ret = sqlite3_bind_text( stmt, i, it->defaultValue.c_str(), -1, 0 );
            SQLCHECK( "SQL bind error: " << ret << " " );
        }
    }

    int count = sqlite3_column_count( stmt );

    const char* typeStr;
    SQLTypes type;
    std::string name;

    for ( int i = 0; i < count; ++i )
    {
        name = sqlite3_column_name( stmt, i );
        typeStr = sqlite3_column_decltype( stmt, i );

        if ( typeStr )
        {
            switch ( *typeStr )
            {
                case 'i': // Integer
                    type = stInt;
                    break;
                case 'f': // Float
                    type = stFloat;
                    break;
                case 'b': // blob
                case 't': // text
                default:
                    type = stText;
            };
        }
        else
            type = stText;

        _elements.output.push_back( SQLElement( name, type, i ) );
    }

    sqlite3_finalize( stmt );

    AbstractGenerator::addSelect( _elements );
}

void SQLiteGenerator::addUpdate( UpdateElements _elements )
{
    execSQL( _elements );
    AbstractGenerator::addUpdate( _elements );
}

void SQLiteGenerator::addInsert( InsertElements _elements )
{
    execSQL( _elements );
    AbstractGenerator::addInsert( _elements );
}

void SQLiteGenerator::addDelete( DeleteElements _elements )
{
    execSQL( _elements );
    AbstractGenerator::addDelete( _elements );
}


}
