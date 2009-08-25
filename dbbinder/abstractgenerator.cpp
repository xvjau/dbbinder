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

#include "main.h"
#include "TinyXML/nvXML.h"

namespace DBBinder
{

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
const char * const tpl_BUFFER_INITIALIZE = "BUFFER_INITIALIZE";

const char * const tpl_SEL_OUT_FIELDS = "SEL_OUT_FIELDS";
const char * const tpl_SEL_OUT_FIELD_TYPE = "SEL_OUT_FIELD_TYPE";
const char * const tpl_SEL_OUT_FIELD_NAME = "SEL_OUT_FIELD_NAME";
const char * const tpl_SEL_OUT_FIELD_COMMA = "SEL_OUT_FIELD_COMMA";
const char * const tpl_SEL_OUT_FIELD_INIT = "SEL_OUT_FIELD_INIT";
const char * const tpl_SEL_OUT_FIELD_GETVALUE = "SEL_OUT_FIELD_GETVALUE";
const char * const tpl_SEL_OUT_FIELDS_BUFFERS = "SEL_OUT_FIELDS_BUFFERS";

const char * const tpl_DBENGINE_STATEMENT_TYPE = "DBENGINE_STATEMENT_TYPE";
const char * const tpl_DBENGINE_STATEMENT_NULL = "DBENGINE_STATEMENT_NULL";

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

const char * const tpl_DBENGINE_GLOBAL_PARAMS = "DBENGINE_GLOBAL_PARAMS";
const char * const tpl_TYPE = "TYPE";
const char * const tpl_PARAM = "PARAM";
const char * const tpl_VALUE = "VALUE";

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

SQLTypes typeNameToSQLType(String _name)
{
	if ( _name.length() )
	{
		_name = stringToLower( _name );

		switch( _name[0] )
		{
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

AbstractGenerator* AbstractGenerator::s_generator = 0;
AbstractGenerator* AbstractGenerator::getGenerator(const String & _type)
{
	if ( s_generator )
		return s_generator;

	if ( _type.length() > 1 )
	{
		String type = stringToLower( _type );
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

String AbstractGenerator::getType(SQLTypes _sqlType)
{
	String result = m_types[_sqlType];

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
			case stText:
			{
				result = "std::string";
				break;
			}
		}
	}

	return result;
}

String AbstractGenerator::getInit(SQLTypes _sqlType)
{
	String result;

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
		case stTimeStamp:
		case stTime:
		case stDate:
		case stText:
		{
			result = "\"\"";
			break;
		}
	}

	return result;
}

void AbstractGenerator::addSelect(SelectElements _elements)
{
	String name = stringToLower( _elements.name );

	_classParamsPtr params = m_classParams[ name ];

	if ( !params )
		params.reset( new _classParams );

	params->select = _elements;
	m_classParams[ name ] = params;
}

void AbstractGenerator::addUpdate(UpdateElements _elements)
{
	String name = stringToLower( _elements.name );

	_classParamsPtr params = m_classParams[ name ];

	if ( !params )
		params.reset( new _classParams );

	params->update = _elements;
	m_classParams[ name ] = params;
}

void AbstractGenerator::addInsert(InsertElements _elements)
{
	String name = stringToLower( _elements.name );

	_classParamsPtr params = m_classParams[ name ];

	if ( !params )
		params.reset( new _classParams );

	params->insert = _elements;
	m_classParams[ name ] = params;
}

static void cleanExcessiveLineBreaks(String &_str, std::ostream &_stream)
{
	int ln = 0;
	foreach(char c, _str)
	{
		if ( c == '\n' )
			++ln;
		else
		{
			if ( c != ' ' )
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

	String str;
	{
		std::ofstream out( m_outIntFile.c_str(), std::ios_base::trunc );
		m_templ[ftIntf]->Expand(&str, m_dict);

		cleanExcessiveLineBreaks(str, out);
		//out << str;
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
		m_templ[ftImpl]->Expand(&str, m_dict);

		cleanExcessiveLineBreaks(str, out);
		//out << str;

		str.clear();
	}

	// TODO Put this in an XML
	str = "astyle --style=ansi -n > /dev/null 2>&1 ";
	str += m_outImplFile;

	if ( system( str.c_str() ) == -1 )
		std::cerr << "warning: unable to run astyle" << std::endl;

	str.clear();
}

void AbstractGenerator::loadDatabase()
{
	ctemplate::TemplateDictionary *subDict;
	_dbParams::iterator it;
	for(it = m_dbParams.begin(); it != m_dbParams.end(); ++it)
	{
		subDict = m_dict->AddSectionDictionary(tpl_DBENGINE_GLOBAL_PARAMS);

		subDict->SetValue(tpl_PARAM, it->first);

		if ( it->second.isInt )
		{
			subDict->SetValue(tpl_VALUE, it->second.value);
			subDict->SetValue(tpl_TYPE, "const int");
		}
		else
		{
			subDict->SetValue(tpl_VALUE, String("\"") + cescape(it->second.value) + String("\""));
			subDict->SetValue(tpl_TYPE, "const char * const");
		}
	}

	String str;
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

	FATAL("dbengine: template file not found for current language.");

LOADED:
	{}
}

bool AbstractGenerator::loadXMLDatabase(const String& _path)
{
#define GET_TEXT_OR_ATTR_SET_TMPL( str, elem, attr, tmpl, var ) \
	GET_TEXT_OR_ATTR( str, elem, attr ); \
	if ( !str.empty() ) { result = true; tmpl->SetValue(var, str); }

	ListString templates = stringTok(optTemplate, ',');
	ListString::iterator templ;
	ctemplate::TemplateDictionary *subDict;

	bool result = false;
	try
	{
		String s = _path + m_dbengine + ".xml";
		XMLDocument xmlFile( s );
		xmlFile.LoadFile();

		DBBinder::optDepends.push_back( s );

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
					String str;

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
								str = String("#include <") + str + ">";
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
							else
							{
								elem = subnode->ToElement();
								m_dict->ShowSection( tpl_DBENGINE_EXTRAS );
								subDict = m_dict->AddSectionDictionary( tpl_DBENGINE_EXTRAS );
								subDict->SetValue( tpl_DBENGINE_EXTRA_VAR,
										elem->FirstChildElement("type")->GetAttributeOrDefault("value", "") + " " +
										elem->FirstChildElement("name")->GetAttributeOrDefault("value", "") + ";"
												 );
							}
						}
					}

					node = 0;
					while( node = lang->IterateChildren( "connect", node ))
					{
						m_dict->SetValue(tpl_DBENGINE_CONNECT, node->ToElement()->GetText());
					}

					node = 0;
					while( node = lang->IterateChildren( "disconnect", node ))
					{
						m_dict->SetValue(tpl_DBENGINE_DISCONNECT, node->ToElement()->GetText());
					}

					node = 0;
					while( node = lang->IterateChildren( "select", node ))
					{
						elem = node->FirstChildElement("create", false);
						if ( elem ) m_dict->SetValue(tpl_DBENGINE_CREATE_SELECT, elem->GetText(false));

						elem = node->FirstChildElement("destroy", false);
						if ( elem ) m_dict->SetValue(tpl_DBENGINE_DESTROY_SELECT, elem->GetText(false));

						elem = node->FirstChildElement("prepare", false);
						if ( elem ) m_dict->SetValue(tpl_DBENGINE_PREPARE_SELECT, elem->GetText(false));

						elem = node->FirstChildElement("execute", false);
						if ( elem ) m_dict->SetValue(tpl_DBENGINE_EXECUTE_SELECT, elem->GetText(false));

						elem = node->FirstChildElement("fetch", false);
						if ( elem ) m_dict->SetValue(tpl_DBENGINE_FETCH_SELECT, elem->GetText(false));

						elem = node->FirstChildElement("reset", false);
						if ( elem ) m_dict->SetValue(tpl_DBENGINE_RESET_SELECT, elem->GetText(false));
					}

					node = 0;
					while( node = lang->IterateChildren( "update", node ))
					{
						elem = node->FirstChildElement("create", false);
						if ( elem ) m_dict->SetValue(tpl_DBENGINE_CREATE_UPDATE, elem->GetText(false));

						elem = node->FirstChildElement("destroy", false);
						if ( elem ) m_dict->SetValue(tpl_DBENGINE_DESTROY_UPDATE, elem->GetText(false));

						elem = node->FirstChildElement("prepare", false);
						if ( elem ) m_dict->SetValue(tpl_DBENGINE_PREPARE_UPDATE, elem->GetText(false));

						elem = node->FirstChildElement("execute", false);
						if ( elem ) m_dict->SetValue(tpl_DBENGINE_EXECUTE_UPDATE, elem->GetText(false));

						elem = node->FirstChildElement("reset", false);
						if ( elem ) m_dict->SetValue(tpl_DBENGINE_RESET_UPDATE, elem->GetText(false));
					}

					node = 0;
					while( node = lang->IterateChildren( "insert", node ))
					{
						elem = node->FirstChildElement("create", false);
						if ( elem ) m_dict->SetValue(tpl_DBENGINE_CREATE_INSERT, elem->GetText(false));

						elem = node->FirstChildElement("destroy", false);
						if ( elem ) m_dict->SetValue(tpl_DBENGINE_DESTROY_INSERT, elem->GetText(false));

						elem = node->FirstChildElement("prepare", false);
						if ( elem ) m_dict->SetValue(tpl_DBENGINE_PREPARE_INSERT, elem->GetText(false));

						elem = node->FirstChildElement("execute", false);
						if ( elem ) m_dict->SetValue(tpl_DBENGINE_EXECUTE_INSERT, elem->GetText(false));

						elem = node->FirstChildElement("reset", false);
						if ( elem ) m_dict->SetValue(tpl_DBENGINE_RESET_INSERT, elem->GetText(false));
					}
				}
			}
		}
	}
	catch( ticpp::Exception &e )
	{
		WARNING("XML: " << _path << ": " << e.what());
	}
	return result;
}

