//
// C++ Interface: {{INTF_FILENAME}}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef __INCLUDE_{{HEADER_NAME}}
#define __INCLUDE_{{HEADER_NAME}}

#include <boost/shared_ptr.hpp>
#include <iostream>
#include <string.h>

#ifdef DEBUG
#define ASSERT(cond) { assert(cond); }
#define ASSERT_MSG(cond, msg) { if (!(cond)) { std::cerr << __FILE__ << "." << __LINE__ << " WARNING: " << msg << std::endl; assert(cond); }}
#define ASSERT_MSG_FILE_LINE(cond, msg, file, line) { if (!(cond)) { std::cerr << file << "." << line << " WARNING: " << msg << std::endl; assert(cond); }}
#else
#define ASSERT(cond)
#define ASSERT_MSG(cond, msg) { if (!(cond)) { std::cerr << " WARNING: " << msg << std::endl; }}
#define ASSERT_MSG_FILE_LINE(cond, msg, file, line) { if (!(cond)) { std::cerr << " WARNING: " << msg << std::endl; }}
#endif

{{#DBENGINE_INCLUDES}}
{{DBENGINE_INCLUDE_NAME}}
{{/DBENGINE_INCLUDES}}
{{#EXTRA_HEADERS}}{{EXTRA_HEADERS_HEADER}}
{{/EXTRA_HEADERS}}


{{#NAMESPACES}}namespace {{NAMESPACE}} {
{{/NAMESPACES}}

{{#EXTRA_HEADERS}}{{EXTRA_HEADERS_TYPE}}
{{/EXTRA_HEADERS}}

{{#CLASS}}
class {{CLASSNAME}}
{
	public:
		{{CLASSNAME}}({{DBENGINE_CONNECTION_TYPE}} _conn = {{DBENGINE_CONNECTION_NULL}});
	private:
		{{DBENGINE_CONNECTION_TYPE}}	m_conn;
		bool							m_needCloseConn;
		{{#EXTRA_HEADERS}}{{EXTRA_HEADERS_MEMBER}}
		{{/EXTRA_HEADERS}}
		{{#DBENGINE_EXTRAS}}{{DBENGINE_EXTRA_VAR}}
		{{/DBENGINE_EXTRAS}}
{{#SELECT}}
	public:
		{{CLASSNAME}}(
						{{#SEL_IN_FIELDS}}{{SEL_IN_FIELD_TYPE}} _{{SEL_IN_FIELD_NAME}},
						{{/SEL_IN_FIELDS}}
						{{DBENGINE_CONNECTION_TYPE}} _conn = {{DBENGINE_CONNECTION_NULL}});
		~{{CLASSNAME}}();

	private:
		static const char* const s_selectSQL;
		static const int s_selectSQL_len;
		static const int s_selectFieldCount;
		static const int s_selectParamCount;

		{{DBENGINE_STATEMENT_TYPE}} m_selectStmt;
		bool						m_selectIsActive;

		bool fetchRow();

		{{#SEL_IN_FIELDS_BUFFERS}}{{BUFFER_DECLARE}}
		{{/SEL_IN_FIELDS_BUFFERS}}

		{{#SEL_OUT_FIELDS_BUFFERS}}{{BUFFER_DECLARE}}
		{{/SEL_OUT_FIELDS_BUFFERS}}
	public:
		void open(
			{{#SEL_IN_FIELDS}}{{SEL_IN_FIELD_TYPE}} _{{SEL_IN_FIELD_NAME}}{{SEL_IN_FIELD_COMMA}}
			{{/SEL_IN_FIELDS}}
				 );

		class _row_type
		{
			friend class {{CLASSNAME}};

			private:
				_row_type():
				{{#SEL_OUT_FIELDS}}	m_{{SEL_OUT_FIELD_NAME}}({{SEL_OUT_FIELD_INIT}}){{SEL_OUT_FIELD_COMMA}}
				{{/SEL_OUT_FIELDS}}
				{}

				_row_type(const {{CLASSNAME}} *_parent)
				{
					{{#SEL_OUT_FIELDS}}{{SEL_OUT_FIELD_GETVALUE}}
					{{/SEL_OUT_FIELDS}}
				}

				{{#SEL_OUT_FIELDS}}{{SEL_OUT_FIELD_TYPE}} m_{{SEL_OUT_FIELD_NAME}};
				{{/SEL_OUT_FIELDS}}

			public:
				{{#SEL_OUT_FIELDS}}{{SEL_OUT_FIELD_TYPE}} get{{SEL_OUT_FIELD_NAME}}() const
				{
					return m_{{SEL_OUT_FIELD_NAME}};
				}
				{{/SEL_OUT_FIELDS}}
		};
		typedef boost::shared_ptr<_row_type> row;

		class iterator
		{
			friend class {{CLASSNAME}};

			private:
				iterator({{CLASSNAME}}* _parent):
					m_parent( _parent )
				{}

				{{CLASSNAME}} *m_parent;

			public:
				const row& operator*() const
				{
					ASSERT_MSG( m_parent, "Called operator* without parent/after end." );
					return m_parent->m_currentRow;
				}

				const row& operator->() const
				{
					ASSERT_MSG( m_parent, "Called operator-> without parent/after end." );
					return m_parent->m_currentRow;
				}

				void operator++()
				{
					ASSERT_MSG( m_parent, "Called operator++ without parent/after end." );
					if ( !m_parent->fetchRow() )
						m_parent = 0;
				}

				bool operator==(const iterator& _other) const
				{
					return m_parent == _other.m_parent;
				}

				bool operator!=(const iterator& _other) const
				{
					return m_parent != _other.m_parent;
				}

				typedef iterator iterator_category;
				typedef row value_type;
				typedef bool difference_type;
				typedef _row_type* pointer;
				typedef row reference;
		};
		typedef iterator const_iterator;

		iterator& begin();
		iterator& end()
		{
			return *m_endIterator;
		}

	private:
		row 		m_currentRow;
		iterator	*m_iterator;
		iterator	*m_endIterator;
{{/SELECT}}
{{#UPDATE}}
	private:
		static const char* const s_updateSQL;
		{{DBENGINE_STATEMENT_TYPE}}	m_updateStmt;
		bool						m_updateIsActive;
	public:
		bool update(
					{{#UPD_IN_FIELDS}}{{UPD_IN_FIELD_TYPE}} _{{UPD_IN_FIELD_NAME}}{{UPD_IN_FIELD_COMMA}}
					{{/UPD_IN_FIELDS}});
{{/UPDATE}}
{{#INSERT}}
	private:
		static const char* const s_insertSQL;
		{{DBENGINE_STATEMENT_TYPE}}	m_insertStmt;
	public:
		bool insert(
					{{#INS_IN_FIELDS}}{{INS_IN_FIELD_TYPE}} _{{INS_IN_FIELD_NAME}}{{INS_IN_FIELD_COMMA}}
					{{/INS_IN_FIELDS}});
{{/INSERT}}
};
{{/CLASS}}

{{#NAMESPACES}}
} // {{NAMESPACE}}
{{/NAMESPACES}}

#undef ASSERT
#undef ASSERT_MSG
#undef ASSERT_MSG_FILE_LINE

#endif
