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
		abort();
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
		std::string NAME = m_dbParams[# NAME].value; \
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

std::string MySQLGenerator::getBind(SQLStatementTypes _type, const ListElements::iterator& _item, int _index)
{
	return "";
}

std::string MySQLGenerator::getReadValue(SQLStatementTypes _type, const ListElements::iterator& _item, int _index)
{
	UNUSED(_type);
	if (_item->type == stTimeStamp)
		return std::string("m_") + _item->name + " = dbbinderConvertTime(_parent->m_buff" + _item->name + ");";
	else
		return std::string("m_") + _item->name + " = _parent->m_buff" + _item->name + ";";
}

std::string MySQLGenerator::getIsNull(SQLStatementTypes _type, const ListElements::iterator& _item, int _index)
{
	return "_parent->m_" + _item->name + "IsNull;";
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

void getMySQLTypes(SQLTypes _type, std::string& _lang, std::string& _mysql)
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

ctemplate::TemplateDictionary* MySQLGenerator::getSubDict(SQLStatementTypes _type)
{
	switch ( _type )
	{
		case sstSelect: return m_dict->AddSectionDictionary(tpl_SEL_IN_FIELDS_BUFFERS);
		case sstInsert: return m_dict->AddSectionDictionary(tpl_INS_IN_FIELDS_BUFFERS);
		case sstUpdate: return m_dict->AddSectionDictionary(tpl_UPD_IN_FIELDS_BUFFERS);
		case sstDelete: return m_dict->AddSectionDictionary(tpl_DEL_IN_FIELDS_BUFFERS);
		default:
			FATAL(__FILE__  << ':' << __LINE__ << ": Invalide statement type.");
	};
}

void MySQLGenerator::addInBuffers(SQLStatementTypes _type, const AbstractElements* _elements)
{
    std::string langType, myType;
    int index = 0;

    if (_elements->input.empty() || _elements->input.size() == 0)
    {
        ctemplate::TemplateDictionary *subDict = getSubDict(_type);
        subDict->SetValue(tpl_BUFFER_DECLARE, "MYSQL_BIND inBuffer[0];\n\n" );
    }
    else
    {
        foreach(SQLElement field, _elements->input)
        {
            std::stringstream decl, init;

            getMySQLTypes( field.type, langType, myType );

            if ( index == 0 )
            {
                decl << "MYSQL_BIND inBuffer[" << _elements->input.size() << "];\n\n";
                init << "memset(inBuffer, 0, sizeof(inBuffer));\n\n";
            }

            if (_type == sstSelect)
                decl << langType << " m_param" << field.name << ";\n";

            decl << "long unsigned m_param" << field.name << "Length;\n";
            decl << "my_bool m_param" << field.name << "IsNull;\n";

            init << "inBuffer[" << index << "].buffer_type = " << myType << ";\n";

            switch( field.type )
            {
                case stDate:
                case stTime:
                case stTimeStamp:
                {
                    decl << "MYSQL_TIME m_param" << field.name << "MyTime;\n";

                    init << "m_param" << field.name << "IsNull = (_" << field.name << ".is_not_a_date_time()) ? 1 : 0;\n"
                        << "m_param" << field.name << "Length = sizeof(MYSQL_TIME);\n"
                        << "m_param" << field.name << "MyTime = dbbinderConvertTime(_" << field.name << ");\n\n";

                    init << "inBuffer[" << index << "].buffer = reinterpret_cast<void *>(&m_param" << field.name << "MyTime);\n";
                    break;
                }
                case stText:
                {
                    init << "m_param" << field.name << "IsNull = (_" << field.name << ") ? 0 : 1;\n"
                        << "m_param" << field.name << "Length = (_" << field.name << ") ? strlen(_" << field.name << ") : 0;\n\n";

                    init << "inBuffer[" << index << "].buffer = const_cast<void*>(reinterpret_cast<const void *>(_" << field.name << "));\n";
                    break;
                }
                default:
                {
                    init << "m_param" << field.name << "IsNull = 0;\n"
                        << "m_param" << field.name << "Length = 0;\n\n";

                    init << "inBuffer[" << index << "].buffer = reinterpret_cast<void *>(&_" << field.name << ");\n";
                    break;
                }
            }


            init << "inBuffer[" << index << "].is_null = &m_param" << field.name << "IsNull;\n"
                    << "inBuffer[" << index << "].length = &m_param" << field.name << "Length;\n"
                    << "\n";

            ctemplate::TemplateDictionary *subDict = getSubDict(_type);
            subDict->SetValue(tpl_BUFFER_DECLARE, decl.str() );
            subDict->SetValue(tpl_BUFFER_ALLOC, init.str() );

            ++index;
        }
    }
}

void MySQLGenerator::addOutBuffers(SQLStatementTypes _type, const AbstractIOElements* _elements)
{
	std::string langType, myType;
	int index = 0;
	ctemplate::TemplateDictionary *subDict;

	foreach(SQLElement field, _elements->output)
	{
		std::stringstream init, decl;

		getMySQLTypes( field.type, langType, myType );

		if ( index == 0 )
		{
			decl << "MYSQL_BIND selOutBuffer[" << _elements->output.size() << "];\n\n";
			init << "memset(selOutBuffer, 0, sizeof(selOutBuffer));\n\n";
		}

		decl << "my_bool	m_" << field.name << "IsNull;\n"
				<< "long unsigned m_" << field.name << "Length;\n";

		if ( field.type != stText )
		{
			decl << langType << " m_buff" << field.name << ";\n";
			init << "selOutBuffer[" << index << "].buffer_length = sizeof(" << langType << ");\n";
		}
		else
		{
			decl << langType << " m_buff" << field.name << "[" << field.length + 1 << "];\n";
			init << "selOutBuffer[" << index << "].buffer_length = " << field.length << ";\n";
		}

		init << "selOutBuffer[" << index << "].buffer_type = " << myType << ";\n"
				<< "selOutBuffer[" << index << "].buffer = reinterpret_cast<void *>(&m_buff" << field.name << ");\n"
				<< "selOutBuffer[" << index << "].is_null = &m_" << field.name << "IsNull;\n"
				<< "selOutBuffer[" << index << "].length = &m_" << field.name << "Length;\n"
				<< "\n";

		subDict = m_dict->AddSectionDictionary(tpl_SEL_OUT_FIELDS_BUFFERS);
		subDict->SetValue(tpl_BUFFER_DECLARE, decl.str() );
		subDict->SetValue(tpl_BUFFER_ALLOC, init.str() );

		++index;
	}
}

}

