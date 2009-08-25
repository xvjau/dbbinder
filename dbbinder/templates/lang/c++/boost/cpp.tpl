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

{{#DBENGINE_GLOBAL_FUNCTIONS}}
{{FUNCTION}}
{{/DBENGINE_GLOBAL_FUNCTIONS}}

{{#CLASS}}

{{#SELECT}}
const char * const {{CLASSNAME}}::s_selectSQL = {{SELECT_SQL}};
const int {{CLASSNAME}}::s_selectSQL_len = {{SELECT_SQL_LEN}};
{{/SELECT}}
{{#UPDATE}}
const char * const {{CLASSNAME}}::s_selectSQL = {{UPDATE_SQL}};
const int {{CLASSNAME}}::s_updateSQL_len = {{UPDATE_SQL_LEN}};
{{/UPDATE}}
{{#INSERT}}
const char * const {{CLASSNAME}}::s_selectSQL = {{INSERT_SQL}};
const int {{CLASSNAME}}::s_insertSQL_len = {{INSERT_SQL_LEN}};
{{/INSERT}}

{{CLASSNAME}}::{{CLASSNAME}}({{DBENGINE_CONNECTION_TYPE}} _conn):
		m_conn( _conn )
{{#SELECT}}
		,m_selectIsActive( false )
		,m_iterator(0)
{{/SELECT}}
{
	ASSERT_MSG(m_conn, "Connection must not be null!");

	{{DBENGINE_PREPARE}}

{{#SELECT}}
	{{#SEL_IN_FIELDS_BUFFERS}}{{BUFFER_INITIALIZE}}
	{{/SEL_IN_FIELDS_BUFFERS}}

	{{#SEL_OUT_FIELDS_BUFFERS}}{{BUFFER_INITIALIZE}}
	{{/SEL_OUT_FIELDS_BUFFERS}}

	{{DBENGINE_CREATE_SELECT}}
	{{DBENGINE_PREPARE_SELECT}}
{{/SELECT}}
{{#UPDATE}}
	{{#UPD_IN_FIELDS_BUFFERS}}{{BUFFER_INITIALIZE}}
	{{/UPD_IN_FIELDS_BUFFERS}}

	{{DBENGINE_CREATE_UPDATE}}
	{{DBENGINE_PREPARE_UPDATE}}
{{/UPDATE}}
{{#INSERT}}
	{{#INS_IN_FIELDS_BUFFERS}}{{BUFFER_INITIALIZE}}
	{{/INS_IN_FIELDS_BUFFERS}}

	{{DBENGINE_CREATE_INSERT}}
	{{DBENGINE_PREPARE_INSERT}}
{{/INSERT}}
}

{{CLASSNAME}}::~{{CLASSNAME}}()
{
	{{#SELECT}}{{DBENGINE_DESTROY_SELECT}}{{/SELECT}}
	{{#UPDATE}}{{DBENGINE_DESTROY_UPDATE}}{{/UPDATE}}
	{{#INSERT}}{{DBENGINE_DESTROY_INSERT}}{{/INSERT}}
}

{{#SELECT}}
const int {{CLASSNAME}}::s_selectFieldCount = {{SELECT_FIELD_COUNT}};
const int {{CLASSNAME}}::s_selectParamCount = {{SELECT_PARAM_COUNT}};
{{CLASSNAME}}::iterator {{CLASSNAME}}::s_endIterator(0);

{{#SELECT_HAS_PARAMS}}
{{CLASSNAME}}::{{CLASSNAME}}(
							 {{#SEL_IN_FIELDS}}{{SEL_IN_FIELD_TYPE}} _{{SEL_IN_FIELD_NAME}},
							 {{/SEL_IN_FIELDS}}
							 {{DBENGINE_CONNECTION_TYPE}} _conn
							):
		m_iterator(0), m_conn( _conn ), m_selectIsActive( false )
{
	ASSERT_MSG(m_conn, "Connection must not be null!");

	{{DBENGINE_PREPARE}}

	{{#SEL_IN_FIELDS_BUFFERS}}{{BUFFER_INITIALIZE}}
	{{/SEL_IN_FIELDS_BUFFERS}}

	{{#SEL_OUT_FIELDS_BUFFERS}}{{BUFFER_INITIALIZE}}
	{{/SEL_OUT_FIELDS_BUFFERS}}

	{{DBENGINE_CREATE_SELECT}}
	{{DBENGINE_PREPARE_SELECT}}

	open(
		 {{#SEL_IN_FIELDS}}_{{SEL_IN_FIELD_NAME}}{{SEL_IN_FIELD_COMMA}}
		 {{/SEL_IN_FIELDS}}
		);
}
{{/SELECT_HAS_PARAMS}}

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
	if ( m_iterator )
		return *m_iterator;

	if ( fetchRow() )
	{
		m_iterator = new {{CLASSNAME}}::iterator(this);
		return *m_iterator;
	}
	else
		return s_endIterator;
}
{{/SELECT}}
{{#UPDATE}}
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
bool {{CLASSNAME}}::insert(
			{{#INS_IN_FIELDS}}{{INS_IN_FIELD_TYPE}} _{{INS_IN_FIELD_NAME}}{{INS_IN_FIELD_COMMA}}
			{{/INS_IN_FIELDS}})
{
	{{#INS_IN_FIELDS_BUFFERS}}{{BUFFER_ALLOC}}
	{{/INS_IN_FIELDS_BUFFERS}}

	{{#INS_IN_FIELDS}}{{INS_IN_FIELD_BIND}}
	{{/INS_IN_FIELDS}}

	{{DBENGINE_EXECUTE_INSERT}}
}
{{/INSERT}}
{{/CLASS}}

{{#NAMESPACES}}
} // {{NAMESPACE}}
{{/NAMESPACES}}
