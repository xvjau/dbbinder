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
                                atoi( port.c_str() ), 0, CLIENT_COMPRESS | CLIENT_MULTI_RESULTS) )
        {
            FATAL("Mysql connect:" << mysql_error(m_conn));
        }
        else
            m_connected = true;
    }

    return m_connected;
}

std::string MySQLGenerator::getBind(SQLStatementTypes /*_type*/, const ListElements::iterator& /*_item*/, int /*_index*/)
{
    return "";
}

std::string MySQLGenerator::getReadValue(SQLStatementTypes _type, const ListElements::iterator& _item, int _index)
{
    UNUSED(_type);
    switch(_item->type)
    {
        case stTimeStamp:
            return std::string("m_") + _item->name + " = dbbinderConvertTime(_parent->m_buff" + _item->name + ");";
        case stText:
        {
            std::stringstream str;
            str << "m_" << _item->name << " = _parent->m_buff" << _item->name + ";\n";
            str << "_parent->m_buff" << _item->name + "[" << _item->length << "] = 0;";
            return str.str();
        }
        case stBlob:
        {
            std::stringstream str;

            str << "if (!_parent->m_" << _item->name + "IsNull)\n{";
            str <<  "m_" << _item->name << " = shared_pointer< std::vector<char> >::type(new std::vector<char>());\n";
            str <<  "m_" << _item->name << "->resize(_parent->m_" << _item->name + "Length);\n\n";

            str << "_parent->selOutBuffer[" << _index << "].buffer_length = _parent->m_" << _item->name << "Length;\n";
            str << "_parent->selOutBuffer[" << _index <<"].buffer = m_" << _item->name << "->data();\n\n";

            str << "if(_parent->m_" << _item->name + "Length)\n";
            str << "mysqlCheckStmtErr(_parent->m_selectStmt, mysql_stmt_fetch_column(_parent->m_selectStmt, &(_parent->selOutBuffer[" << _index << "]), " << _index << ", 0));\n\n";

            str << "_parent->selOutBuffer[" << _index << "].buffer_length = 0;\n";
            str << "_parent->selOutBuffer[" << _index <<"].buffer = NULL;\n";
            str << "}";

            return str.str();
        }
        default:
            return std::string("m_") + _item->name + " = _parent->m_buff" + _item->name + ";";
    }
}

std::string MySQLGenerator::getIsNull(SQLStatementTypes /*_type*/, const ListElements::iterator& _item, int /*_index*/)
{
    return "_parent->m_" + _item->name + "IsNull;";
}

void MySQLGenerator::addInsert(InsertElements _elements)
{
    checkConnection();
    AbstractGenerator::addInsert(_elements);
}

SQLTypes mySQLTypeToBinderType(enum_field_types _type)
{
    switch ( _type )
    {
        case MYSQL_TYPE_TINY:
        case MYSQL_TYPE_SHORT:
        case MYSQL_TYPE_LONG:
        case MYSQL_TYPE_LONGLONG:
        case MYSQL_TYPE_INT24:
            return stInt;

        case MYSQL_TYPE_DECIMAL:
        case MYSQL_TYPE_NEWDECIMAL:
        case MYSQL_TYPE_FLOAT:
        case MYSQL_TYPE_DOUBLE:
            return stFloat;

        case MYSQL_TYPE_TIMESTAMP:
        case MYSQL_TYPE_DATETIME:
            return stTimeStamp;

        case MYSQL_TYPE_TIME:
            return stTime;

        case MYSQL_TYPE_DATE:
        case MYSQL_TYPE_YEAR:
        case MYSQL_TYPE_NEWDATE:
            return stDate;

        case MYSQL_TYPE_BLOB:
            return stBlob;

        case MYSQL_TYPE_NULL:
        case MYSQL_TYPE_VARCHAR:
        case MYSQL_TYPE_VAR_STRING:
        case MYSQL_TYPE_STRING:
        default:
            return stText;
    };
}

void getFields(SelectElements *_elements, MYSQL_RES *_meta)
{
    MYSQL_FIELD *field = mysql_fetch_field( _meta );
    SQLTypes type;

    int i = 0;
    while( field )
    {
        type = mySQLTypeToBinderType(field->type);
        _elements->output.push_back( SQLElement( field->name, type, i++, field->length ));

        field = mysql_fetch_field( _meta );
    }
}