bool AbstractGenerator::loadYAMLDatabase(const String& _path)
{
	FATAL("YAML parser not implemented yet. sorry.");
	DBBinder::optDepends.push_back( _path );
}

void AbstractGenerator::loadTemplates()
{
	String str;

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
				if ( loadXMLTemplate( str ) && m_templ[ftIntf] && m_templ[ftImpl] )
					goto LOADED;
			}
			else if ( stat((str + "template.yaml").c_str(), &fs) == 0 ) // Check if file exists
			{
				if ( loadYAMLTemplate( str ) && m_templ[ftIntf] && m_templ[ftImpl] )
					goto LOADED;
			}
		}
		while(--count);
	}

LOADED:
	if (!( m_templ[ftIntf] && m_templ[ftImpl] ))
		FATAL("template: template files not found.");
}

bool AbstractGenerator::loadXMLTemplate(const String & _path)
{
	try
	{
		struct stat fs;

		XMLDocument xmlFile( _path + "template.xml" );
		xmlFile.LoadFile();

		DBBinder::optDepends.push_back( _path + "template.xml" );

		XMLElementPtr xml = xmlFile.FirstChildElement("xml");

		String str;
		XMLElementPtr elem;

#define READ_PARAM(XML, ENUM, STR) \
		elem = xml->FirstChildElement(XML);																\
		elem->GetAttribute("file", &str, false);														\
		if ( str.empty() )																				\
			elem->GetText( &str, false );																\
		if ( str.length() )																				\
		{																								\
			if ( str[0] != '/' )																		\
				str = _path + '/' + str;																\
			if ( stat(str.c_str(), &fs) == 0 )															\
			{																							\
				m_templ[ENUM] = ctemplate::Template::GetTemplate(str, ctemplate::DO_NOT_STRIP);				\
			}																							\
			else																						\
			{																							\
				WARNING("file :" << str << " does not exist!");											\
			}																							\
		}																								\
		str.clear();																					\
		elem->GetAttribute("extension", &str, false);													\
		if ( str.empty() )																				\
			elem->GetText( &str, false );																\
		if ( str.length() )																				\
			STR = optOutput + str;

		READ_PARAM("interface", ftIntf, m_outIntFile);
		READ_PARAM("implementation", ftImpl, m_outImplFile);
#undef READ_PARAM
		return true;
	}
	catch( ticpp::Exception &e )
	{
		WARNING("XML: " << _path << ": " << e.what());
	}
	return false;
}

