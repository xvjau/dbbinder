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

#include "abstractgenerator.h"
#include "sqlitegenerator.h"

#ifdef WITH_ORACLE
#include "oraclegenerator.h"
#endif

#ifdef WITH_MYSQL
#include "mysqlgenerator.h"
#endif

#ifdef WITH_FIREBIRD
#include "firebirdgenerator.h"
#endif

#ifdef WITH_POSTGRESQL
#include "postgresqlgenerator.h"
#endif

#include "main.h"
#include "TinyXML/nvXML.h"

#include <ctype.h>
#include <apr-1/apr_poll.h>

namespace DBBinder
{

const char * const tpl_FILENAME = "FILENAME";
const char * const tpl_INTF_FILENAME = "INTF_FILENAME";
const char * const tpl_IMPL_FILENAME = "IMPL_FILENAME";

const char * const tpl_HEADER_NAME = "HEADER_NAME";
const char * const tpl_DBENGINE_INCLUDES = "DBENGINE_INCLUDES";
const char * const tpl_DBENGINE_INCLUDE_NAME = "DBENGINE_INCLUDE_NAME";
const char * const tpl_EXTRA_HEADERS = "EXTRA_HEADERS";
const char * const tpl_EXTRA_HEADERS_HEADER = "EXTRA_HEADERS_HEADER";
const char * const tpl_EXTRA_HEADERS_TYPE = "EXTRA_HEADERS_TYPE";
const char * const tpl_EXTRA_HEADERS_MEMBER = "EXTRA_HEADERS_MEMBER";
const char * const tpl_DBENGINE_GLOBAL_FUNCTIONS = "DBENGINE_GLOBAL_FUNCTIONS";
const char * const tpl_FUNCTION = "FUNCTION";

const char * const tpl_NAMESPACES = "NAMESPACES";
const char * const tpl_NAMESPACE = "NAMESPACE";

const char * const tpl_CLASS = "CLASS";
const char * const tpl_CLASSNAME = "CLASSNAME";

const char * const tpl_DBENGINE_CONNECTION_TYPE = "DBENGINE_CONNECTION_TYPE";
const char * const tpl_DBENGINE_CONNECTION_NULL = "DBENGINE_CONNECTION_NULL";

const char * const tpl_SELECT = "SELECT";
const char * const tpl_SELECT_SQL = "SELECT_SQL";
const char * const tpl_SELECT_SQL_UNESCAPED = "SELECT_SQL_UNESCAPED";
const char * const tpl_SELECT_SQL_LEN = "SELECT_SQL_LEN";
const char * const tpl_SELECT_FIELD_COUNT = "SELECT_FIELD_COUNT";
const char * const tpl_SELECT_PARAM_COUNT = "SELECT_PARAM_COUNT";

const char * const tpl_SELECT_HAS_PARAMS = "SELECT_HAS_PARAMS";

const char * const tpl_SEL_IN_FIELDS = "SEL_IN_FIELDS";
const char * const tpl_SEL_IN_FIELD_TYPE = "SEL_IN_FIELD_TYPE";
const char * const tpl_SEL_IN_FIELD_NAME = "SEL_IN_FIELD_NAME";
const char * const tpl_SEL_IN_FIELD_COMMA = "SEL_IN_FIELD_COMMA";
const char * const tpl_SEL_IN_FIELD_INIT = "SEL_IN_FIELD_INIT";
const char * const tpl_SEL_IN_FIELD_BIND = "SEL_IN_FIELD_BIND";
const char * const tpl_SEL_IN_FIELDS_BUFFERS = "SEL_IN_FIELDS_BUFFERS";

const char * const tpl_BUFFER_DECLARE = "BUFFER_DECLARE";
const char * const tpl_BUFFER_ALLOC = "BUFFER_ALLOC";
const char * const tpl_BUFFER_FREE = "BUFFER_FREE";
const char * const tpl_BUFFER_INITIALIZE = "BUFFER_INITIALIZE";

const char * const tpl_SEL_OUT_FIELDS = "SEL_OUT_FIELDS";
const char * const tpl_SEL_OUT_FIELD_TYPE = "SEL_OUT_FIELD_TYPE";
const char * const tpl_SEL_OUT_FIELD_NAME = "SEL_OUT_FIELD_NAME";
const char * const tpl_SEL_OUT_FIELD_COMMA = "SEL_OUT_FIELD_COMMA";
const char * const tpl_SEL_OUT_FIELD_INIT = "SEL_OUT_FIELD_INIT";
const char * const tpl_SEL_OUT_FIELD_GETVALUE = "SEL_OUT_FIELD_GETVALUE";
const char * const tpl_SEL_OUT_FIELD_ISNULL = "SEL_OUT_FIELD_ISNULL";
const char * const tpl_SEL_OUT_FIELDS_BUFFERS = "SEL_OUT_FIELDS_BUFFERS";
const char * const tpl_SEL_OUT_FIELD_COMMENT = "SEL_OUT_FIELD_COMMENT";
const char * const tpl_SEL_OUT_KEY_FIELD_NAME = "SEL_OUT_KEY_FIELD_NAME";
const char * const tpl_SEL_OUT_KEY_FIELD_TYPE = "SEL_OUT_KEY_FIELD_TYPE";

const char * const tpl_DBENGINE_STATEMENT_TYPE = "DBENGINE_STATEMENT_TYPE";
const char * const tpl_DBENGINE_STATEMENT_NULL = "DBENGINE_STATEMENT_NULL";

const char * const tpl_DBENGINE_TRANSACTION = "DBENGINE_TRANSACTION";
const char * const tpl_DBENGINE_TRANSACTION_TYPE = "DBENGINE_TRANSACTION_TYPE";
const char * const tpl_DBENGINE_TRANSACTION_NULL = "DBENGINE_TRANSACTION_NULL";
const char * const tpl_DBENGINE_TRANSACTION_INIT = "DBENGINE_TRANSACTION_INIT";
const char * const tpl_DBENGINE_TRANSACTION_ROLLBACK = "DBENGINE_TRANSACTION_ROLLBACK";
const char * const tpl_DBENGINE_TRANSACTION_COMMIT = "DBENGINE_TRANSACTION_COMMIT";

const char * const tpl_UPDATE = "UPDATE";
const char * const tpl_UPDATE_SQL = "UPDATE_SQL";
const char * const tpl_UPDATE_SQL_UNESCAPED = "UPDATE_SQL_UNESCAPED";
const char * const tpl_UPDATE_SQL_LEN = "UPDATE_SQL_LEN";
const char * const tpl_UPDATE_FIELD_COUNT = "UPDATE_FIELD_COUNT";
const char * const tpl_UPDATE_PARAM_COUNT = "UPDATE_PARAM_COUNT";
const char * const tpl_UPD_IN_FIELDS = "UPD_IN_FIELDS";
const char * const tpl_UPD_IN_FIELD_TYPE = "UPD_IN_FIELD_TYPE";
const char * const tpl_UPD_IN_FIELD_NAME = "UPD_IN_FIELD_NAME";
const char * const tpl_UPD_IN_FIELD_COMMA = "UPD_IN_FIELD_COMMA";
const char * const tpl_UPD_IN_FIELD_INIT = "UPD_IN_FIELD_INIT";
const char * const tpl_UPD_IN_FIELD_BIND = "UPD_IN_FIELD_BIND";
const char * const tpl_UPD_IN_FIELDS_BUFFERS = "UPD_IN_FIELDS_BUFFERS";

const char * const tpl_INSERT = "INSERT";
const char * const tpl_INSERT_SQL = "INSERT_SQL";
const char * const tpl_INSERT_SQL_UNESCAPED = "INSERT_SQL_UNESCAPED";
const char * const tpl_INSERT_SQL_LEN = "INSERT_SQL_LEN";
const char * const tpl_INSERT_FIELD_COUNT = "INSERT_FIELD_COUNT";
const char * const tpl_INSERT_PARAM_COUNT = "INSERT_PARAM_COUNT";
const char * const tpl_INS_IN_FIELDS = "INS_IN_FIELDS";
const char * const tpl_INS_IN_FIELD_TYPE = "INS_IN_FIELD_TYPE";
const char * const tpl_INS_IN_FIELD_NAME = "INS_IN_FIELD_NAME";
const char * const tpl_INS_IN_FIELD_COMMA = "INS_IN_FIELD_COMMA";
const char * const tpl_INS_IN_FIELD_INIT = "INS_IN_FIELD_INIT";
const char * const tpl_INS_IN_FIELD_BIND = "INS_IN_FIELD_BIND";
const char * const tpl_INS_IN_FIELDS_BUFFERS = "INS_IN_FIELDS_BUFFERS";

const char * const tpl_DELETE = "DELETE";
const char * const tpl_DELETE_SQL = "DELETE_SQL";
const char * const tpl_DELETE_SQL_UNESCAPED = "DELETE_SQL_UNESCAPED";
const char * const tpl_DELETE_SQL_LEN = "DELETE_SQL_LEN";
const char * const tpl_DELETE_FIELD_COUNT = "DELETE_FIELD_COUNT";
const char * const tpl_DELETE_PARAM_COUNT = "DELETE_PARAM_COUNT";
const char * const tpl_DEL_IN_FIELDS = "DEL_IN_FIELDS";
const char * const tpl_DEL_IN_FIELD_TYPE = "DEL_IN_FIELD_TYPE";
const char * const tpl_DEL_IN_FIELD_NAME = "DEL_IN_FIELD_NAME";
const char * const tpl_DEL_IN_FIELD_COMMA = "DEL_IN_FIELD_COMMA";
const char * const tpl_DEL_IN_FIELD_INIT = "DEL_IN_FIELD_INIT";
const char * const tpl_DEL_IN_FIELD_BIND = "DEL_IN_FIELD_BIND";
const char * const tpl_DEL_IN_FIELDS_BUFFERS = "DEL_IN_FIELDS_BUFFERS";

const char * const tpl_DBENGINE_CONNECT_PARAMS = "DBENGINE_CONNECT_PARAMS";
const char * const tpl_DBENGINE_CONNECT_PARAM_TYPE = "DBENGINE_CONNECT_PARAM_TYPE";
const char * const tpl_DBENGINE_CONNECT_PARAM_PARAM = "DBENGINE_CONNECT_PARAM_PARAM";
const char * const tpl_DBENGINE_CONNECT_PARAM_VALUE = "DBENGINE_CONNECT_PARAM_VALUE";
const char * const tpl_DBENGINE_CONNECT_PARAM_COMMA = "DBENGINE_CONNECT_PARAM_COMMA";

const char * const tpl_DBENGINE_CONNECT = "DBENGINE_CONNECT";
const char * const tpl_DBENGINE_PREPARE = "DBENGINE_PREPARE";
const char * const tpl_DBENGINE_EXTRAS = "DBENGINE_EXTRAS";
const char * const tpl_DBENGINE_EXTRA_VAR = "DBENGINE_EXTRA_VAR";
const char * const tpl_DBENGINE_DISCONNECT = "DBENGINE_DISCONNECT";

const char * const tpl_DBENGINE_CREATE_SELECT = "DBENGINE_CREATE_SELECT";
const char * const tpl_DBENGINE_PREPARE_SELECT = "DBENGINE_PREPARE_SELECT";
const char * const tpl_DBENGINE_DESTROY_SELECT = "DBENGINE_DESTROY_SELECT";
const char * const tpl_DBENGINE_RESET_SELECT = "DBENGINE_RESET_SELECT";
const char * const tpl_DBENGINE_EXECUTE_SELECT = "DBENGINE_EXECUTE_SELECT";
const char * const tpl_DBENGINE_FETCH_SELECT = "DBENGINE_FETCH_SELECT";

const char * const tpl_DBENGINE_CREATE_UPDATE = "DBENGINE_CREATE_UPDATE";
const char * const tpl_DBENGINE_PREPARE_UPDATE = "DBENGINE_PREPARE_UPDATE";
const char * const tpl_DBENGINE_DESTROY_UPDATE = "DBENGINE_DESTROY_UPDATE";
const char * const tpl_DBENGINE_RESET_UPDATE = "DBENGINE_RESET_UPDATE";
const char * const tpl_DBENGINE_EXECUTE_UPDATE = "DBENGINE_EXECUTE_UPDATE";

const char * const tpl_DBENGINE_CREATE_INSERT = "DBENGINE_CREATE_INSERT";
const char * const tpl_DBENGINE_PREPARE_INSERT = "DBENGINE_PREPARE_INSERT";
const char * const tpl_DBENGINE_DESTROY_INSERT = "DBENGINE_DESTROY_INSERT";
const char * const tpl_DBENGINE_RESET_INSERT = "DBENGINE_RESET_INSERT";
const char * const tpl_DBENGINE_EXECUTE_INSERT = "DBENGINE_EXECUTE_INSERT";

const char * const tpl_DBENGINE_CREATE_DELETE = "DBENGINE_CREATE_DELETE";
const char * const tpl_DBENGINE_PREPARE_DELETE = "DBENGINE_PREPARE_DELETE";
const char * const tpl_DBENGINE_DESTROY_DELETE = "DBENGINE_DESTROY_DELETE";
const char * const tpl_DBENGINE_RESET_DELETE = "DBENGINE_RESET_DELETE";
const char * const tpl_DBENGINE_EXECUTE_DELETE = "DBENGINE_EXECUTE_DELETE";

const char * const tpl_SPROC = "SPROC";
const char * const tpl_SPROC_SQL = "SPROC_SQL";
const char * const tpl_SPROC_SQL_UNESCAPED = "SPROC_SQL_UNESCAPED";
const char * const tpl_SPROC_SQL_LEN = "SPROC_SQL_LEN";
const char * const tpl_SPROC_FIELD_COUNT = "SPROC_FIELD_COUNT";
const char * const tpl_SPROC_PARAM_COUNT = "SPROC_PARAM_COUNT";

const char * const tpl_SPROC_HAS_PARAMS = "SPROC_HAS_PARAMS";

const char * const tpl_SP_IN_FIELDS = "SP_IN_FIELDS";
const char * const tpl_SP_IN_FIELD_TYPE = "SP_IN_FIELD_TYPE";
const char * const tpl_SP_IN_FIELD_NAME = "SP_IN_FIELD_NAME";
const char * const tpl_SP_IN_FIELD_COMMA = "SP_IN_FIELD_COMMA";
const char * const tpl_SP_IN_FIELD_INIT = "SP_IN_FIELD_INIT";
const char * const tpl_SP_IN_FIELD_BIND = "SP_IN_FIELD_BIND";
const char * const tpl_SP_IN_FIELDS_BUFFERS = "SP_IN_FIELDS_BUFFERS";

const char * const tpl_SP_OUT_FIELDS = "SP_OUT_FIELDS";
const char * const tpl_SP_OUT_FIELD_TYPE = "SP_OUT_FIELD_TYPE";
const char * const tpl_SP_OUT_FIELD_NAME = "SP_OUT_FIELD_NAME";
const char * const tpl_SP_OUT_FIELD_COMMA = "SP_OUT_FIELD_COMMA";
const char * const tpl_SP_OUT_FIELD_INIT = "SP_OUT_FIELD_INIT";
const char * const tpl_SP_OUT_FIELD_GETVALUE = "SP_OUT_FIELD_GETVALUE";
const char * const tpl_SP_OUT_FIELD_ISNULL = "SP_OUT_FIELD_ISNULL";
const char * const tpl_SP_OUT_FIELDS_BUFFERS = "SP_OUT_FIELDS_BUFFERS";
const char * const tpl_SP_OUT_FIELD_COMMENT = "SP_OUT_FIELD_COMMENT";

SQLTypes typeNameToSQLType(std::string _name)
{
    if ( _name.length() )
    {
        _name = stringToLower( _name );

        switch( _name[0] )
        {
            case 'b':
            {
                if ( _name == "blob" )
                    return stBlob;

                break;
            }
            case 'd':
            {
                if ( _name == "double" )
                    return stDouble;
                else if ( _name == "date" )
                    return stDate;
                else if ( _name == "datetime" )
                    return stTimeStamp;

                break;
            }
            case 'f':
            {
                if ( _name == "float" )
                    return stFloat;

                break;
            }
            case 'i':
            {
                if ( _name == "int" || _name == "integer" )
                    return stInt;

                break;
            }
            case 's':
            {
                if ( _name == "str" || _name == "string" )
                    return stText;

                break;
            }
            case 't':
            {
                if ( _name == "text" )
                    return stText;
                else if ( _name == "timestamp" )
                    return stTimeStamp;
                else if ( _name == "time" )
                    return stTime;

                break;
            }
            default:
            {
                WARNING("unknown param type: " << _name);
                return stUnknown;
            }
        }
    }

    WARNING("unknown param type: " << _name);
    return stUnknown;
}

std::string sqlTypeToName(SQLTypes _type)
{
#define __typeToString(TYPE) case TYPE: return # TYPE ;
    switch(_type)
    {
        __typeToString(stUnknown);       
        __typeToString(stInt);
        __typeToString(stInt64);
        __typeToString(stUInt);
        __typeToString(stUInt64);
        __typeToString(stFloat);
        __typeToString(stDouble);
        __typeToString(stUFloat);
        __typeToString(stUDouble);
        __typeToString(stTimeStamp);
        __typeToString(stTime);
        __typeToString(stDate);
        __typeToString(stText);
        __typeToString(stBlob);
    }
#undef __typeToString

    WARNING("unknown param type: " << _type);
    return std::string("stUnknown");
}

AbstractGenerator* AbstractGenerator::s_generator = 0;
AbstractGenerator* AbstractGenerator::getGenerator(const std::string & _type)
{
    if ( s_generator )
        return s_generator;

    if ( _type.length() > 1 )
    {
        std::string type = stringToLower( _type );
        switch ( type[0] )
        {
#ifdef WITH_FIREBIRD
            case 'f':
            case 'i':
            {
                if ( type == "firebird" || type == "interbase" )
                {
                    s_generator = new FirebirdGenerator();
                    return s_generator;
                }
            }
#endif // WITH_FIREBRID
#ifdef WITH_MYSQL
            case 'm':
            {
                if ( type == "mysql" )
                {
                    s_generator = new MySQLGenerator();
                    return s_generator;
                }
            }
#endif // WITH_MYSQL
#ifdef WITH_ORACLE
            case 'o':
            {
                if ( type == "oracle" )
                {
                    s_generator = new OracleGenerator();
                    return s_generator;
                }
            }
#endif // WITH_ORACLE
#ifdef WITH_POSTGRESQL
            case 'p':
            {
                if ( type == "postgres" || type == "postgresql" )
                {
                    s_generator = new PostgreSQLGenerator();
                    return s_generator;
                }
            }
#endif // WITH_POSTGRESQL
            case 's':
            {
                if ( type == "sqlite" || type == "sqlite3" )
                {
                    s_generator = new SQLiteGenerator();
                    return s_generator;
                }
            }
        }
    }

    FATAL("AbstractGenerator: invalid generator type " << _type);
}

AbstractGenerator::AbstractGenerator():
        m_connected(false),
        m_dict(0)
{
}

AbstractGenerator::~AbstractGenerator()
{
}

bool AbstractGenerator::checkConnection()
{
    return m_connected;
}

std::string AbstractGenerator::getType(SQLTypes _sqlType)
{
    std::string result = m_types[_sqlType];

    if ( result.empty() )
    {
        // TODO Abstract this
        switch ( _sqlType )
        {
            case stUnknown:
            {
                FATAL("BUG BUG BUG! " << __FILE__ << __LINE__);
            }
            case stInt:
            {
                result = "int";
                break;
            }
            case stUInt:
            {
                result = "unsigned int";
                break;
            }
            case stInt64:
            {
                result = "long long int";
                break;
            }
            case stUInt64:
            {
                result = "unsigned long long int";
                break;
            }
            case stFloat:
            {
                result = "float";
                break;
            }
            case stUFloat:
            {
                result = "unsigned float";
                break;
            }
            case stDouble:
            {
                result = "double";
                break;
            }
            case stUDouble:
            {
                result = "unsigned double";
                break;
            }
            case stTimeStamp:
            case stTime:
            case stDate:
            {
                result = "ptime";
                break;
            }
            case stText:
            {
                result = "const char*";
                break;
            }
            case stBlob:
            {
                result = "shared_pointer< std::vector<char> >::type";
                break;
            }
        }
    }

    return result;
}

std::string AbstractGenerator::getInit(SQLTypes _sqlType)
{
    std::string result;

    // TODO Abstract this
    switch ( _sqlType )
    {
        case stUnknown:
        {
            FATAL("BUG BUG BUG! " << __FILE__ << __LINE__);
        }
        case stInt:
        case stUInt:
        case stInt64:
        case stUInt64:
        case stFloat:
        case stUFloat:
        case stDouble:
        case stUDouble:
        {
            result = "0";
            break;
        }
        case stDate:
        case stTime:
        case stTimeStamp:
        {
            result = "ptime()";
            break;
        }
        case stText:
        {
            result = "NULL";
            break;
        }
        case stBlob:
        {
            // Nothing!  Use default initializer
            break;
        }
    }

    return result;
}

void AbstractGenerator::addSelect(SelectElements _elements)
{
    std::string name = stringToLower( _elements.name );

    if (!_elements.keyFieldName.empty())
    {
        std::string key = _elements.keyFieldName;

        // Check whether the key field's index was passed
        char *endptr = NULL;
        int index = strtol(key.c_str(), &endptr, 10);

        if (endptr && (*endptr == '\0' || isblank(*endptr) || *endptr == '\n'))
        {
            _elements.keyField = index - 1;
        }
        else
        {
            // Assunme, that the key field's name was passed
            ListElements::iterator it = _elements.output.begin(), end = _elements.output.end();
            for(; it != end; it++)
            {
                if (strcasecmp(it->name.c_str(), key.c_str()) == 0)
                {
                    _elements.keyField = it->index;
                    break;
                }
            }
        }
    }

    _classParamsPtr params = m_classParams[ name ];

    if ( !params )
        params.reset( new _classParams );

    params->select = _elements;
    m_classParams[ name ] = params;
}

void AbstractGenerator::addUpdate(UpdateElements _elements)
{
    std::string name = stringToLower( _elements.name );

    _classParamsPtr params = m_classParams[ name ];

    if ( !params )
        params.reset( new _classParams );

    params->update = _elements;
    m_classParams[ name ] = params;
}

void AbstractGenerator::addInsert(InsertElements _elements)
{
    std::string name = stringToLower( _elements.name );

    _classParamsPtr params = m_classParams[ name ];

    if ( !params )
        params.reset( new _classParams );

    params->insert = _elements;
    m_classParams[ name ] = params;
}

void AbstractGenerator::addDelete(DeleteElements _elements)
{
    std::string name = stringToLower( _elements.name );

    _classParamsPtr params = m_classParams[ name ];

    if ( !params )
        params.reset( new _classParams );

    params->del = _elements;
    m_classParams[ name ] = params;
}

void AbstractGenerator::addStoredProcedure(StoredProcedureElements _elements)
{
    std::string name = stringToLower( _elements.name );

    _classParamsPtr params = m_classParams[ name ];

    if ( !params )
        params.reset( new _classParams );

    params->stoProc = _elements;
    m_classParams[ name ] = params;
}

static void cleanExcessiveLineBreaks(std::string &_str, std::ostream &_stream)
{
    int ln = 0;
    foreach(char c, _str)
    {
        if ( c == '\n' )
            ++ln;
        else
        {
            if ( !isspace(c) )
                ln = 0;
        }

        if ( ln < 3 )
            _stream << c;
    }
}

void AbstractGenerator::generate()
{
    // Defaults values - might be replaced by 'loadTemplates'
    m_outIntFile = optOutput + ".h";
    m_outImplFile = optOutput + ".cpp";

    loadTemplates();
    loadDictionary();
    loadDatabase();

    if ( DBBinder::optListDepends )
        return;

    std::string str;
    {
        std::ofstream out( m_outIntFile.c_str(), std::ios_base::trunc );
        if ( !ctemplate::ExpandTemplate(m_templ[ftIntf], DO_NOT_STRIP, m_dict, &str) )
            FATAL("Error processing: " << m_templ[ftIntf]);

        cleanExcessiveLineBreaks(str, out);
        str.clear();
    }

    // TODO Put this in an XML
    str = "astyle --style=ansi -n > /dev/null 2>&1 ";
    str += m_outIntFile;

    if ( system( str.c_str() ) == -1 )
        std::cerr << "warning: unable to run astyle" << std::endl;

    str.clear();

    {
        std::ofstream out( m_outImplFile.c_str(), std::ios_base::trunc );
        if ( !ctemplate::ExpandTemplate(m_templ[ftImpl], DO_NOT_STRIP, m_dict, &str) )
            FATAL("Error processing: " << m_templ[ftImpl]);

        cleanExcessiveLineBreaks(str, out);
        str.clear();
    }

    // TODO Put this in an XML
    str = "astyle --style=ansi -n > /dev/null 2>&1 ";
    str += m_outImplFile;

    if ( system( str.c_str() ) == -1 )
        std::cerr << "warning: unable to run astyle" << std::endl;

    if ( DBBinder::optExtras )
        for( ListTplDestPair::const_iterator it = m_extraFiles.begin();
                it != m_extraFiles.end(); ++it )
        {
            str.clear();

            m_dict->SetValue(tpl_FILENAME, it->dest);

            Template *tmpl = Template::GetTemplate(it->tmpl, DO_NOT_STRIP);

            std::ofstream out( it->dest.c_str(), std::ios_base::trunc );
            tmpl->Expand(&str, m_dict);

            cleanExcessiveLineBreaks(str, out);
            str.clear();

            // TODO Put this in an XML
            str = "astyle --style=ansi -n > /dev/null 2>&1 ";
            str += it->dest;

            if ( system( str.c_str() ) == -1 )
                std::cerr << "warning: unable to run astyle" << std::endl;
        }

    str.clear();
}

void AbstractGenerator::loadDatabase()
{
    TemplateDictionary *subDict = 0;
    _dbParams::iterator it;
    for(it = m_dbParams.begin(); it != m_dbParams.end(); ++it)
    {
        subDict = m_dict->AddSectionDictionary(tpl_DBENGINE_CONNECT_PARAMS);

        subDict->SetValue(tpl_DBENGINE_CONNECT_PARAM_PARAM, it->first);

        if ( it->second.isInt )
        {
            subDict->SetValue(tpl_DBENGINE_CONNECT_PARAM_VALUE, it->second.value);
            subDict->SetValue(tpl_DBENGINE_CONNECT_PARAM_TYPE, "const int");
        }
        else
        {
            subDict->SetValue(tpl_DBENGINE_CONNECT_PARAM_VALUE, std::string("\"") + cescape(it->second.value) + std::string("\""));
            subDict->SetValue(tpl_DBENGINE_CONNECT_PARAM_TYPE, "const char * const");
        }
        subDict->SetValue( tpl_DBENGINE_CONNECT_PARAM_COMMA, "," );
    }
    if ( subDict )
        subDict->SetValue( tpl_DBENGINE_CONNECT_PARAM_COMMA, "" );

    std::string str;
    ListString::iterator dirs;
    for(dirs = optTemplateDirs.begin(); dirs != optTemplateDirs.end(); ++dirs)
    {
        str = (*dirs) + "/dbengines/";

        struct stat fs;

        if ( stat((str + m_dbengine + ".xml").c_str(), &fs) == 0 ) // Check if file exists
        {
            if ( loadXMLDatabase( str ) )
                goto LOADED;
        }
        else if ( stat((str + m_dbengine + ".yaml").c_str(), &fs) == 0 ) // Check if file exists
        {
            if ( loadYAMLDatabase( str ) )
                goto LOADED;
        }
    }

    FATAL("dbengine: template file not found for current language or database.");

LOADED:
    {}
}

bool AbstractGenerator::loadXMLDatabase(const std::string& _path)
{
#define GET_TEXT_OR_ATTR_SET_TMPL( str, elem, attr, tmpl, var ) \
    GET_TEXT_OR_ATTR( str, elem, attr ); \
    if ( !str.empty() ) { result = true; tmpl->SetValue(var, parseStringVariables(str)); }

    ListString templates = stringTok(optTemplate, ',');
    ListString::iterator templ;
    TemplateDictionary *subDict;

    bool result = false;

    std::string xmlFileName = _path + m_dbengine + ".xml";
    try
    {
        XMLDocument xmlFile( xmlFileName );
        xmlFile.LoadFile();

        DBBinder::optDepends.push_back( xmlFileName );

        XMLElementPtr xml = xmlFile.FirstChildElement("xml");

        XMLElementPtr lang;
        XMLNodePtr node = 0;
        while( node = xml->IterateChildren( "lang", node ))
        {
            lang = node->ToElement();

            for(templ = templates.begin(); templ != templates.end(); ++templ)
            {
                if ( lang->GetAttributeOrDefault("type", "-- INVALID LANGUAGE --") == *templ )
                {
                    XMLElementPtr elem;
                    XMLNodePtr node, subnode;
                    std::string str;

                    node = 0;
                    while( node = lang->IterateChildren( "includes", node ))
                    {
                        m_dict->ShowSection( tpl_DBENGINE_INCLUDES );

                        subnode = 0;
                        while( subnode = node->IterateChildren( "file", subnode ))
                        {
                            subDict = m_dict->AddSectionDictionary( tpl_DBENGINE_INCLUDES );
                            elem = subnode->ToElement();

                            GET_TEXT_OR_ATTR( str, elem, "name" );
                            if ( !str.empty() )
                            {
                                result = true;
                                str = std::string("#include <") + parseStringVariables(str) + ">";
                                subDict->SetValue(tpl_DBENGINE_INCLUDE_NAME, str);
                            }
                        }
                    }

                    node = 0;
                    while( node = lang->IterateChildren( "global_functions", node ))
                    {
                        m_dict->ShowSection( tpl_EXTRA_HEADERS );

                        subnode = 0;
                        while( subnode = node->IterateChildren( "function", subnode ))
                        {
                            subDict = m_dict->AddSectionDictionary( tpl_DBENGINE_GLOBAL_FUNCTIONS );
                            elem = subnode->ToElement();
                            GET_TEXT_OR_ATTR_SET_TMPL( str, elem, "code", subDict, tpl_FUNCTION);
                        }
                    }

                    node = 0;
                    while( node = lang->IterateChildren( "extra_headers", node ))
                    {
                        m_dict->ShowSection( tpl_EXTRA_HEADERS );


                        subnode = 0;
                        while( subnode = node->IterateChildren( "define", subnode ))
                        {
                            subDict = m_dict->AddSectionDictionary( tpl_EXTRA_HEADERS );
                            elem = subnode->ToElement();
                            GET_TEXT_OR_ATTR_SET_TMPL( str, elem, "code", subDict, tpl_EXTRA_HEADERS_HEADER);
                        }

                        subnode = 0;
                        while( subnode = node->IterateChildren( "type", subnode ))
                        {
                            subDict = m_dict->AddSectionDictionary( tpl_EXTRA_HEADERS );
                            elem = subnode->ToElement();
                            GET_TEXT_OR_ATTR_SET_TMPL( str, elem, "code", subDict, tpl_EXTRA_HEADERS_TYPE);
                        }

                        subnode = 0;
                        while( subnode = node->IterateChildren( "member", subnode ))
                        {
                            subDict = m_dict->AddSectionDictionary( tpl_EXTRA_HEADERS );
                            elem = subnode->ToElement();
                            GET_TEXT_OR_ATTR_SET_TMPL( str, elem, "code", subDict, tpl_EXTRA_HEADERS_MEMBER);
                        }
                    }

                    node = 0;
                    while( node = lang->IterateChildren( "types", node ))
                    {
                        subnode = 0;
                        while( subnode = node->IterateChildren( subnode ))
                        {
                            if ( subnode->Value() == "connection" )
                            {
                                GET_TEXT_OR_ATTR_SET_TMPL( str, subnode->FirstChildElement("type"), "value", m_dict, tpl_DBENGINE_CONNECTION_TYPE);
                                GET_TEXT_OR_ATTR_SET_TMPL( str, subnode->FirstChildElement("null"), "value", m_dict, tpl_DBENGINE_CONNECTION_NULL);
                            }
                            else if ( subnode->Value() == "statement" )
                            {
                                GET_TEXT_OR_ATTR_SET_TMPL( str, subnode->FirstChildElement("type"), "value", m_dict, tpl_DBENGINE_STATEMENT_TYPE);
                                GET_TEXT_OR_ATTR_SET_TMPL( str, subnode->FirstChildElement("null"), "value", m_dict, tpl_DBENGINE_STATEMENT_NULL);
                            }
                            else if ( subnode->Value() == "transaction" )
                            {
                                m_dict->ShowSection( tpl_DBENGINE_TRANSACTION );
                                GET_TEXT_OR_ATTR_SET_TMPL( str, subnode->FirstChildElement("type"), "value", m_dict, tpl_DBENGINE_TRANSACTION_TYPE);
                                GET_TEXT_OR_ATTR_SET_TMPL( str, subnode->FirstChildElement("null"), "value", m_dict, tpl_DBENGINE_TRANSACTION_NULL);
                                GET_TEXT_OR_ATTR_SET_TMPL( str, subnode->FirstChildElement("init"), "value", m_dict, tpl_DBENGINE_TRANSACTION_INIT);
                                GET_TEXT_OR_ATTR_SET_TMPL( str, subnode->FirstChildElement("rollback"), "value", m_dict, tpl_DBENGINE_TRANSACTION_ROLLBACK);
                                GET_TEXT_OR_ATTR_SET_TMPL( str, subnode->FirstChildElement("commit"), "value", m_dict, tpl_DBENGINE_TRANSACTION_COMMIT);
                            }
                            else
                            {
                                elem = subnode->ToElement();
                                m_dict->ShowSection( tpl_DBENGINE_EXTRAS );
                                subDict = m_dict->AddSectionDictionary( tpl_DBENGINE_EXTRAS );
                                subDict->SetValue( tpl_DBENGINE_EXTRA_VAR,
                                                    parseStringVariables(elem->FirstChildElement("type")->GetAttributeOrDefault("value", "")) + " " +
                                                    parseStringVariables(elem->FirstChildElement("name")->GetAttributeOrDefault("value", "")) + ";"
                                                    );
                            }
                        }
                    }

                    node = 0;
                    while( node = lang->IterateChildren( "connect", node ))
                    {
                        m_dict->SetValue(tpl_DBENGINE_CONNECT, parseStringVariables(node->ToElement()->GetText()));
                    }

                    node = 0;
                    while( node = lang->IterateChildren( "disconnect", node ))
                    {
                        m_dict->SetValue(tpl_DBENGINE_DISCONNECT, parseStringVariables(node->ToElement()->GetText()));
                    }

                    node = 0;
                    while( node = lang->IterateChildren( "select", node ))
                    {
                        elem = node->FirstChildElement("create", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_CREATE_SELECT, parseStringVariables(elem->GetText(false)));

                        elem = node->FirstChildElement("destroy", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_DESTROY_SELECT, parseStringVariables(elem->GetText(false)));

                        elem = node->FirstChildElement("prepare", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_PREPARE_SELECT, parseStringVariables(elem->GetText(false)));

                        elem = node->FirstChildElement("execute", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_EXECUTE_SELECT, parseStringVariables(elem->GetText(false)));

                        elem = node->FirstChildElement("fetch", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_FETCH_SELECT, parseStringVariables(elem->GetText(false)));

                        elem = node->FirstChildElement("reset", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_RESET_SELECT, parseStringVariables(elem->GetText(false)));
                    }

                    node = 0;
                    while( node = lang->IterateChildren( "update", node ))
                    {
                        elem = node->FirstChildElement("create", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_CREATE_UPDATE, parseStringVariables(elem->GetText(false)));

                        elem = node->FirstChildElement("destroy", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_DESTROY_UPDATE, parseStringVariables(elem->GetText(false)));

                        elem = node->FirstChildElement("prepare", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_PREPARE_UPDATE, parseStringVariables(elem->GetText(false)));

                        elem = node->FirstChildElement("execute", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_EXECUTE_UPDATE, parseStringVariables(elem->GetText(false)));

                        elem = node->FirstChildElement("reset", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_RESET_UPDATE, parseStringVariables(elem->GetText(false)));
                    }

                    node = 0;
                    while( node = lang->IterateChildren( "insert", node ))
                    {
                        elem = node->FirstChildElement("create", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_CREATE_INSERT, parseStringVariables(elem->GetText(false)));

                        elem = node->FirstChildElement("destroy", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_DESTROY_INSERT, parseStringVariables(elem->GetText(false)));

                        elem = node->FirstChildElement("prepare", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_PREPARE_INSERT, parseStringVariables(elem->GetText(false)));

                        elem = node->FirstChildElement("execute", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_EXECUTE_INSERT, parseStringVariables(elem->GetText(false)));

                        elem = node->FirstChildElement("reset", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_RESET_INSERT, parseStringVariables(elem->GetText(false)));
                    }

                    node = 0;
                    while( node = lang->IterateChildren( "delete", node ))
                    {
                        elem = node->FirstChildElement("create", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_CREATE_DELETE, parseStringVariables(elem->GetText(false)));

                        elem = node->FirstChildElement("destroy", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_DESTROY_DELETE, parseStringVariables(elem->GetText(false)));

                        elem = node->FirstChildElement("prepare", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_PREPARE_DELETE, parseStringVariables(elem->GetText(false)));

                        elem = node->FirstChildElement("execute", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_EXECUTE_DELETE, parseStringVariables(elem->GetText(false)));

                        elem = node->FirstChildElement("reset", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_RESET_DELETE, parseStringVariables(elem->GetText(false)));
                    }
                }
            }
        }
    }
    catch( ticpp::Exception &e )
    {
        WARNING("XML: " << xmlFileName << ": " << e.what());
    }
    return result;
}

bool AbstractGenerator::loadYAMLDatabase(const std::string& _path)
{
    FATAL("YAML parser not implemented yet. sorry.");
    DBBinder::optDepends.push_back( _path );
}

void AbstractGenerator::loadTemplates()
{
    std::string str;

    ListString templates = stringTok(optTemplate, ',');
    ListString::iterator templ, dirs;

    int count;
    int i;

    ASSERT(optTemplateDirs.size(), "Empty list of template dirs");

    for(dirs = optTemplateDirs.begin(); dirs != optTemplateDirs.end(); ++dirs)
    {
        count = templates.size();
        do
        {
            str.clear();
            for(templ = templates.begin(), i = 0; templ != templates.end() && i < count; ++templ)
            {
                str += (*templ) + '/';
            }
            str = (*dirs) + "/lang/" + str;

            struct stat fs;

            if ( stat((str + "template.xml").c_str(), &fs) == 0 ) // Check if file exists
            {
                if ( loadXMLTemplate( str ) && m_templ[ftIntf].size() && m_templ[ftImpl].size() )
                    goto LOADED;
            }
            else if ( stat((str + "template.yaml").c_str(), &fs) == 0 ) // Check if file exists
            {
                if ( loadYAMLTemplate( str ) && m_templ[ftIntf].size() && m_templ[ftImpl].size() )
                    goto LOADED;
            }
        }
        while(--count);
    }

LOADED:
    if (!( fileExists(m_templ[ftIntf]) && fileExists(m_templ[ftImpl]) ))
        FATAL("template: template files not found.");
}

std::string AbstractGenerator::parseStringVariables(std::string str)
{
    const char* KEYWORDS[] = { "SELECT_NAME", 0 };
    std::size_t KEYWORDS_LEN[] = { strlen(KEYWORDS[0]), 0 };

    std::string::size_type i, len;

    i = 0;
    len = str.length();

    while(i < len)
    {
        if (str[i] == '%')
        {
            for(int k = 0; KEYWORDS[k]; k++)
            {
                const size_t end = i + KEYWORDS_LEN[k];
                if (str[end + 1] == '%')
                {
                    std::string val;

                    switch(k)
                    {
                        case 0:
                        {
                            for(classParams::iterator it = m_classParams.begin(); it != m_classParams.end(); it++)
                            {
                                if ( !(it->second->select.sql.empty() ))
                                {
                                    val = it->first;
                                    break;
                                }
                            }
                            break;
                        }
                    }

                    // replace n char, n = delta(first %, second %) + (2x'%')
                    str.replace(i, end - i + 2, val);
                    i += val.length();
                    goto FOUND_KEYWORD;
                }
            }
        }

        i++;

        FOUND_KEYWORD: {}
    }

    return str;
}

void AbstractGenerator::readParam(void* xml, const char *xmlElem, _fileTypes fileType, std::string& outFile, std::string& str, const std::string & _path)
{
    XMLElementPtr elem;

    elem = static_cast<XMLElementPtr>(xml)->FirstChildElement(xmlElem);
    if ( elem )
    {
        elem->GetAttribute("file", &str, false);
        if ( str.empty() )
            elem->GetText( &str, false );
        if ( str.length() )
        {
            if ( str[0] != '/' )
                str = _path + '/' + str;
            if ( fileExists(str) )
            {
                m_templ[fileType] = str;
            }
            else
            {
                WARNING("file :" << str << " does not exist!");
            }
        }
        str.clear();
        elem->GetAttribute("extension", &str, false);
        if ( str.empty() )
            elem->GetText( &str, false );
        if ( str.length() )
            outFile = optOutput + str;
    }
}

bool AbstractGenerator::loadXMLTemplate(const std::string & _path)
{
    try
    {
        XMLDocument xmlFile( _path + "template.xml" );
        xmlFile.LoadFile();

        DBBinder::optDepends.push_back( _path + "template.xml" );

        XMLElementPtr xml = xmlFile.FirstChildElement("xml");

        std::string str;
        readParam(xml, "interface", ftIntf, m_outIntFile, str, _path);
        readParam(xml, "implementation", ftImpl, m_outImplFile, str, _path);

        XMLElementPtr elem;
        elem = xml->FirstChildElement("extra");
        if ( elem )
        {
            XMLNodePtr node = 0;
            while( node = elem->IterateChildren( "file", node ))
            {
                TmplDestPair pair;

                node->ToElement()->GetAttribute("file", &pair.tmpl, false);
                node->ToElement()->GetAttribute("dest", &pair.dest, false);

                pair.tmpl = getFilenameRelativeTo(_path + "/.", pair.tmpl);
                m_extraFiles.push_back( pair );
            }
        }

        return true;
    }
    catch( ticpp::Exception &e )
    {
        WARNING("XML: " << _path << ": " << e.what());
    }
    return false;
}

bool AbstractGenerator::loadYAMLTemplate(const std::string & _path)
{
    FATAL("YAML parser not implemented yet. sorry.");
    DBBinder::optDepends.push_back( _path );
}

// Helper just so we don't have to put a * on each invididual case.
typedef const char* CCPtr;

struct TemplateFieldMap
{
    CCPtr
        section,
        sql, sqlLen, sqlUnescaped,
        paramCount, fieldCount,
        hasParams,
        inFields, inFieldsType, inFieldsName, inFieldsComma, inFieldsInit, inFieldsBind,
        outFields, outFieldsType, outFieldsName, outFieldsComma, outFieldsInit, outFieldsGetValue, outFieldsIsNull, outFieldsComment,
        outFieldsKeyName, outFieldsKeyType
    ;
};

static CCPtr
    tpl_INSERT_HAS_PARAMS = NULL,
    tpl_INS_OUT_FIELDS = NULL, tpl_INS_OUT_FIELD_TYPE = NULL, tpl_INS_OUT_FIELD_NAME = NULL, tpl_INS_OUT_FIELD_COMMA = NULL, tpl_INS_OUT_FIELD_INIT = NULL, tpl_INS_OUT_FIELD_GETVALUE = NULL, tpl_INS_OUT_FIELD_ISNULL = NULL, tpl_INS_OUT_FIELD_COMMENT = NULL,
    tpl_INS_OUT_KEY_FIELD_NAME = NULL, tpl_INS_OUT_KEY_FIELD_TYPE = NULL,
    tpl_UPDATE_HAS_PARAMS = NULL,
    tpl_UPD_OUT_FIELDS = NULL, tpl_UPD_OUT_FIELD_TYPE = NULL, tpl_UPD_OUT_FIELD_NAME = NULL, tpl_UPD_OUT_FIELD_COMMA = NULL, tpl_UPD_OUT_FIELD_INIT = NULL, tpl_UPD_OUT_FIELD_GETVALUE = NULL, tpl_UPD_OUT_FIELD_ISNULL = NULL, tpl_UPD_OUT_FIELD_COMMENT = NULL,
    tpl_UPD_OUT_KEY_FIELD_NAME = NULL, tpl_UPD_OUT_KEY_FIELD_TYPE = NULL,
    tpl_DELETE_HAS_PARAMS = NULL,
    tpl_DEL_OUT_FIELDS = NULL, tpl_DEL_OUT_FIELD_TYPE = NULL, tpl_DEL_OUT_FIELD_NAME = NULL, tpl_DEL_OUT_FIELD_COMMA = NULL, tpl_DEL_OUT_FIELD_INIT = NULL, tpl_DEL_OUT_FIELD_GETVALUE = NULL, tpl_DEL_OUT_FIELD_ISNULL = NULL, tpl_DEL_OUT_FIELD_COMMENT = NULL,
    tpl_DEL_OUT_KEY_FIELD_NAME = NULL, tpl_DEL_OUT_KEY_FIELD_TYPE = NULL,
    tpl_SP_OUT_KEY_FIELD_NAME = NULL, tpl_SP_OUT_KEY_FIELD_TYPE = NULL;

static TemplateFieldMap 
    selectFieldMap = 
    {
        tpl_SELECT, 
        tpl_SELECT_SQL, tpl_SELECT_SQL_LEN, tpl_SELECT_SQL_UNESCAPED,
        tpl_SELECT_PARAM_COUNT, tpl_SELECT_FIELD_COUNT,
        tpl_SELECT_HAS_PARAMS,
        tpl_SEL_IN_FIELDS, tpl_SEL_IN_FIELD_TYPE, tpl_SEL_IN_FIELD_NAME, tpl_SEL_IN_FIELD_COMMA, tpl_SEL_IN_FIELD_INIT, tpl_SEL_IN_FIELD_BIND,
        tpl_SEL_OUT_FIELDS, tpl_SEL_OUT_FIELD_TYPE, tpl_SEL_OUT_FIELD_NAME, tpl_SEL_OUT_FIELD_COMMA, tpl_SEL_OUT_FIELD_INIT, tpl_SEL_OUT_FIELD_GETVALUE, tpl_SEL_OUT_FIELD_ISNULL, tpl_SEL_OUT_FIELD_COMMENT,
        tpl_SEL_OUT_KEY_FIELD_NAME, tpl_SEL_OUT_KEY_FIELD_TYPE
    },
    insertFieldMap = 
    {
        tpl_INSERT, 
        tpl_INSERT_SQL, tpl_INSERT_SQL_LEN, tpl_INSERT_SQL_UNESCAPED,
        tpl_INSERT_PARAM_COUNT, tpl_INSERT_FIELD_COUNT,
        tpl_INSERT_HAS_PARAMS,
        tpl_INS_IN_FIELDS, tpl_INS_IN_FIELD_TYPE, tpl_INS_IN_FIELD_NAME, tpl_INS_IN_FIELD_COMMA, tpl_INS_IN_FIELD_INIT, tpl_INS_IN_FIELD_BIND,
        tpl_INS_OUT_FIELDS, tpl_INS_OUT_FIELD_TYPE, tpl_INS_OUT_FIELD_NAME, tpl_INS_OUT_FIELD_COMMA, tpl_INS_OUT_FIELD_INIT, tpl_INS_OUT_FIELD_GETVALUE, tpl_INS_OUT_FIELD_ISNULL, tpl_INS_OUT_FIELD_COMMENT,
        tpl_INS_OUT_KEY_FIELD_NAME, tpl_INS_OUT_KEY_FIELD_TYPE
    },
    updateFieldMap = 
    {
        tpl_UPDATE, 
        tpl_UPDATE_SQL, tpl_UPDATE_SQL_LEN, tpl_UPDATE_SQL_UNESCAPED,
        tpl_UPDATE_PARAM_COUNT, tpl_UPDATE_FIELD_COUNT,
        tpl_UPDATE_HAS_PARAMS,
        tpl_UPD_IN_FIELDS, tpl_UPD_IN_FIELD_TYPE, tpl_UPD_IN_FIELD_NAME, tpl_UPD_IN_FIELD_COMMA, tpl_UPD_IN_FIELD_INIT, tpl_UPD_IN_FIELD_BIND,
        tpl_UPD_OUT_FIELDS, tpl_UPD_OUT_FIELD_TYPE, tpl_UPD_OUT_FIELD_NAME, tpl_UPD_OUT_FIELD_COMMA, tpl_UPD_OUT_FIELD_INIT, tpl_UPD_OUT_FIELD_GETVALUE, tpl_UPD_OUT_FIELD_ISNULL, tpl_UPD_OUT_FIELD_COMMENT,
        tpl_UPD_OUT_KEY_FIELD_NAME, tpl_UPD_OUT_KEY_FIELD_TYPE
    },
    deleteFieldMap = 
    {
        tpl_DELETE, 
        tpl_DELETE_SQL, tpl_DELETE_SQL_LEN, tpl_DELETE_SQL_UNESCAPED,
        tpl_DELETE_PARAM_COUNT, tpl_DELETE_FIELD_COUNT,
        tpl_DELETE_HAS_PARAMS,
        tpl_DEL_IN_FIELDS, tpl_DEL_IN_FIELD_TYPE, tpl_DEL_IN_FIELD_NAME, tpl_DEL_IN_FIELD_COMMA, tpl_DEL_IN_FIELD_INIT, tpl_DEL_IN_FIELD_BIND,
        tpl_DEL_OUT_FIELDS, tpl_DEL_OUT_FIELD_TYPE, tpl_DEL_OUT_FIELD_NAME, tpl_DEL_OUT_FIELD_COMMA, tpl_DEL_OUT_FIELD_INIT, tpl_DEL_OUT_FIELD_GETVALUE, tpl_DEL_OUT_FIELD_ISNULL, tpl_DEL_OUT_FIELD_COMMENT,
        tpl_DEL_OUT_KEY_FIELD_NAME, tpl_DEL_OUT_KEY_FIELD_TYPE
    },
    storedProcedureFieldMap = 
    {
        tpl_SPROC, 
        tpl_SPROC_SQL, tpl_SPROC_SQL_LEN, tpl_SPROC_SQL_UNESCAPED,
        tpl_SPROC_PARAM_COUNT, tpl_SPROC_FIELD_COUNT,
        tpl_SPROC_HAS_PARAMS,
        tpl_SP_IN_FIELDS, tpl_SP_IN_FIELD_TYPE, tpl_SP_IN_FIELD_NAME, tpl_SP_IN_FIELD_COMMA, tpl_SP_IN_FIELD_INIT, tpl_SP_IN_FIELD_BIND,
        tpl_SP_OUT_FIELDS, tpl_SP_OUT_FIELD_TYPE, tpl_SP_OUT_FIELD_NAME, tpl_SP_OUT_FIELD_COMMA, tpl_SP_OUT_FIELD_INIT, tpl_SP_OUT_FIELD_GETVALUE, tpl_SP_OUT_FIELD_ISNULL, tpl_SP_OUT_FIELD_COMMENT,
        tpl_SP_OUT_KEY_FIELD_NAME, tpl_SP_OUT_KEY_FIELD_TYPE
    };

void AbstractGenerator::setDictionaryElements(const TemplateFieldMap *_map, TemplateDictionary *_classDict, AbstractElements *_elements, SQLElement *_keyField)
{
    TemplateDictionary *subDict;
    std::string str;

    _classDict->SetValue(tpl_CLASSNAME, _elements->name);
    _classDict->ShowSection(_map->section);

    str = std::string("\"") + cescape(_elements->sql) + std::string("\"");
    _classDict->SetValue(_map->sql, str);
    _classDict->SetIntValue(_map->sqlLen, _elements->sql.length());
    _classDict->SetIntValue(_map->paramCount, _elements->input.size());
    _classDict->SetIntValue(_map->fieldCount, _elements->output.size());
    _classDict->SetValue(_map->sqlUnescaped, _elements->sql);

    int index = 0;
    subDict = 0;
    ListElements::iterator elit;

    if (_elements->input.size())
    {
        if (_map->hasParams)
            _classDict->ShowSection(_map->hasParams);

        for (elit = _elements->input.begin(); elit != _elements->input.end(); ++elit, ++index)
        {
            subDict = _classDict->AddSectionDictionary(_map->inFields);
            subDict->SetValue(_map->inFieldsType, getType(elit->type));
            subDict->SetValue(_map->inFieldsName, elit->name);
            subDict->SetValue(_map->inFieldsComma, ",");
            subDict->SetValue(_map->inFieldsInit, getInit(elit->type));
            subDict->SetValue(_map->inFieldsBind, getBind(sstSelect, elit, index));
        }

        if (subDict)
            subDict->SetValue(_map->inFieldsComma, "");
    }

    index = 0;
    subDict = 0;

    if (_elements->output.size())
    {
        for (elit = _elements->output.begin(); elit != _elements->output.end(); ++elit, ++index)
        {
            subDict = _classDict->AddSectionDictionary(_map->outFields);
            subDict->SetValue(_map->outFieldsType, getType(elit->type));
            subDict->SetValue(_map->outFieldsName, elit->name);
            subDict->SetValue(_map->outFieldsComma, ",");
            subDict->SetValue(_map->outFieldsInit, getInit(elit->type));
            subDict->SetValue(_map->outFieldsGetValue, getReadValue(sstSelect, elit, index));
            subDict->SetValue(_map->outFieldsIsNull, getIsNull(sstSelect, elit, index));
            subDict->SetValue(_map->outFieldsComment, elit->comment);
        }

        if (subDict)
            subDict->SetValue(_map->outFieldsComma, "");
    }

    if (_keyField)
    {
        _classDict->SetValue(_map->outFieldsKeyName, _keyField->name);
        _classDict->SetValue(_map->outFieldsKeyType, getType(_keyField->type));
    }
}


void AbstractGenerator::loadDictionary()
{
    std::string str;
    m_dict = new TemplateDictionary("dict");

    m_dict->SetValue( tpl_INTF_FILENAME, extractFileName( m_outIntFile ));
    m_dict->SetValue( tpl_IMPL_FILENAME, extractFileName( m_outImplFile ));

    str = extractFileName( stringToUpper( m_outIntFile ));
    std::string::size_type pos = str.find( '.' );
    while ( pos != std::string::npos )
    {
        str.replace( pos, 1, "_" );
        pos = str.find( '.' );
    }

    m_dict->SetValue( tpl_HEADER_NAME, str );

    foreach(str, m_namespaces)
        m_dict->SetValueAndShowSection(tpl_NAMESPACE, str, tpl_NAMESPACES);

    foreach(str, m_headers)
        m_dict->SetValueAndShowSection(tpl_EXTRA_HEADERS_HEADER, str, tpl_EXTRA_HEADERS);

    TemplateDictionary *classDict;
    classParams::iterator it;
    for(it = m_classParams.begin(); it != m_classParams.end(); ++it)
    {
        classDict = m_dict->AddSectionDictionary(tpl_CLASS);

        // ----- SELECT -----
        if ( !(it->second->select.sql.empty() ))
        {
            SelectElements *elements = &it->second->select;
            SQLElement *keyField = &elements->output.at(elements->keyField);
            setDictionaryElements(&selectFieldMap, classDict, elements, keyField);
            
            if (needIOBuffers())
            {
                addInBuffers(sstSelect, elements);
                addOutBuffers(sstSelect, elements);
            }
        }
        
        // ----- INSERT -----
        if ( !(it->second->insert.sql.empty() ))
        {
            AbstractElements *elements = &it->second->insert;
            setDictionaryElements(&insertFieldMap, classDict, elements, NULL);
            
            if (needIOBuffers())
                addInBuffers(sstInsert, elements);
        }
        
        // ----- UPDATE -----
        if ( !(it->second->update.sql.empty() ))
        {
            AbstractElements *elements = &it->second->update;
            setDictionaryElements(&updateFieldMap, classDict, elements, NULL);
            
            if (needIOBuffers())
                addInBuffers(sstUpdate, elements);
        }
        
        // ----- DELETE -----
        if ( !(it->second->del.sql.empty() ))
        {
            AbstractElements *elements = &it->second->del;
            setDictionaryElements(&deleteFieldMap, classDict, elements, NULL);
            
            if (needIOBuffers())
                addInBuffers(sstDelete, elements);
        }
        
        // ----- STORED PROCEDURE -----
        if ( !(it->second->stoProc.sql.empty() ))
        {
            SelectElements *elements = &it->second->stoProc;
            setDictionaryElements(&storedProcedureFieldMap, classDict, elements, NULL);
            
            if (needIOBuffers())
            {
                addInBuffers(sstStoredProcedure, elements);
                addOutBuffers(sstStoredProcedure, elements);
            }
        }
    }
}

bool AbstractGenerator::needIOBuffers() const
{
    return false;
}

void AbstractGenerator::addInBuffers(SQLStatementTypes /*_type*/, const AbstractElements* /*_elements*/)
{
}

void AbstractGenerator::addOutBuffers(SQLStatementTypes /*_type*/, const AbstractElements* /*_elements*/)
{
}

}
