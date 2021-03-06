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
const char * const tpl_UPDATE = "UPDATE";
const char * const tpl_INSERT = "INSERT";
const char * const tpl_DELETE = "DELETE";
const char * const tpl_SPROC = "SPROC";

const char * const tpl_STMT_SQL = "STMT_SQL";
const char * const tpl_STMT_SQL_UNESCAPED = "STMT_SQL_UNESCAPED";
const char * const tpl_STMT_SQL_LEN = "STMT_SQL_LEN";
const char * const tpl_STMT_FIELD_COUNT = "STMT_FIELD_COUNT";
const char * const tpl_STMT_PARAM_COUNT = "STMT_PARAM_COUNT";

const char * const tpl_STMT_HAS_PARAMS = "STMT_HAS_PARAMS";

const char * const tpl_STMT_IN_FIELDS = "STMT_IN_FIELDS";
const char * const tpl_STMT_IN_FIELD_TYPE = "STMT_IN_FIELD_TYPE";
const char * const tpl_STMT_IN_FIELD_NAME = "STMT_IN_FIELD_NAME";
const char * const tpl_STMT_IN_FIELD_COMMA = "STMT_IN_FIELD_COMMA";
const char * const tpl_STMT_IN_FIELD_INIT = "STMT_IN_FIELD_INIT";
const char * const tpl_STMT_IN_FIELD_BIND = "STMT_IN_FIELD_BIND";
const char * const tpl_STMT_IN_FIELDS_BUFFERS = "STMT_IN_FIELDS_BUFFERS";

const char * const tpl_STMT_OUT_FIELDS = "STMT_OUT_FIELDS";
const char * const tpl_STMT_OUT_FIELD_TYPE = "STMT_OUT_FIELD_TYPE";
const char * const tpl_STMT_OUT_FIELD_NAME = "STMT_OUT_FIELD_NAME";
const char * const tpl_STMT_OUT_FIELD_COMMA = "STMT_OUT_FIELD_COMMA";
const char * const tpl_STMT_OUT_FIELD_INIT = "STMT_OUT_FIELD_INIT";
const char * const tpl_STMT_OUT_FIELD_GETVALUE = "STMT_OUT_FIELD_GETVALUE";
const char * const tpl_STMT_OUT_FIELD_ISNULL = "STMT_OUT_FIELD_ISNULL";
const char * const tpl_STMT_OUT_FIELDS_BUFFERS = "STMT_OUT_FIELDS_BUFFERS";
const char * const tpl_STMT_OUT_FIELD_COMMENT = "STMT_OUT_FIELD_COMMENT";
const char * const tpl_STMT_OUT_KEY_FIELD_NAME = "STMT_OUT_KEY_FIELD_NAME";
const char * const tpl_STMT_OUT_KEY_FIELD_TYPE = "STMT_OUT_KEY_FIELD_TYPE";

const char * const tpl_BUFFER_DECLARE = "BUFFER_DECLARE";
const char * const tpl_BUFFER_ALLOC = "BUFFER_ALLOC";
const char * const tpl_BUFFER_FREE = "BUFFER_FREE";
const char * const tpl_BUFFER_INITIALIZE = "BUFFER_INITIALIZE";

const char * const tpl_DBENGINE_STATEMENT_TYPE = "DBENGINE_STATEMENT_TYPE";
const char * const tpl_DBENGINE_STATEMENT_NULL = "DBENGINE_STATEMENT_NULL";

const char * const tpl_DBENGINE_TRANSACTION = "DBENGINE_TRANSACTION";
const char * const tpl_DBENGINE_TRANSACTION_TYPE = "DBENGINE_TRANSACTION_TYPE";
const char * const tpl_DBENGINE_TRANSACTION_NULL = "DBENGINE_TRANSACTION_NULL";
const char * const tpl_DBENGINE_TRANSACTION_INIT = "DBENGINE_TRANSACTION_INIT";
const char * const tpl_DBENGINE_TRANSACTION_ROLLBACK = "DBENGINE_TRANSACTION_ROLLBACK";
const char * const tpl_DBENGINE_TRANSACTION_COMMIT = "DBENGINE_TRANSACTION_COMMIT";

const char * const tpl_DBENGINE_CONNECT_PARAMS = "DBENGINE_CONNECT_PARAMS";
const char * const tpl_DBENGINE_CONNECT_PARAM_TYPE = "DBENGINE_CONNECT_PARAM_TYPE";
const char * const tpl_DBENGINE_CONNECT_PARAM_PARAM = "DBENGINE_CONNECT_PARAM_PARAM";
const char * const tpl_DBENGINE_CONNECT_PARAM_VALUE = "DBENGINE_CONNECT_PARAM_VALUE";
const char * const tpl_DBENGINE_CONNECT_PARAM_COMMA = "DBENGINE_CONNECT_PARAM_COMMA";

