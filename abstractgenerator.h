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

#ifndef DBBINDERABSTRACTGENERATOR_H
#define DBBINDERABSTRACTGENERATOR_H

#include "main.h"
#include <sstream>
#include <boost/smart_ptr.hpp>

namespace DBBinder
{

enum SQLTypes
{
	stUnknown,
	stInt,
	stInt64,
	stUInt,
	stUInt64,
	stFloat,
	stDouble,
	stUFloat,
	stUDouble,
	stTimeStamp,
	stTime,
	stDate,
	stText
};
SQLTypes typeNameToSQLType(String _name);

enum SQLStatementTypes
{
	sstUnknown,
	sstSelect,
	sstInsert,
	sstUpdate,
	sstDelete,
};

struct SQLElement
{
	SQLElement( const String& _name, SQLTypes _type, int _index = -1, int _length = 0, const String& _comment = "" ):
			name( _name ), type( _type ), index( _index ), length( _length ), comment(_comment)
	{}
	SQLElement( const String& _name, SQLTypes _type, int _index, const String& _default, const String& _comment = "" ):
			name( _name ), type( _type ), index( _index ), length(0), defaultValue( _default ), comment(_comment)
	{}

	String		name;
	SQLTypes	type;
	int			index;
	int			length;
	String		defaultValue;
	String		comment;
};
typedef std::list<SQLElement> ListElements;

struct Location
{
	String	file;
	int		line;
	int		col;
};

struct AbstractElements
{
	String			name;
	String			sql;
	Location		sql_location;
	ListElements	input;
	SQLStatementTypes	type;
};

struct AbstractIOElements: public AbstractElements
{
	ListElements	output;
};

struct SelectElements: public AbstractIOElements
{
	SelectElements(): AbstractIOElements() { type = sstSelect; }
};

struct UpdateElements: public AbstractElements
{
	UpdateElements(): AbstractElements() { type = sstUpdate; }
};

struct InsertElements: public AbstractElements
{
	InsertElements(): AbstractElements() { type = sstInsert; }
};

struct DeleteElements: public AbstractElements
{
	DeleteElements(): AbstractElements() { type = sstDelete; }
};

class AbstractGenerator
{
	protected:
		AbstractGenerator();
		virtual ~AbstractGenerator();

		struct dbParam
		{
			dbParam():
				isInt( false )
			{}

			dbParam( const String& _value ):
				isInt( false )
			{
				value = _value;
			}

			dbParam( int _value ):
				isInt( true )
			{
				std::stringstream str;
				str << _value;
				value = str.str();
			}

			String	value;
			bool	isInt;
		};
		typedef std::map<String, dbParam> _dbParams;

		_dbParams	m_dbParams;

		enum _fileTypes
		{
			ftIntf,
			ftImpl,
			ftMAX
		};
		struct _classParams
		{
			SelectElements	select;
			UpdateElements	update;
			InsertElements	insert;
			DeleteElements	del;
		};
		typedef boost::shared_ptr<_classParams> _classParamsPtr;
		typedef std::map<String, _classParamsPtr> classParams;

		typedef std::map<SQLTypes, String> mapTypes;
		mapTypes	m_types;

		ListString	m_namespaces;
		ListString	m_headers;

		struct TmplDestPair
		{
			String tmpl;
			String dest;
		};
		typedef std::list<TmplDestPair> ListTplDestPair;
		ListTplDestPair m_extraFiles;

		String		m_dbengine;

		classParams	m_classParams;

		bool m_connected;
		virtual bool checkConnection();

		String	m_outIntFile;
		String	m_outImplFile;

		TemplateDictionary	*m_dict;
		Template			*m_templ[ftMAX];

		virtual void loadDictionary();

		virtual void loadDatabase();
		virtual bool loadXMLDatabase(const String& _path);
		virtual bool loadYAMLDatabase(const String& _path);

		virtual void loadTemplates();
		virtual bool loadXMLTemplate(const String& _path);
		virtual bool loadYAMLTemplate(const String& _path);

