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

#include "mysqlgenerator.h"
#include <mysql/errmsg.h>

namespace DBBinder
{

void mysqlCheckStmtErr( MYSQL_STMT *_stmt, int _status )
{
	if ( _status )
	{
		switch( _status )
		{
			case CR_COMMANDS_OUT_OF_SYNC:
				FATAL( "Commands were executed in an improper order." );
				break;
			case CR_OUT_OF_MEMORY:
				FATAL( "Out of memory." );
				break;
			case CR_SERVER_GONE_ERROR:
				FATAL( "The MySQL server has gone away." );
				break;
			case CR_SERVER_LOST:
				FATAL( "The m_connection to the server was lost during the query." );
				break;
			case CR_UNKNOWN_ERROR:
				FATAL( "An unknown error occurred." );
				break;
			case CR_UNSUPPORTED_PARAM_TYPE:
				FATAL( "The buffer type is MYSQL_TYPE_DATE, MYSQL_TYPE_TIME, MYSQL_TYPE_DATETIME, or MYSQL_TYPE_TIMESTAMP, but the data type is not DATE, TIME, DATETIME, or TIMESTAMP." );
				break;
		}
		
		FATAL( "MySQL: " << mysql_stmt_error(_stmt) );
		exit(-1);
	}
}
	
MySQLGenerator::MySQLGenerator()
 : AbstractGenerator(), m_conn(0)
{
	m_dbengine = "mysql";
	
	std::cout << "Using MySQL version: " << mysql_get_client_info() << std::endl;
}


MySQLGenerator::~MySQLGenerator()
{
}


bool MySQLGenerator::checkConnection()
{
    if ( !m_connected )
	{
#define READ_PARAM(NAME) \
		String NAME = m_dbParams[# NAME].value; \
		if ( NAME.empty() ) \
			FATAL( "MySQL: '" # NAME "' db parameter is empty." );
		
		READ_PARAM(host);
		READ_PARAM(db);
		READ_PARAM(user);
		READ_PARAM(password);
		READ_PARAM(port);
		
		m_conn = mysql_init(0);
		
		if ( !mysql_real_connect(m_conn, host.c_str(), user.c_str(), password.c_str(), db.c_str(), 
								  atoi( port.c_str() ), 0, CLIENT_COMPRESS) )
		{
			FATAL("Mysql connect:" << mysql_error(m_conn));
		}
		else
			m_connected = true;
	}
	
	return m_connected;
}

String MySQLGenerator::getBind(const ListElements::iterator& _item, int _index)
{
	return String("m_param") + _item->name + " = _" + _item->name + ";";
}

String MySQLGenerator::getReadValue(const ListElements::iterator& _item, int _index)
{
	return String("m_buff") + _item->name + ";";
}

void MySQLGenerator::addInsert(InsertElements _elements)
{
	checkConnection();
	AbstractGenerator::addInsert(_elements);
}

void MySQLGenerator::addSelect(SelectElements _elements)
{
	checkConnection();
	
	MYSQL_STMT *stmt = mysql_stmt_init( m_conn );

	mysqlCheckStmtErr( stmt, mysql_stmt_prepare(stmt, _elements.sql.c_str(), _elements.sql.length() ));

	//std::cout << "Param count: " << mysql_stmt_param_count(stmt) << std::endl;

	MYSQL_RES *meta = mysql_stmt_result_metadata(stmt);
	
	MYSQL_FIELD *field = mysql_fetch_field( meta );
	SQLTypes type;
	
	int i = 0;
	while( field )
	{
		switch ( field->type )
		{
			case MYSQL_TYPE_TINY:
			case MYSQL_TYPE_SHORT:
			case MYSQL_TYPE_LONG:
			case MYSQL_TYPE_LONGLONG:
			case MYSQL_TYPE_INT24:
				type = stInt;
				break;
				
			case MYSQL_TYPE_DECIMAL:
			case MYSQL_TYPE_NEWDECIMAL:
			case MYSQL_TYPE_FLOAT:
			case MYSQL_TYPE_DOUBLE:
				type = stFloat;
				break;
				
			case MYSQL_TYPE_TIMESTAMP:
			case MYSQL_TYPE_DATETIME:
				type = stTimeStamp;
				break;
				
			case MYSQL_TYPE_TIME:
				type = stTime;
				break;
			
			case MYSQL_TYPE_DATE:
			case MYSQL_TYPE_YEAR:
			case MYSQL_TYPE_NEWDATE:
				type = stDate;
				break;
				
			case MYSQL_TYPE_NULL:
			case MYSQL_TYPE_VARCHAR:
			case MYSQL_TYPE_VAR_STRING:
			case MYSQL_TYPE_STRING:
			default:
				type = stText;
		};
		
		_elements.output.push_back( SQLElement( field->name, type, i++, field->length ));
		
		field = mysql_fetch_field( meta );
	}

	mysql_free_result(meta);
	mysql_stmt_close(stmt);

	AbstractGenerator::addSelect(_elements);
}

void MySQLGenerator::addUpdate(UpdateElements _elements)
{
	checkConnection();
	AbstractGenerator::addUpdate(_elements);
}

bool MySQLGenerator::needIOBuffers() const
{
	return true;
}

void getMySQLTypes(SQLTypes _type, String& _lang, String& _mysql)
{
	switch( _type )
	{
		case stUnknown:
			FATAL("BUG BUG BUG! " << __FILE__ << __LINE__);
			
		case stInt:
			_lang = "int";
			_mysql = "MYSQL_TYPE_LONG";
			break;
			
		case stFloat:
			_lang = "float";
			_mysql = "MYSQL_TYPE_FLOAT";
			break;
			
		case stDouble:
			_lang = "double";
			_mysql = "MYSQL_TYPE_DOUBLE";
			break;
			
		case stTimeStamp:
			_lang = "MYSQL_TIME";
			_mysql = "MYSQL_TYPE_TIMESTAMP";
			break;

		case stTime:
			_lang = "MYSQL_TIME";
			_mysql = "MYSQL_TYPE_TIME";
			break;

		case stDate:
			_lang = "MYSQL_TIME";
			_mysql = "MYSQL_TYPE_DATE";
			break;
			
		case stText:
		default:
		{
			_lang = "char";
			_mysql = "MYSQL_TYPE_STRING";
			break;
		}
	}

}

String MySQLGenerator::getSelInBuffers(const SelectElements* _select)
{
	std::stringstream result;
	result << "MYSQL_BIND selInBuffer[s_selectParamCount];\n";

	String langType, myType;
	int index = 0;
	foreach(SQLElement field, _select->input)
	{
		getMySQLTypes( field.type, langType, myType );
		
		if ( field.type != stText )
			result << langType << " m_param" << field.name << ";\n";
		else
			result << langType << " m_param" << field.name << "[" << field.length + 1 << "];\n";

		result << "unsigned int m_param" << field.name << "Length;\n";
		
		result << "selInBuffer[" << index << "].buffer_type = " << myType << ";\n"
				<< "selInBuffer[" << index << "].buffer = (char *)&m_param" << field.name << ";\n"
				<< "selInBuffer[" << index << "].is_null = 0;\n"
				<< "selInBuffer[" << index << "].length = &m_param" << field.length << "Length;\n";
		++index;
	}
	
	return result.str();
}

String MySQLGenerator::getSelOutBuffers(const SelectElements* _select)
{
	std::stringstream result;
	result << "MYSQL_BIND selOutBuffer[s_selectFieldCount];\n";

	String langType, myType;
	int index = 0;
	foreach(SQLElement field, _select->output)
	{
		getMySQLTypes( field.type, langType, myType );
		
		result << "MY_BOOL	m_" << field.name << "IsNull;\n"
				<< "unsigned int m_" << field.name << "Length;\n";
				
		if ( field.type != stText )
		{
			result << langType << " m_buff" << field.name << ";\n"
					<< "selOutBuffer[" << index << "].buffer_length = sizeof(" << langType << ");\n";
		}
		else
		{
			result << langType << " m_buff" << field.name << "[" << field.length + 1 << "];\n"
					<< "selOutBuffer[" << index << "].buffer_length = " << field.length << ";\n";
		}

		result << "selOutBuffer[" << index << "].buffer_type = " << myType << ";\n"
				<< "selOutBuffer[" << index << "].buffer = (char *)&m_buff" << field.name << ";\n"
				<< "selOutBuffer[" << index << "].is_null = &m_" << field.name << "IsNull;\n"
				<< "selOutBuffer[" << index << "].length = &m_" << field.name << "Length;\n";
				
		++index;
	}
	
	return result.str();
}

}

