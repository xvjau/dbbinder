//
// C++ Interface: {{FILENAME}}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef __INCLUDE_DBBINDER_DB_H
#define __INCLUDE_DBBINDER_DB_H

#include <boost/shared_ptr.hpp>
#include <iostream>
#include <string.h>
#include <libgen.h>

{{#DBENGINE_INCLUDES}}{{DBENGINE_INCLUDE_NAME}}
{{/DBENGINE_INCLUDES}}
{{#EXTRA_HEADERS}}{{EXTRA_HEADERS_HEADER}}
{{/EXTRA_HEADERS}}

{{#NAMESPACES}}namespace {{NAMESPACE}} {
{{/NAMESPACES}}

namespace DBBinder
{

class Connection
{
	private:
		{{DBENGINE_CONNECTION_TYPE}} m_conn;

	public:
		Connection({{#DBENGINE_CONNECT_PARAMS}} {{DBENGINE_CONNECT_PARAM_TYPE}} _{{DBENGINE_CONNECT_PARAM_PARAM}}{{DBENGINE_CONNECT_PARAM_COMMA}}{{/DBENGINE_CONNECT_PARAMS}});
		~Connection();
		
		operator {{DBENGINE_CONNECTION_TYPE}}()
		{
			return m_conn;
		}
};
}

typedef boost::shared_ptr<DBBinder::Connection> Connection;


{{#NAMESPACES}}} // {{NAMESPACE}}
{{/NAMESPACES}}

#endif // __INCLUDE_DBBINDER_DB_H