		// <SUCKS>
		// TODO: There must be a better way to implement this.
		virtual bool needIOBuffers() const;

		virtual void addInBuffers(SQLStatementTypes _type, const AbstractElements* _elements);
		virtual void addOutBuffers(SQLStatementTypes _type, const AbstractIOElements* _elements);

		virtual String getBind(SQLStatementTypes _type, const ListElements::iterator& _item, int _index) = 0;
		virtual String getReadValue(SQLStatementTypes _type, const ListElements::iterator& _item, int _index) = 0;
		virtual String getIsNull(SQLStatementTypes _type, const ListElements::iterator& _item, int _index) = 0;

		// </SUCKS>

		//TODO: This might be better implemented;
		static AbstractGenerator* s_generator;
	public:
		static AbstractGenerator* getGenerator() { return s_generator; }
		static AbstractGenerator* getGenerator(const String& _type);

		void setDBParam(const String& _key, const String& _value)
		{
			m_dbParams[_key] = _value;
		}

		void setDBParam(const String& _key, const int _value)
		{
			m_dbParams[_key] = _value;
		}

		void setType(SQLTypes _sqlType, const String& _genType)
		{
			m_types[_sqlType] = _genType;
		}

		String getType(SQLTypes _sqlType);
		String getInit(SQLTypes _sqlType);

		void addNamespace(const String& _name)
		{
			m_namespaces.push_back(_name);
		}

		void addHeader(const String& _header)
		{
			m_headers.push_back(_header);
		}

		virtual void generate();

