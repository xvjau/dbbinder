//
// C++ Implementation: {{IMPL_FILENAME}}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "{{INTF_FILENAME}}"

{{#NAMESPACES}}namespace {{NAMESPACE}} {
{{/NAMESPACES}}

{{#DBENGINE_GLOBAL_PARAMS}}static {{TYPE}} s_dbparam_{{PARAM}} = {{VALUE}};
{{/DBENGINE_GLOBAL_PARAMS}}

{{#CLASS}}
{{#SELECT}}
const char * const {{CLASSNAME}}::s_selectSQL = {{SELECT_SQL}};

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
{{/UPDATE}}
{{#INSERT}}
const char * const {{CLASSNAME}}::s_insertSQL = {{INSERT_SQL}};
{{/INSERT}}
{{/CLASS}}

{{#NAMESPACES}}
} // {{NAMESPACE}}
{{/NAMESPACES}}