bool AbstractGenerator::loadYAMLTemplate(const String & _path)
{
	FATAL("YAML parser not implemented yet. sorry.");
	DBBinder::optDepends.push_back( _path );
}

void AbstractGenerator::loadDictionary()
{
	String str;
	m_dict = new ctemplate::TemplateDictionary("dict");

	m_dict->SetValue( tpl_INTF_FILENAME, extractFileName( m_outIntFile ));
	m_dict->SetValue( tpl_IMPL_FILENAME, extractFileName( m_outImplFile ));

	str = extractFileName( stringToUpper( m_outIntFile ));
	String::size_type pos = str.find( '.' );
	while ( pos != String::npos )
	{
		str.replace( pos, 1, "_" );
		pos = str.find( '.' );
	}

	m_dict->SetValue( tpl_HEADER_NAME, str );

	foreach(str, m_namespaces)
		m_dict->SetValueAndShowSection(tpl_NAMESPACE, str, tpl_NAMESPACES);

	foreach(str, m_headers)
		m_dict->SetValueAndShowSection(tpl_EXTRA_HEADERS_HEADER, str, tpl_EXTRA_HEADERS);

	ctemplate::TemplateDictionary *classDict, *subDict;
	classParams::iterator it;
	for(it = m_classParams.begin(); it != m_classParams.end(); ++it)
	{
		classDict = m_dict->AddSectionDictionary(tpl_CLASS);

		// ----- SELECT -----
		if ( !(it->second->select.sql.empty() ))
		{
			classDict->SetValue(tpl_CLASSNAME, it->second->select.name);
			classDict->ShowSection(tpl_SELECT);

			str = String("\"") + cescape(it->second->select.sql) + String("\"");
			classDict->SetValue( tpl_SELECT_SQL, str );
			classDict->SetIntValue( tpl_SELECT_SQL_LEN, it->second->select.sql.length() );
			classDict->SetIntValue( tpl_SELECT_PARAM_COUNT, it->second->select.input.size() );
			classDict->SetIntValue( tpl_SELECT_FIELD_COUNT, it->second->select.output.size() );
			classDict->SetValue( tpl_SELECT_SQL_UNESCAPED, it->second->select.sql );

			int index = 0;
			subDict = 0;
			ListElements::iterator elit;

			if ( it->second->select.input.size() )
			{
				classDict->ShowSection( tpl_SELECT_HAS_PARAMS );

				for(elit = it->second->select.input.begin(); elit != it->second->select.input.end(); ++elit, ++index)
				{
					subDict = classDict->AddSectionDictionary(tpl_SEL_IN_FIELDS);
					subDict->SetValue( tpl_SEL_IN_FIELD_TYPE, getType( elit->type ));
					subDict->SetValue( tpl_SEL_IN_FIELD_NAME, elit->name );
					subDict->SetValue( tpl_SEL_IN_FIELD_COMMA, "," );
					subDict->SetValue( tpl_SEL_IN_FIELD_INIT, getInit( elit->type ));
					subDict->SetValue( tpl_SEL_IN_FIELD_BIND, getBind( sstSelect, elit, index ));
				}
				if ( subDict )
					subDict->SetValue( tpl_SEL_IN_FIELD_COMMA, "" );
			}

			index = 0;
			subDict = 0;
			for(elit = it->second->select.output.begin(); elit != it->second->select.output.end(); ++elit, ++index)
			{
				subDict = classDict->AddSectionDictionary(tpl_SEL_OUT_FIELDS);
				subDict->SetValue( tpl_SEL_OUT_FIELD_TYPE, getType( elit->type ));
				subDict->SetValue( tpl_SEL_OUT_FIELD_NAME, elit->name );
				subDict->SetValue( tpl_SEL_OUT_FIELD_COMMA, "," );
				subDict->SetValue( tpl_SEL_OUT_FIELD_INIT, getInit( elit->type ));
				subDict->SetValue( tpl_SEL_OUT_FIELD_GETVALUE, getReadValue( sstSelect, elit, index ));
			}
			if ( subDict )
				subDict->SetValue( tpl_SEL_OUT_FIELD_COMMA, "" );

			if ( needIOBuffers() )
			{
				addSelInBuffers( &it->second->select );
				addSelOutBuffers( &it->second->select );
			}
		}

		// ----- UPDATE -----
		if ( !(it->second->update.sql.empty() ))
		{
			classDict->SetValue(tpl_CLASSNAME, it->second->update.name);
			classDict->ShowSection(tpl_UPDATE);

			str = String("\"") + cescape(it->second->update.sql) + String("\"");
			classDict->SetValue( tpl_UPDATE_SQL, str );
			classDict->SetIntValue( tpl_UPDATE_SQL_LEN, it->second->update.sql.length() );
			classDict->SetIntValue( tpl_UPDATE_PARAM_COUNT, it->second->update.input.size() );
			classDict->SetValue( tpl_UPDATE_SQL_UNESCAPED, it->second->update.sql );

			int index = 0;
			subDict = 0;
			ListElements::iterator elit;
			for(elit = it->second->update.input.begin(); elit != it->second->update.input.end(); ++elit, ++index)
			{
				subDict = classDict->AddSectionDictionary(tpl_UPD_IN_FIELDS);
				subDict->SetValue( tpl_UPD_IN_FIELD_TYPE, getType( elit->type ));
				subDict->SetValue( tpl_UPD_IN_FIELD_NAME, elit->name );
				subDict->SetValue( tpl_UPD_IN_FIELD_COMMA, "," );
				subDict->SetValue( tpl_UPD_IN_FIELD_INIT, getInit( elit->type ));
				subDict->SetValue( tpl_UPD_IN_FIELD_BIND, getBind( sstUpdate, elit, index ));
			}
			if ( subDict )
				subDict->SetValue( tpl_UPD_IN_FIELD_COMMA, "" );
		}

		// ----- INSERT -----
		if ( !(it->second->insert.sql.empty() ))
		{
			classDict->SetValue(tpl_CLASSNAME, it->second->insert.name);
			classDict->ShowSection(tpl_INSERT);

			str = String("\"") + cescape(it->second->insert.sql) + String("\"");
			classDict->SetValue( tpl_INSERT_SQL, str );
			classDict->SetIntValue( tpl_INSERT_SQL_LEN, it->second->insert.sql.length() );
			classDict->SetIntValue( tpl_INSERT_PARAM_COUNT, it->second->insert.input.size() );
			classDict->SetValue( tpl_INSERT_SQL_UNESCAPED, it->second->insert.sql );

			int index = 0;
			subDict = 0;
			ListElements::iterator elit;
			for(elit = it->second->insert.input.begin(); elit != it->second->insert.input.end(); ++elit, ++index)
			{
				subDict = classDict->AddSectionDictionary(tpl_INS_IN_FIELDS);
				subDict->SetValue( tpl_INS_IN_FIELD_TYPE, getType( elit->type ));
				subDict->SetValue( tpl_INS_IN_FIELD_NAME, elit->name );
				subDict->SetValue( tpl_INS_IN_FIELD_COMMA, "," );
				subDict->SetValue( tpl_INS_IN_FIELD_INIT, getInit( elit->type ));
				subDict->SetValue( tpl_INS_IN_FIELD_BIND, getBind( sstInsert, elit, index ));
			}
			if ( subDict )
				subDict->SetValue( tpl_INS_IN_FIELD_COMMA, "" );
		}
	}
}

bool AbstractGenerator::needIOBuffers() const
{
	return false;
}

void AbstractGenerator::addSelInBuffers(const SelectElements* _select)
{
}

void AbstractGenerator::addSelOutBuffers(const SelectElements* _select)
{
}

}