const char * const tpl_DBENGINE_CONNECT = "DBENGINE_CONNECT";

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

const char * const tpl_DBENGINE_CREATE_SPROC = "DBENGINE_CREATE_SPROC";
const char * const tpl_DBENGINE_PREPARE_SPROC = "DBENGINE_PREPARE_SPROC";
const char * const tpl_DBENGINE_DESTROY_SPROC = "DBENGINE_DESTROY_SPROC";
const char * const tpl_DBENGINE_RESET_SPROC = "DBENGINE_RESET_SPROC";
const char * const tpl_DBENGINE_EXECUTE_SPROC = "DBENGINE_EXECUTE_SPROC";
const char * const tpl_DBENGINE_FETCH_SPROC = "DBENGINE_FETCH_SPROC";

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

AbstractGenerator* AbstractGenerator::s_generator = NULL;
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
        m_dict(NULL)
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
    TemplateDictionary *subDict = NULL;
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
        XMLNodePtr node = NULL;
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

                    node = NULL;
                    while( node = lang->IterateChildren( "includes", node ))
                    {
                        m_dict->ShowSection( tpl_DBENGINE_INCLUDES );

                        subnode = NULL;
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

                    node = NULL;
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

                    node = NULL;
                    while( node = lang->IterateChildren( "extra_headers", node ))
                    {
                        m_dict->ShowSection( tpl_EXTRA_HEADERS );


                        subnode = NULL;
                        while( subnode = node->IterateChildren( "define", subnode ))
                        {
                            subDict = m_dict->AddSectionDictionary( tpl_EXTRA_HEADERS );
                            elem = subnode->ToElement();
                            GET_TEXT_OR_ATTR_SET_TMPL( str, elem, "code", subDict, tpl_EXTRA_HEADERS_HEADER);
                        }

                        subnode = NULL;
                        while( subnode = node->IterateChildren( "type", subnode ))
                        {
                            subDict = m_dict->AddSectionDictionary( tpl_EXTRA_HEADERS );
                            elem = subnode->ToElement();
                            GET_TEXT_OR_ATTR_SET_TMPL( str, elem, "code", subDict, tpl_EXTRA_HEADERS_TYPE);
                        }

                        subnode = NULL;
                        while( subnode = node->IterateChildren( "member", subnode ))
                        {
                            subDict = m_dict->AddSectionDictionary( tpl_EXTRA_HEADERS );
                            elem = subnode->ToElement();
                            GET_TEXT_OR_ATTR_SET_TMPL( str, elem, "code", subDict, tpl_EXTRA_HEADERS_MEMBER);
                        }
                    }

                    node = NULL;
                    while( node = lang->IterateChildren( "types", node ))
                    {
                        subnode = NULL;
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

                    node = NULL;
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
                    
                    node = 0;
                    while( node = lang->IterateChildren( "sproc", node ))
                    {
                        elem = node->FirstChildElement("create", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_CREATE_SPROC, parseStringVariables(elem->GetText(false)));

                        elem = node->FirstChildElement("destroy", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_DESTROY_SPROC, parseStringVariables(elem->GetText(false)));

                        elem = node->FirstChildElement("prepare", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_PREPARE_SPROC, parseStringVariables(elem->GetText(false)));

                        elem = node->FirstChildElement("execute", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_EXECUTE_SPROC, parseStringVariables(elem->GetText(false)));

                        elem = node->FirstChildElement("fetch", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_FETCH_SPROC, parseStringVariables(elem->GetText(false)));

                        elem = node->FirstChildElement("reset", false);
                        if ( elem ) m_dict->SetValue(tpl_DBENGINE_RESET_SPROC, parseStringVariables(elem->GetText(false)));
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

TemplateDictionary* AbstractGenerator::setDictionaryElements(const char *_section, TemplateDictionary *_classDict, AbstractElements *_elements, SQLElement *_keyField)
{
    TemplateDictionary *subDict;
    std::string str;

    _classDict->SetValue(tpl_CLASSNAME, _elements->name);
    
    TemplateDictionary *result = _classDict->AddSectionDictionary(_section);

    str = std::string("\"") + cescape(_elements->sql) + std::string("\"");
    _classDict->SetValue(tpl_STMT_SQL, str);
    _classDict->SetIntValue(tpl_STMT_SQL_LEN, _elements->sql.length());
    _classDict->SetIntValue(tpl_STMT_PARAM_COUNT, _elements->input.size());
    _classDict->SetIntValue(tpl_STMT_FIELD_COUNT, _elements->output.size());
    _classDict->SetValue(tpl_STMT_SQL_UNESCAPED, _elements->sql);

    int index = 0;
    subDict = 0;
    ListElements::iterator elit;

    if (_elements->input.size())
    {
        _classDict->ShowSection(tpl_STMT_HAS_PARAMS);

        for (elit = _elements->input.begin(); elit != _elements->input.end(); ++elit, ++index)
        {
            subDict = _classDict->AddSectionDictionary(tpl_STMT_IN_FIELDS);
            subDict->SetValue(tpl_STMT_IN_FIELD_TYPE, getType(elit->type));
            subDict->SetValue(tpl_STMT_IN_FIELD_NAME, elit->name);
            subDict->SetValue(tpl_STMT_IN_FIELD_COMMA, ",");
            subDict->SetValue(tpl_STMT_IN_FIELD_INIT, getInit(elit->type));
            subDict->SetValue(tpl_STMT_IN_FIELD_BIND, getBind(sstSelect, elit, index));
        }

        if (subDict)
            subDict->SetValue(tpl_STMT_IN_FIELD_COMMA, "");
    }

    index = 0;
    subDict = 0;

    if (_elements->output.size())
    {
        for (elit = _elements->output.begin(); elit != _elements->output.end(); ++elit, ++index)
        {
            subDict = _classDict->AddSectionDictionary(tpl_STMT_OUT_FIELDS);
            subDict->SetValue(tpl_STMT_OUT_FIELD_TYPE, getType(elit->type));
            subDict->SetValue(tpl_STMT_OUT_FIELD_NAME, elit->name);
            subDict->SetValue(tpl_STMT_OUT_FIELD_COMMA, ",");
            subDict->SetValue(tpl_STMT_OUT_FIELD_INIT, getInit(elit->type));
            subDict->SetValue(tpl_STMT_OUT_FIELD_GETVALUE, getReadValue(sstSelect, elit, index));
            subDict->SetValue(tpl_STMT_OUT_FIELD_ISNULL, getIsNull(sstSelect, elit, index));
            subDict->SetValue(tpl_STMT_OUT_FIELD_COMMENT, elit->comment);
        }

        if (subDict)
            subDict->SetValue(tpl_STMT_OUT_FIELD_COMMA, "");
    }

    if (_keyField)
    {
        _classDict->SetValue(tpl_STMT_OUT_KEY_FIELD_NAME, _keyField->name);
        _classDict->SetValue(tpl_STMT_OUT_KEY_FIELD_TYPE, getType(_keyField->type));
    }
    
    return result;
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
            TemplateDictionary *subDict = setDictionaryElements(tpl_SELECT, classDict, elements, keyField);
            
            if (needIOBuffers())
            {
                addInBuffers(sstSelect, subDict, elements);
                addOutBuffers(sstSelect, subDict, elements);
            }
        }
        
        // ----- INSERT -----
        if ( !(it->second->insert.sql.empty() ))
        {
            AbstractElements *elements = &it->second->insert;
            TemplateDictionary *subDict = setDictionaryElements(tpl_INSERT, classDict, elements, NULL);
            
            if (needIOBuffers())
                addInBuffers(sstInsert, subDict, elements);
        }
        
        // ----- UPDATE -----
        if ( !(it->second->update.sql.empty() ))
        {
            AbstractElements *elements = &it->second->update;
            TemplateDictionary *subDict = setDictionaryElements(tpl_UPDATE, classDict, elements, NULL);
            
            if (needIOBuffers())
                addInBuffers(sstUpdate, subDict, elements);
        }
        
        // ----- DELETE -----
        if ( !(it->second->del.sql.empty() ))
        {
            AbstractElements *elements = &it->second->del;
            TemplateDictionary *subDict = setDictionaryElements(tpl_DELETE, classDict, elements, NULL);
            
            if (needIOBuffers())
                addInBuffers(sstDelete, subDict, elements);
        }
        
        // ----- STORED PROCEDURE -----
        if ( !(it->second->stoProc.sql.empty() ))
        {
            SelectElements *elements = &it->second->stoProc;
            TemplateDictionary *subDict = setDictionaryElements(tpl_SPROC, classDict, elements, NULL);
            
            if (needIOBuffers())
            {
                addInBuffers(sstStoredProcedure, subDict, elements);
                addOutBuffers(sstStoredProcedure, subDict, elements);
            }
        }
    }
}

bool AbstractGenerator::needIOBuffers() const
{
    return false;
}

void AbstractGenerator::addInBuffers(SQLStatementTypes /*_type*/, TemplateDictionary */*_subDict*/, const AbstractElements */*_elements*/)
{
}

void AbstractGenerator::addOutBuffers(SQLStatementTypes /*_type*/, TemplateDictionary */*_subDict*/, const AbstractElements */*_elements*/)
{
}

}
