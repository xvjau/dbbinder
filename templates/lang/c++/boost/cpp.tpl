//
// C++ Implementation: {{IMPL_FILENAME}}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "{{INTF_FILENAME}}"

#ifdef NDEBUG
#define ASSERT(cond)
#define ASSERT_MSG(cond, msg) { if (!(cond)) { std::cerr << " WARNING: " << msg << std::endl; }}
#define ASSERT_MSG_FILE_LINE(cond, msg, file, line) { if (!(cond)) { std::cerr << " WARNING: " << msg << std::endl; }}
#define LOG_MSG(msg) { std::cerr << " WARNING: " << msg << std::endl; } while (false)
#else
#define ASSERT(cond) { assert(cond); }
#define ASSERT_MSG(cond, msg) { if (!(cond)) { std::cerr << __FILE__ << "." << __LINE__ << " WARNING: " << msg << std::endl; assert(cond); }}
#define ASSERT_MSG_FILE_LINE(cond, msg, file, line) { if (!(cond)) { std::cerr << file << "." << line << " WARNING: " << msg << std::endl; assert(cond); }}
#define LOG_MSG(msg) do { std::cerr << " WARNING: " << msg << std::endl; } while (false)
#endif

{{#NAMESPACES}}namespace {{NAMESPACE}} {
{{/NAMESPACES}}

{{#CLASS}}

{{#SELECT}}
const char * const {{CLASSNAME}}::s_selectSQL = {{STMT_SQL}};
const int {{CLASSNAME}}::s_selectSQL_len = {{STMT_SQL_LEN}};
{{/SELECT}}
{{#UPDATE}}
const char * const {{CLASSNAME}}::s_updateSQL = {{STMT_SQL}};
const int {{CLASSNAME}}::s_updateSQL_len = {{STMT_SQL_LEN}};
{{/UPDATE}}
{{#INSERT}}
const char * const {{CLASSNAME}}::s_insertSQL = {{STMT_SQL}};
const int {{CLASSNAME}}::s_insertSQL_len = {{STMT_SQL_LEN}};
{{/INSERT}}
{{#DELETE}}
const char * const {{CLASSNAME}}::s_deleteSQL = {{STMT_SQL}};
const int {{CLASSNAME}}::s_deleteSQL_len = {{STMT_SQL_LEN}};
{{/DELETE}}

{{CLASSNAME}}::{{CLASSNAME}}({{DBENGINE_CONNECTION_TYPE}} _conn):
        m_conn( _conn )
{{#DBENGINE_TRANSACTION}}
        ,m_tr( {{DBENGINE_TRANSACTION_NULL}} )
{{/DBENGINE_TRANSACTION}}
{{#SELECT}}
        ,m_selectStmt({{DBENGINE_STATEMENT_NULL}})
        ,m_selectIsActive( false )
        ,m_iterator(0)
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
    {{#STMT_IN_FIELDS_BUFFERS}}{{BUFFER_INITIALIZE}}
    {{/STMT_IN_FIELDS_BUFFERS}}

    {{#STMT_OUT_FIELDS_BUFFERS}}{{BUFFER_INITIALIZE}}
    {{/STMT_OUT_FIELDS_BUFFERS}}

    {{DBENGINE_CREATE_SELECT}}
    {{DBENGINE_PREPARE_SELECT}}
{{/SELECT}}
{{#UPDATE}}
    {{#STMT_IN_FIELDS_BUFFERS}}{{BUFFER_INITIALIZE}}
    {{/STMT_IN_FIELDS_BUFFERS}}

    {{DBENGINE_CREATE_UPDATE}}
    {{DBENGINE_PREPARE_UPDATE}}
{{/UPDATE}}
{{#INSERT}}
    {{#STMT_IN_FIELDS_BUFFERS}}{{BUFFER_INITIALIZE}}
    {{/STMT_IN_FIELDS_BUFFERS}}

    {{DBENGINE_CREATE_INSERT}}
    {{DBENGINE_PREPARE_INSERT}}
{{/INSERT}}
{{#DELETE}}
    {{#STMT_IN_FIELDS_BUFFERS}}{{BUFFER_INITIALIZE}}
    {{/STMT_IN_FIELDS_BUFFERS}}

    {{DBENGINE_CREATE_DELETE}}
    {{DBENGINE_PREPARE_DELETE}}
{{/DELETE}}
}

{{CLASSNAME}}::~{{CLASSNAME}}()
{
    {{#SELECT}}
    {{CLASSNAME}}::close();
    {{/SELECT}}

    {{#DBENGINE_TRANSACTION}}
    {{#UPDATE}}{{DBENGINE_TRANSACTION_COMMIT}}{{/UPDATE}}
    {{#INSERT}}{{DBENGINE_TRANSACTION_COMMIT}}{{/INSERT}}
    {{DBENGINE_TRANSACTION_COMMIT}}
    {{/DBENGINE_TRANSACTION}}


    {{#UPDATE}}
    {{DBENGINE_DESTROY_UPDATE}}
    {{#STMT_IN_FIELDS_BUFFERS}}{{BUFFER_FREE}}
    {{/STMT_IN_FIELDS_BUFFERS}}

    {{#STMT_OUT_FIELDS_BUFFERS}}{{BUFFER_FREE}}
    {{/STMT_OUT_FIELDS_BUFFERS}}
    {{/UPDATE}}

    {{#INSERT}}
    {{DBENGINE_DESTROY_INSERT}}
    {{#STMT_IN_FIELDS_BUFFERS}}{{BUFFER_FREE}}
    {{/STMT_IN_FIELDS_BUFFERS}}
    {{/INSERT}}

    {{#DELETE}}
    {{DBENGINE_DESTROY_DELETE}}
    {{#STMT_IN_FIELDS_BUFFERS}}{{BUFFER_FREE}}
    {{/STMT_IN_FIELDS_BUFFERS}}
    {{/DELETE}}
}

{{#SELECT}}
const int {{CLASSNAME}}::s_selectFieldCount = {{STMT_FIELD_COUNT}};
const int {{CLASSNAME}}::s_selectParamCount = {{STMT_PARAM_COUNT}};
{{CLASSNAME}}::iterator {{CLASSNAME}}::s_endIterator;

{{#STMT_HAS_PARAMS}}
{{CLASSNAME}}::{{CLASSNAME}}(
                            {{#STMT_IN_FIELDS}}{{STMT_IN_FIELD_TYPE}} _{{STMT_IN_FIELD_NAME}},
                            {{/STMT_IN_FIELDS}}
                            {{DBENGINE_CONNECTION_TYPE}} _conn
                            ):
        m_conn( _conn ),
{{#DBENGINE_TRANSACTION}}
        m_tr( {{DBENGINE_TRANSACTION_NULL}} ),
{{/DBENGINE_TRANSACTION}}
        m_selectStmt({{DBENGINE_STATEMENT_NULL}}),
        m_selectIsActive( false ),
        m_iterator( 0 )
{
    ASSERT_MSG(m_conn, "Connection must not be null!");

    {{DBENGINE_PREPARE}}

    {{#DBENGINE_TRANSACTION}}{{DBENGINE_TRANSACTION_INIT}}
    {{/DBENGINE_TRANSACTION}}

    {{#STMT_IN_FIELDS_BUFFERS}}{{BUFFER_INITIALIZE}}
    {{/STMT_IN_FIELDS_BUFFERS}}

    {{#STMT_OUT_FIELDS_BUFFERS}}{{BUFFER_INITIALIZE}}
    {{/STMT_OUT_FIELDS_BUFFERS}}

    {{DBENGINE_CREATE_SELECT}}
    {{DBENGINE_PREPARE_SELECT}}

    open(
        {{#STMT_IN_FIELDS}}_{{STMT_IN_FIELD_NAME}}{{STMT_IN_FIELD_COMMA}}
        {{/STMT_IN_FIELDS}}
        );
}
{{/STMT_HAS_PARAMS}}

void {{CLASSNAME}}::open(
                        {{#STMT_IN_FIELDS}}{{STMT_IN_FIELD_TYPE}} _{{STMT_IN_FIELD_NAME}}{{STMT_IN_FIELD_COMMA}}
                        {{/STMT_IN_FIELDS}}
                        )
{
    if ( m_selectIsActive )
    {
        {{DBENGINE_RESET_SELECT}}
    }

    {{#STMT_IN_FIELDS_BUFFERS}}{{BUFFER_ALLOC}}
    {{/STMT_IN_FIELDS_BUFFERS}}

    {{#STMT_OUT_FIELDS_BUFFERS}}{{BUFFER_ALLOC}}
    {{/STMT_OUT_FIELDS_BUFFERS}}

    {{#STMT_IN_FIELDS}}{{STMT_IN_FIELD_BIND}}
    {{/STMT_IN_FIELDS}}

    {{DBENGINE_EXECUTE_SELECT}}

    m_selectIsActive = true;
}

void {{CLASSNAME}}::close()
{
    if ( m_selectIsActive )
    {
        m_selectIsActive = false;

        {{#DBENGINE_TRANSACTION}}
        {{DBENGINE_TRANSACTION_ROLLBACK}}
        {{DBENGINE_TRANSACTION_COMMIT}}
        {{/DBENGINE_TRANSACTION}}

        {{DBENGINE_DESTROY_SELECT}}
        {{#STMT_IN_FIELDS_BUFFERS}}{{BUFFER_FREE}}
        {{/STMT_IN_FIELDS_BUFFERS}}
        {{#STMT_OUT_FIELDS_BUFFERS}}{{BUFFER_FREE}}
        {{/STMT_OUT_FIELDS_BUFFERS}}

        delete m_iterator;
        m_iterator = NULL;
    }
}

bool {{CLASSNAME}}::fetchRow()
{
    {{DBENGINE_FETCH_SELECT}}
}

{{CLASSNAME}}::iterator & {{CLASSNAME}}::begin()
{
    ASSERT_MSG(m_selectIsActive, "Select is not active.  Ensure open() was called.");

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
const int {{CLASSNAME}}::s_updateParamCount = {{STMT_PARAM_COUNT}};

void {{CLASSNAME}}::update(
            {{#STMT_IN_FIELDS}}{{STMT_IN_FIELD_TYPE}} _{{STMT_IN_FIELD_NAME}}{{STMT_IN_FIELD_COMMA}}
            {{/STMT_IN_FIELDS}})

{
    {{#STMT_IN_FIELDS_BUFFERS}}{{BUFFER_DECLARE}}
    {{BUFFER_ALLOC}}
    {{/STMT_IN_FIELDS_BUFFERS}}

    {{#STMT_OUT_FIELDS_BUFFERS}}{{BUFFER_ALLOC}}
    {{/STMT_OUT_FIELDS_BUFFERS}}

    {{#STMT_IN_FIELDS}}{{STMT_IN_FIELD_BIND}}
    {{/STMT_IN_FIELDS}}

    {{DBENGINE_EXECUTE_UPDATE}}
    {{DBENGINE_RESET_UPDATE}}
}
{{/UPDATE}}
{{#INSERT}}
const int {{CLASSNAME}}::s_insertParamCount = {{STMT_PARAM_COUNT}};

void {{CLASSNAME}}::insert(
            {{#STMT_IN_FIELDS}}{{STMT_IN_FIELD_TYPE}} _{{STMT_IN_FIELD_NAME}}{{STMT_IN_FIELD_COMMA}}
            {{/STMT_IN_FIELDS}})
{
    {{#STMT_IN_FIELDS_BUFFERS}}{{BUFFER_DECLARE}}
    {{BUFFER_ALLOC}}
    {{/STMT_IN_FIELDS_BUFFERS}}

    {{#STMT_IN_FIELDS}}{{STMT_IN_FIELD_BIND}}
    {{/STMT_IN_FIELDS}}

    {{DBENGINE_EXECUTE_INSERT}}
    {{DBENGINE_RESET_INSERT}}
}
{{/INSERT}}
{{#DELETE}}
const int {{CLASSNAME}}::s_deleteParamCount = {{STMT_PARAM_COUNT}};

void {{CLASSNAME}}::del(
            {{#STMT_IN_FIELDS}}{{STMT_IN_FIELD_TYPE}} _{{STMT_IN_FIELD_NAME}}{{STMT_IN_FIELD_COMMA}}
            {{/STMT_IN_FIELDS}})
{
    {{#STMT_IN_FIELDS_BUFFERS}}{{BUFFER_DECLARE}}
    {{BUFFER_ALLOC}}
    {{/STMT_IN_FIELDS_BUFFERS}}

    {{#STMT_IN_FIELDS}}{{STMT_IN_FIELD_BIND}}
    {{/STMT_IN_FIELDS}}

    {{DBENGINE_EXECUTE_DELETE}}
    {{DBENGINE_RESET_DELETE}}
}
{{/DELETE}}
{{/CLASS}}

{{#NAMESPACES}}
} // {{NAMESPACE}}
{{/NAMESPACES}}
