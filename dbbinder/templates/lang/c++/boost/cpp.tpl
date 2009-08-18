//
// C++ Implementation: {{IMPL_FILENAME}}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "{{INTF_FILENAME}}"

#ifdef DEBUG
#define ASSERT(cond) { assert(cond); }
#define ASSERT_MSG(cond, msg) { if (!(cond)) { std::cerr << __FILE__ << "." << __LINE__ << " WARNING: " << msg << std::endl; assert(cond); }}
#define ASSERT_MSG_FILE_LINE(cond, msg, file, line) { if (!(cond)) { std::cerr << file << "." << line << " WARNING: " << msg << std::endl; assert(cond); }}
#else
#define ASSERT(cond)
#define ASSERT_MSG(cond, msg) { if (!(cond)) { std::cerr << " WARNING: " << msg << std::endl; }}
#define ASSERT_MSG_FILE_LINE(cond, msg, file, line) { if (!(cond)) { std::cerr << " WARNING: " << msg << std::endl; }}
#endif

{{#NAMESPACES}}namespace {{NAMESPACE}} {
{{/NAMESPACES}}

{{#DBENGINE_GLOBAL_PARAMS}}static {{TYPE}} s_dbparam_{{PARAM}} = {{VALUE}};
{{/DBENGINE_GLOBAL_PARAMS}}

{{#DBENGINE_GLOBAL_FUNCTIONS}}
{{FUNCTION}}
{{/DBENGINE_GLOBAL_FUNCTIONS}}

{{#CLASS}}
{{#SELECT}}
const char * const {{CLASSNAME}}::s_selectSQL = {{SELECT_SQL}};
const int {{CLASSNAME}}::s_selectSQL_len = {{SELECT_SQL_LEN}};
const int {{CLASSNAME}}::s_selectFieldCount = {{SELECT_FIELD_COUNT}};
const int {{CLASSNAME}}::s_selectParamCount = {{SELECT_PARAM_COUNT}};

{{CLASSNAME}}::{{CLASSNAME}}({{DBENGINE_CONNECTION_TYPE}} _conn):
		m_conn( _conn ), m_needCloseConn( false ), m_selectIsActive( false ), m_iterator(0)
{
	if ( !m_conn )
	{
		m_needCloseConn = true;
		{{DBENGINE_CONNECT}}
	}

	{{DBENGINE_PREPARE}}
}

{{CLASSNAME}}::{{CLASSNAME}}(
							 {{#SEL_IN_FIELDS}}{{SEL_IN_FIELD_TYPE}} _{{SEL_IN_FIELD_NAME}},
							 {{/SEL_IN_FIELDS}}
							 {{DBENGINE_CONNECTION_TYPE}} _conn
							):
		m_iterator(0), m_conn( _conn ), m_selectIsActive( false ), m_needCloseConn( false )
{
	{{#SEL_IN_FIELDS_BUFFERS}}{{BUFFER_INITIALIZE}}
	{{/SEL_IN_FIELDS_BUFFERS}}

	{{#SEL_OUT_FIELDS_BUFFERS}}{{BUFFER_INITIALIZE}}
	{{/SEL_OUT_FIELDS_BUFFERS}}

	if ( !m_conn )
	{
		m_needCloseConn = true;
		{{DBENGINE_CONNECT}}
	}

	{{DBENGINE_CREATE_SELECT}}
	{{DBENGINE_PREPARE_SELECT}}

	open(
		 {{#SEL_IN_FIELDS}}_{{SEL_IN_FIELD_NAME}}{{SEL_IN_FIELD_COMMA}}
		 {{/SEL_IN_FIELDS}}
		);
}

{{CLASSNAME}}::~{{CLASSNAME}}()
{
	{{DBENGINE_DESTROY_SELECT}}
	if ( m_needCloseConn )
	{
		{{DBENGINE_DISCONNECT}}
	}
}

void {{CLASSNAME}}::open(
						 {{#SEL_IN_FIELDS}}{{SEL_IN_FIELD_TYPE}} _{{SEL_IN_FIELD_NAME}}{{SEL_IN_FIELD_COMMA}}
						 {{/SEL_IN_FIELDS}}
						)
{
	if ( m_selectIsActive )
	{
		{{DBENGINE_RESET_SELECT}}
	}

	{{#SEL_IN_FIELDS_BUFFERS}}{{BUFFER_ALLOC}}
	{{/SEL_IN_FIELDS_BUFFERS}}

	{{#SEL_OUT_FIELDS_BUFFERS}}{{BUFFER_ALLOC}}
	{{/SEL_OUT_FIELDS_BUFFERS}}

	{{#SEL_IN_FIELDS}}{{SEL_IN_FIELD_BIND}}
	{{/SEL_IN_FIELDS}}

	{{DBENGINE_EXECUTE_SELECT}}

	m_selectIsActive = true;
}

bool {{CLASSNAME}}::fetchRow()
{
	{{DBENGINE_FETCH_SELECT}}
}

{{CLASSNAME}}::iterator & {{CLASSNAME}}::begin()
{
	m_endIterator = new {{CLASSNAME}}::iterator(0);

	if ( fetchRow() )
	{
		if ( !m_iterator )
			m_iterator = new {{CLASSNAME}}::iterator(this);

		return *m_iterator;
	}
	else
		return *m_endIterator;
}
{{/SELECT}}
{{#UPDATE}}
const char * const {{CLASSNAME}}::s_updateSQL = {{UPDATE_SQL}};

bool {{CLASSNAME}}::update(
			{{#UPD_IN_FIELDS}}{{UPD_IN_FIELD_TYPE}} _{{UPD_IN_FIELD_NAME}}{{UPD_IN_FIELD_COMMA}}
			{{/UPD_IN_FIELDS}})

{
	if ( m_updateIsActive )
	{
		{{DBENGINE_RESET_UPDATE}}
	}

	{{#UPD_IN_FIELDS_BUFFERS}}{{BUFFER_ALLOC}}
	{{/UPD_IN_FIELDS_BUFFERS}}

	{{#UPD_OUT_FIELDS_BUFFERS}}{{BUFFER_ALLOC}}
	{{/UPD_OUT_FIELDS_BUFFERS}}

	{{#UPD_IN_FIELDS}}{{UPD_IN_FIELD_BIND}}
	{{/UPD_IN_FIELDS}}

	{{DBENGINE_EXECUTE_UPDATE}}

	m_updateIsActive = true;
}
{{/UPDATE}}
{{#INSERT}}
const char * const {{CLASSNAME}}::s_insertSQL = {{INSERT_SQL}};

bool {{CLASSNAME}}::insert(
			{{#INS_IN_FIELDS}}{{INS_IN_FIELD_TYPE}} _{{INS_IN_FIELD_NAME}}{{INS_IN_FIELD_COMMA}}
			{{/INS_IN_FIELDS}})
{
}
{{/INSERT}}
{{/CLASS}}

{{#NAMESPACES}}
} // {{NAMESPACE}}
{{/NAMESPACES}}