void MySQLGenerator::addSelect(SelectElements _elements)
{
    checkConnection();

    MYSQL_STMT *stmt = mysql_stmt_init( m_conn );

    mysqlCheckStmtErr( stmt, mysql_stmt_prepare(stmt, _elements.sql.c_str(), _elements.sql.length() ));

    //std::cout << "Param count: " << mysql_stmt_param_count(stmt) << std::endl;

    MYSQL_RES *meta = mysql_stmt_result_metadata(stmt);

    getFields(&_elements, meta);

    mysql_free_result(meta);
    mysql_stmt_close(stmt);

    AbstractGenerator::addSelect(_elements);
}

void MySQLGenerator::addUpdate(UpdateElements _elements)
{
    checkConnection();
    AbstractGenerator::addUpdate(_elements);
}

struct MySQLInBufferHolder
{    
public:
    MySQLInBufferHolder(const MySQLInBufferHolder &_other):
        intBuffer(NULL),
        doubleBuffer(NULL),
        charBuffer(NULL),
        isNull(NULL)
    { 
        MySQLInBufferHolder *rhs = const_cast<MySQLInBufferHolder*>(&_other);
        std::swap(intBuffer, rhs->intBuffer);
        std::swap(doubleBuffer, rhs->doubleBuffer);
        std::swap(charBuffer, rhs->charBuffer);
        std::swap(isNull, rhs->isNull);
    }
    
    MySQLInBufferHolder(SQLTypes _type):
        intBuffer(NULL),
        doubleBuffer(NULL),
        charBuffer(NULL)
    {
        isNull = new my_bool(1);
        
        switch ( _type )
        {
            case stInt:
            case stInt64:
            case stUInt:
            case stUInt64:
            {
                intBuffer = new int(0);
                break;
            }
            
            case stFloat:
            case stDouble:
            case stUFloat:
            case stUDouble:
            {
                doubleBuffer = new double(0);
                break;
            }
            
            case stTimeStamp:
            case stTime:
            case stDate:
            {
                FATAL("Not implemented! " << __FILE__ << __LINE__);
            }

            case stText:
            case stBlob:
            {
                charBuffer = new char[1024];
                memset(charBuffer, 0, 1024);
                break;
            }
            
            default:
            {
                FATAL("Not implemented! " << __FILE__ << __LINE__);
            }
        };
    }
    
    ~MySQLInBufferHolder()
    {
        delete intBuffer;
        delete doubleBuffer;
        delete[] charBuffer;
        delete isNull;
    }
    
    int *intBuffer;
    double *doubleBuffer;
    char *charBuffer;
    
    my_bool *isNull;
};

