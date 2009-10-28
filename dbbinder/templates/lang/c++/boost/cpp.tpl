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
const char * const {{CLASSNAME}}::s_updateSQL = {{UPDATE_SQL}};
const int {{CLASSNAME}}::s_updateSQL_len = {{UPDATE_SQL_LEN}};
{{/UPDATE}}
{{#INSERT}}
const char * const {{CLASSNAME}}::s_insertSQL = {{INSERT_SQL}};
const int {{CLASSNAME}}::s_insertSQL_len = {{INSERT_SQL_LEN}};
{{/INSERT}}
{{#DELETE}}
const char * const {{CLASSNAME}}::s_deleteSQL = {{DELETE_SQL}};
const int {{CLASSNAME}}::s_deleteSQL_len = {{DELETE_SQL_LEN}};
{{/DELETE}}

{{CLASSNAME}}::{{CLASSNAME}}({{DBENGINE_CONNECTION_TYPE}} _conn):
		m_conn( _conn )
{{#DBENGINE_TRANSACTION}}
		,m_tr( {{DBENGINE_TRANSACTION_NULL}} )
{{/DBENGINE_TRANSACTION}}
{{#SELECT}}
		,m_selectIsActive( false )
		,m_iterator(0)
		,m_selectStmt({{DBENGINE_STATEMENT_NULL}})
{{/SELECT}}
{{#UPDATE}}
		,m_updateStmt({{DBENGINE_STATEMENT_NULL}})
{{/UPDATE}}
{{#INSERT}}
		,m_insertStmt({{DBENGINE_STATEMENT_NULL}})
{{/INSERT}}
{{#DELETE}}
		,m_deleteStmt({{DBENGINE_STATEMENT_NULL}})
{{/DELETE}}
{
	ASSERT_MSG(m_conn, "Connection must not be null!");

	{{DBENGINE_PREPARE}}

	{{#DBENGINE_TRANSACTION}}{{DBENGINE_TRANSACTION_INIT}}
	{{/DBENGINE_TRANSACTION}}

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
{{#DELETE}}
	{{#DEL_IN_FIELDS_BUFFERS}}{{BUFFER_INITIALIZE}}
	{{/DEL_IN_FIELDS_BUFFERS}}

	{{DBENGINE_CREATE_DELETE}}
	{{DBENGINE_PREPARE_DELETE}}
{{/DELETE}}
}

{{CLASSNAME}}::~{{CLASSNAME}}()
{
	{{#DBENGINE_TRANSACTION}}
	{{#SELECT}}{{DBENGINE_TRANSACTION_ROLLBACK}}{{/SELECT}}
	{{#UPDATE}}{{DBENGINE_TRANSACTION_COMMIT}}{{/UPDATE}}
	{{#INSERT}}{{DBENGINE_TRANSACTION_COMMIT}}{{/INSERT}}
	{{DBENGINE_TRANSACTION_COMMIT}}
	{{/DBENGINE_TRANSACTION}}

	{{#SELECT}}
	{{DBENGINE_DESTROY_SELECT}}
	{{#SEL_IN_FIELDS_BUFFERS}}{{BUFFER_FREE}}
	{{/SEL_IN_FIELDS_BUFFERS}}
	{{#SEL_OUT_FIELDS_BUFFERS}}{{BUFFER_FREE}}
	{{/SEL_OUT_FIELDS_BUFFERS}}
	{{/SELECT}}

	{{#UPDATE}}
	{{DBENGINE_DESTROY_UPDATE}}
	{{#UPD_IN_FIELDS_BUFFERS}}{{BUFFER_FREE}}
	{{/UPD_IN_FIELDS_BUFFERS}}

	{{#UPD_OUT_FIELDS_BUFFERS}}{{BUFFER_FREE}}
	{{/UPD_OUT_FIELDS_BUFFERS}}
	{{/UPDATE}}

	{{#INSERT}}
	{{DBENGINE_DESTROY_INSERT}}
	{{#INS_IN_FIELDS_BUFFERS}}{{BUFFER_FREE}}
	{{/INS_IN_FIELDS_BUFFERS}}
	{{/INSERT}}

	{{#DELETE}}
	{{DBENGINE_DESTROY_DELETE}}
	{{#DEL_IN_FIELDS_BUFFERS}}{{BUFFER_FREE}}
	{{/DEL_IN_FIELDS_BUFFERS}}
	{{/DELETE}}
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
		m_conn( _conn ),
{{#DBENGINE_TRANSACTION}}
		m_tr( {{DBENGINE_TRANSACTION_NULL}} ),
{{/DBENGINE_TRANSACTION}}
		m_iterator( 0 ),
		m_selectStmt({{DBENGINE_STATEMENT_NULL}}),
		m_selectIsActive( false )
{
	ASSERT_MSG(m_conn, "Connection must not be null!");

	{{DBENGINE_PREPARE}}

	{{#DBENGINE_TRANSACTION}}{{DBENGINE_TRANSACTION_INIT}}
	{{/DBENGINE_TRANSACTION}}

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
const int {{CLASSNAME}}::s_updateParamCount = {{UPDATE_PARAM_COUNT}};

void {{CLASSNAME}}::update(
			{{#UPD_IN_FIELDS}}{{UPD_IN_FIELD_TYPE}} _{{UPD_IN_FIELD_NAME}}{{UPD_IN_FIELD_COMMA}}
			{{/UPD_IN_FIELDS}})

{
	{{#UPD_IN_FIELDS_BUFFERS}}{{BUFFER_ALLOC}}
	{{/UPD_IN_FIELDS_BUFFERS}}

	{{#UPD_OUT_FIELDS_BUFFERS}}{{BUFFER_ALLOC}}
	{{/UPD_OUT_FIELDS_BUFFERS}}

	{{#UPD_IN_FIELDS}}{{UPD_IN_FIELD_BIND}}
	{{/UPD_IN_FIELDS}}

	{{DBENGINE_EXECUTE_UPDATE}}
	{{DBENGINE_RESET_UPDATE}}
}
{{/UPDATE}}
{{#INSERT}}
const int {{CLASSNAME}}::s_insertParamCount = {{INSERT_PARAM_COUNT}};

void {{CLASSNAME}}::insert(
			{{#INS_IN_FIELDS}}{{INS_IN_FIELD_TYPE}} _{{INS_IN_FIELD_NAME}}{{INS_IN_FIELD_COMMA}}
			{{/INS_IN_FIELDS}})
{
	{{#INS_IN_FIELDS_BUFFERS}}{{BUFFER_ALLOC}}
	{{/INS_IN_FIELDS_BUFFERS}}

	{{#INS_IN_FIELDS}}{{INS_IN_FIELD_BIND}}
	{{/INS_IN_FIELDS}}

	{{DBENGINE_EXECUTE_INSERT}}
	{{DBENGINE_RESET_INSERT}}
}
{{/INSERT}}
{{#DELETE}}
const int {{CLASSNAME}}::s_deleteParamCount = {{DELETE_PARAM_COUNT}};

void {{CLASSNAME}}::del(
			{{#DEL_IN_FIELDS}}{{DEL_IN_FIELD_TYPE}} _{{DEL_IN_FIELD_NAME}}{{DEL_IN_FIELD_COMMA}}
			{{/DEL_IN_FIELDS}})
{
	{{#DEL_IN_FIELDS_BUFFERS}}{{BUFFER_ALLOC}}
	{{/DEL_IN_FIELDS_BUFFERS}}

	{{#DEL_IN_FIELDS}}{{DEL_IN_FIELD_BIND}}
	{{/DEL_IN_FIELDS}}

	{{DBENGINE_EXECUTE_DELETE}}
	{{DBENGINE_RESET_DELETE}}
}
{{/DELETE}}
{{/CLASS}}

{{#NAMESPACES}}
} // {{NAMESPACE}}
{{/NAMESPACES}}