		virtual void addSelect(SelectElements _elements);
		virtual void addUpdate(UpdateElements _elements);
		virtual void addInsert(InsertElements _elements);
		virtual void addDelete(DeleteElements _elements);
};
typedef AbstractGenerator* AbstractGeneratorPtr;

extern const char * const tpl_INTF_FILENAME;
extern const char * const tpl_IMPL_FILENAME;

extern const char * const tpl_HEADER_NAME;
extern const char * const tpl_DBENGINE_INCLUDES;
extern const char * const tpl_DBENGINE_INCLUDE_NAME;
extern const char * const tpl_EXTRA_HEADERS;
extern const char * const tpl_EXTRA_HEADERS_HEADER;
extern const char * const tpl_DBENGINE_GLOBAL_FUNCTIONS;
extern const char * const tpl_FUNCTION;

extern const char * const tpl_NAMESPACES;
extern const char * const tpl_NAMESPACE;

extern const char * const tpl_CLASS;
extern const char * const tpl_CLASSNAME;

extern const char * const tpl_DBENGINE_CONNECTION_TYPE;
extern const char * const tpl_DBENGINE_CONNECTION_NULL;

extern const char * const tpl_SELECT;
extern const char * const tpl_SELECT_SQL;
extern const char * const tpl_SELECT_SQL_LEN;
extern const char * const tpl_SELECT_FIELD_COUNT;
extern const char * const tpl_SELECT_PARAM_COUNT;

extern const char * const tpl_SEL_IN_FIELDS;
extern const char * const tpl_SEL_IN_FIELD_TYPE;
extern const char * const tpl_SEL_IN_FIELD_NAME;
extern const char * const tpl_SEL_IN_FIELD_COMMA;
extern const char * const tpl_SEL_IN_FIELD_INIT;
extern const char * const tpl_SEL_IN_FIELD_BIND;
extern const char * const tpl_SEL_IN_FIELDS_BUFFERS;

extern const char * const tpl_BUFFER_DECLARE;
extern const char * const tpl_BUFFER_ALLOC;
extern const char * const tpl_BUFFER_FREE;
extern const char * const tpl_BUFFER_INITIALIZE;

extern const char * const tpl_SEL_OUT_FIELDS;
extern const char * const tpl_SEL_OUT_FIELD_TYPE;
extern const char * const tpl_SEL_OUT_FIELD_NAME;
extern const char * const tpl_SEL_OUT_FIELD_COMMA;
extern const char * const tpl_SEL_OUT_FIELD_INIT;
extern const char * const tpl_SEL_OUT_FIELD_GETVALUE;
extern const char * const tpl_SEL_OUT_FIELD_ISNULL;
extern const char * const tpl_SEL_OUT_FIELDS_BUFFERS;
extern const char * const tpl_SEL_OUT_FIELD_COMMENT;

extern const char * const tpl_DBENGINE_STATEMENT_TYPE;
extern const char * const tpl_DBENGINE_STATEMENT_NULL;

extern const char * const tpl_UPDATE;
extern const char * const tpl_UPDATE_SQL;
extern const char * const tpl_UPDATE_SQL_LEN;
extern const char * const tpl_UPDATE_FIELD_COUNT;
extern const char * const tpl_UPDATE_PARAM_COUNT;
extern const char * const tpl_UPD_IN_FIELDS;
extern const char * const tpl_UPD_IN_FIELD_TYPE;
extern const char * const tpl_UPD_IN_FIELD_NAME;
extern const char * const tpl_UPD_IN_FIELD_COMMA;
extern const char * const tpl_UPD_IN_FIELD_INIT;
extern const char * const tpl_UPD_IN_FIELD_BIND;
extern const char * const tpl_UPD_IN_FIELDS_BUFFERS;

extern const char * const tpl_INSERT;
extern const char * const tpl_INSERT_SQL;
extern const char * const tpl_INSERT_SQL_LEN;
extern const char * const tpl_INSERT_FIELD_COUNT;
extern const char * const tpl_INSERT_PARAM_COUNT;
extern const char * const tpl_INS_IN_FIELDS;
extern const char * const tpl_INS_IN_FIELD_TYPE;
extern const char * const tpl_INS_IN_FIELD_NAME;
extern const char * const tpl_INS_IN_FIELD_COMMA;
extern const char * const tpl_INS_IN_FIELD_INIT;
extern const char * const tpl_INS_IN_FIELD_BIND;
extern const char * const tpl_INS_IN_FIELDS_BUFFERS;

extern const char * const tpl_DELETE;
extern const char * const tpl_DELETE_SQL;
extern const char * const tpl_DELETE_SQL_LEN;
extern const char * const tpl_DELETE_FIELD_COUNT;
extern const char * const tpl_DELETE_PARAM_COUNT;
extern const char * const tpl_DEL_IN_FIELDS;
extern const char * const tpl_DEL_IN_FIELD_TYPE;
extern const char * const tpl_DEL_IN_FIELD_NAME;
extern const char * const tpl_DEL_IN_FIELD_COMMA;
extern const char * const tpl_DEL_IN_FIELD_INIT;
extern const char * const tpl_DEL_IN_FIELD_BIND;
extern const char * const tpl_DEL_IN_FIELDS_BUFFERS;

extern const char * const tpl_DBENGINE_CONNECT_PARAMS;
extern const char * const tpl_DBENGINE_CONNECT_PARAM_TYPE;
extern const char * const tpl_DBENGINE_CONNECT_PARAM_PARAM;
extern const char * const tpl_DBENGINE_CONNECT_PARAM_VALUE;
extern const char * const tpl_DBENGINE_CONNECT_PARAM_COMMA;

extern const char * const tpl_DBENGINE_CONNECT;
extern const char * const tpl_DBENGINE_PREPARE;
extern const char * const tpl_DBENGINE_CREATE_SELECT;
extern const char * const tpl_DBENGINE_PREPARE_SELECT;
extern const char * const tpl_DBENGINE_DESTROY_SELECT;
extern const char * const tpl_DBENGINE_DISCONNECT;
extern const char * const tpl_DBENGINE_RESET_SELECT;
extern const char * const tpl_DBENGINE_EXECUTE_SELECT;
extern const char * const tpl_DBENGINE_FETCH_SELECT;

}

#endif