void MySQLGenerator::addStoredProcedure(StoredProcedureElements _elements)
{
    checkConnection();

    MYSQL_STMT *stmt = mysql_stmt_init( m_conn );

    mysqlCheckStmtErr( stmt, mysql_stmt_prepare(stmt, _elements.sql.c_str(), _elements.sql.length() ));
    
    std::vector<MySQLInBufferHolder> inBuffers;
    std::vector<MYSQL_BIND> inValues;
    
    ListElements::iterator it = _elements.input.begin(), end = _elements.input.end();
    for(; it != end; it++)
    {
        MYSQL_BIND bindValue;
        memset(&bindValue, 0, sizeof(bindValue));
        
        inBuffers.push_back(MySQLInBufferHolder(it->type));
        
        my_bool *isNull = inBuffers.rbegin()->isNull;
        bindValue.is_null = isNull;
        
        switch (it->type)
        {
            case stInt:
            case stInt64:
            case stUInt:
            case stUInt64:
            {
                int *val = inBuffers.rbegin()->intBuffer;
                
                bindValue.buffer = val;
                bindValue.buffer_type = MYSQL_TYPE_LONG;
                
                if (!it->defaultValue.empty())
                {
                    isNull = 0;
                    *val = atoi(it->defaultValue.c_str());
                }
                break;
            }
            case stFloat:
            case stDouble:
            case stUFloat:
            case stUDouble:
            {
                double *val = inBuffers.rbegin()->doubleBuffer;
                
                bindValue.buffer = val;
                bindValue.buffer_type = MYSQL_TYPE_DOUBLE;
                
                if (!it->defaultValue.empty())
                {
                    isNull = 0;
                    *val = atof(it->defaultValue.c_str());
                }
                break;
            }

            case stTimeStamp:
            case stTime:
            case stDate:
            {
                FATAL("Not implemented! " << __FILE__ << __LINE__);
            }

            case stText:
            case stBlob:
            {
                char *val = inBuffers.rbegin()->charBuffer;
                
                bindValue.buffer = val;
                bindValue.buffer_type = MYSQL_TYPE_STRING;
                
                if (!it->defaultValue.empty())
                {
                    isNull = 0;
                    strncpy(val, it->defaultValue.c_str(), 1023);
                }
                break;
            }
            
            default:
            {
                FATAL("Not implemented! " << __FILE__ << __LINE__);
            }
        };
        
        inValues.push_back(bindValue);
    }
       
    mysqlCheckStmtErr(stmt, mysql_stmt_bind_param(stmt, inValues.data()));
    mysqlCheckStmtErr(stmt, mysql_stmt_execute(stmt));

    MYSQL_RES *result = mysql_store_result(m_conn);
    MYSQL_RES *meta = mysql_stmt_result_metadata(stmt);
    
    getFields(&_elements, meta);

    mysql_free_result(result);
    mysql_stmt_close(stmt);
    
    AbstractGenerator::addStoredProcedure(_elements);
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

        case stBlob:
            _lang = "shared_pointer< std::vector<char> >::type";
            _mysql = "MYSQL_TYPE_BLOB";
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

void MySQLGenerator::addInBuffers(SQLStatementTypes _type, TemplateDictionary *_subDict, const AbstractElements *_elements)
{
    std::string langType, myType;
    int index = 0;

    if (_elements->input.empty() || _elements->input.size() == 0)
    {
        TemplateDictionary *buffDict = _subDict->AddSectionDictionary(tpl_STMT_IN_FIELDS_BUFFERS);
        buffDict->SetValue(tpl_BUFFER_DECLARE, "MYSQL_BIND inBuffer[0];\n\n" );
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

            if (_type == sstSelect || _type == sstStoredProcedure)
                decl << langType << " m_param" << field.name << ";\n";

            decl << "long unsigned m_param" << field.name << "Length;\n";
            decl << "my_bool m_param" << field.name << "IsNull;\n";

            init << "inBuffer[" << index << "].buffer_type = " << myType << ";\n";

            switch( field.type )
            {
                case stBlob:
                {
                    init << "m_param" << field.name << "IsNull = (_" << field.name << ") ? 0 : 1;\n"
                        << "m_param" << field.name << "Length = (_" << field.name << ") ? _" << field.name << "->size() : 0;\n\n";

                    init << "inBuffer[" << index << "].buffer = reinterpret_cast<void*>(_" << field.name << "->data());\n";
                    break;
                }
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

            TemplateDictionary *buffDict = _subDict->AddSectionDictionary(tpl_STMT_IN_FIELDS_BUFFERS);
            buffDict->SetValue(tpl_BUFFER_DECLARE, decl.str() );
            buffDict->SetValue(tpl_BUFFER_ALLOC, init.str() );

            ++index;
        }
    }
}

void MySQLGenerator::addOutBuffers(SQLStatementTypes /*_type*/, TemplateDictionary *_subDict, const AbstractElements *_elements)
{
    std::string langType, myType;
    int index = 0;

    foreach(SQLElement field, _elements->output)
    {
        std::stringstream init, decl;

        getMySQLTypes( field.type, langType, myType );

        if ( index == 0 )
        {
            decl << "MYSQL_BIND selOutBuffer[" << _elements->output.size() << "];\n\n";
            init << "memset(selOutBuffer, 0, sizeof(selOutBuffer));\n\n";
        }

        decl << "my_bool m_" << field.name << "IsNull;\n"
                << "long unsigned m_" << field.name << "Length;\n";

        switch(field.type)
        {
            case stBlob:
            {
                break;
            }
            case stText:
            {
                decl << langType << " m_buff" << field.name << "[" << field.length + 1 << "];\n";
                init << "selOutBuffer[" << index << "].buffer_length = " << field.length << ";\n";
                break;
            }
            default:
            {
                decl << langType << " m_buff" << field.name << ";\n";
                init << "selOutBuffer[" << index << "].buffer_length = sizeof(" << langType << ");\n";
            }
        }

        init << "selOutBuffer[" << index << "].buffer_type = " << myType << ";\n";

        if (field.type != stBlob)
            init << "selOutBuffer[" << index << "].buffer = reinterpret_cast<void *>(&m_buff" << field.name << ");\n";

        init << "selOutBuffer[" << index << "].is_null = &m_" << field.name << "IsNull;\n"
                << "selOutBuffer[" << index << "].length = &m_" << field.name << "Length;\n"
                << "\n";

        TemplateDictionary *buffDict = _subDict->AddSectionDictionary(tpl_STMT_OUT_FIELDS_BUFFERS);
        buffDict->SetValue(tpl_BUFFER_DECLARE, decl.str() );
        buffDict->SetValue(tpl_BUFFER_ALLOC, init.str() );

        ++index;
    }
}

}

