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
#define SQLCHECK(STR) { if ( ret != SQLITE_OK ) FATAL( STR << sqlite3_errmsg(m_db) ) }

namespace DBBuilder
{

SQLiteGenerator::SQLiteGenerator(): AbstractGenerator(),
								 m_db( 0 )
{
	m_dbengine = "sqlite3";
}

SQLiteGenerator::~SQLiteGenerator()
{
	if ( m_db)
		sqlite3_close(m_db);
}

bool SQLiteGenerator::checkConnection()
{
	if ( !m_connected )
	{
		String dbName = m_dbParams["file"];
		if ( dbName.empty() )
			FATAL( "SQLite3: 'file' db parameter is empty." );
		
		int ret = sqlite3_open(dbName.c_str(), &m_db);
		if( ret )
		{
			SQLFATAL( "Can't open database: " );
			sqlite3_close(m_db);
		}
		else
			m_connected = true;
	}

	return m_connected;
}

void SQLiteGenerator::addSelect(SelectElements _elements)
{
	checkConnection();
	
	const char *tail;
	sqlite3_stmt *stmt;

	int ret = sqlite3_prepare(m_db, _elements.sql.c_str(), 0, &stmt,  &tail);
	SQLCHECK("SQL statement error: " << _elements.sql << " :");

	int i = 1;
	for(ListElements::iterator it = _elements.input.begin();
		it != _elements.input.end(); ++it, ++i
	   )
	{
		if ( !it->defaultValue.empty() )
		{
			sqlite3_bind_text( stmt, i, it->defaultValue.c_str(), -1, 0 );
		}
	}

	ret = sqlite3_step( stmt );
	if (!(( ret == SQLITE_ROW ) || ( ret == SQLITE_DONE )))
	{
		FATAL( "fetch error: " << _elements.sql << " :" << sqlite3_errmsg(m_db) );
	}
	
	int count = sqlite3_column_count( stmt );
	SQLTypes type;
	String name;
	for( int i = 0; i < count; ++i )
	{
		name = sqlite3_column_name( stmt, i );
		
		switch ( sqlite3_column_type( stmt, i ) )
		{
			case SQLITE_INTEGER:
				type = stInt;
				break;
			case SQLITE_FLOAT:
				type = stFloat;
				break;
			case SQLITE_BLOB:
			case SQLITE_TEXT:
#if SQLITE_TEXT != SQLITE3_TEXT
			case SQLITE3_TEXT:
#endif
			default:
				type = stText;
		};
		
		_elements.output.push_back( SQLElement( name, type, i ));
	}

	sqlite3_finalize(stmt);

	AbstractGenerator::addSelect(_elements);
}

String SQLiteGenerator::getBind(const ListElements::iterator & _item, int _index)
{
	// TODO Abstract this
	std::stringstream str;
	str << "SQLCHECK( sqlite3_bind_";

	switch ( _item->type )
	{
		case stUnknown:
		{
			FATAL("BUG BUG BUG! " << __FILE__ << __LINE__);
		}
		case stInt:
		{
			str << "int";
			break;
		}
		case stFloat:
		case stDouble:
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
			break;
		}

	}

	str << "(m_selectStmt, " << _index + 1 << ", _" << _item->name << "));";

	return str.str();
}

String SQLiteGenerator::getReadValue(const ListElements::iterator & _item, int _index)
{
	// TODO Abstract this
	std::stringstream str;

	switch ( _item->type )
	{
		case stUnknown:
		{
			FATAL("BUG BUG BUG! " << __FILE__ << __LINE__);
		}
		case stInt:
		{
			str << "m_" << _item->name << " = sqlite3_column_int(_stmt, " << _index << ");";
			break;
		}
		case stFloat:
		case stDouble:
		{
			str << "m_" << _item->name << " = sqlite3_column_double(_stmt, " << _index << ");";
			break;
		}
		case stTimeStamp:
		case stTime:
		case stDate:
		case stText:
		{
			str << "const char *str = reinterpret_cast<const char*>( sqlite3_column_text(_stmt, " << _index << ") );\n"
				<< "if ( str ) m_" << _item->name << " = str;";
			break;
		}
	}

	return str.str();
}

}
