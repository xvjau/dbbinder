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

#include "oraclegenerator.h"

namespace DBBinder
{

#define oraCheckErr(ERROR, STATUS) oraCheckErrFn(ERROR, STATUS, __FILE__, __LINE__)
void oraCheckErrFn ( OCIError *_err, sword _status, const char* _file, unsigned int _line )
{
	switch ( _status )
	{
		case OCI_SUCCESS:
			break;
		case OCI_SUCCESS_WITH_INFO: FATAL("OCI_SUCCESS_WITH_INFO");
		case OCI_NEED_DATA: FATAL("OCI_NEED_DATA");
		case OCI_NO_DATA: FATAL("OCI_NODATA");
		case OCI_ERROR:
		{
			text errbuf[2048];
			sb4 errcode;
			
			OCIErrorGet ( _err, ( ub4 ) 1, ( text * ) NULL, &errcode,
			                       errbuf, ( ub4 ) sizeof ( errbuf ), OCI_HTYPE_ERROR );
			
			FATAL( errbuf );
		}
		case OCI_INVALID_HANDLE: FATAL("OCI_INVALID_HANDLE");
		case OCI_STILL_EXECUTING: FATAL("OCI_STILL_EXECUTE");
		default: FATAL("UNKNOWN ERROR: " << _status);
	}
}
	
OracleGenerator::OracleGenerator()
 : AbstractGenerator(), m_env(0), m_err(0), m_srv(0), m_svc(0), m_auth(0)
{
	m_dbengine = "oracle";
}

OracleGenerator::~OracleGenerator()
{
	if ( m_connected )
	{
		/* Disconnect */
		oraCheckErr( m_err, OCILogoff ( m_svc, m_err ));
		
		/* Free handles */
		OCIHandleFree ( ( dvoid * ) m_svc, OCI_HTYPE_SVCCTX );
		OCIHandleFree ( ( dvoid * ) m_err, OCI_HTYPE_ERROR );
	}
}

bool OracleGenerator::checkConnection()
{
	if ( !m_connected )
	{
		String dbName = m_dbParams["db"].value;
		String user = m_dbParams["user"].value;
		String pass = m_dbParams["password"].value;
	
		int errcode = OCIEnvCreate ( &m_env, OCI_DEFAULT,
		                             0, ( dvoid * ( * ) ( dvoid *,size_t ) ) 0,
		                             ( dvoid * ( * ) ( dvoid *, dvoid *, size_t ) ) 0,
		                             ( void ( * ) ( dvoid *, dvoid * ) ) 0, 0, 0 );
		if ( errcode != 0 )
		{
			std::cerr << "OCIEnvCreate failed with errcode = " << errcode << std::endl;
			exit ( 1 );
		}
	
		/* Initialize handles */
		OCIHandleAlloc( m_env, (dvoid**)&m_err, OCI_HTYPE_ERROR, 0, 0 );
		OCIHandleAlloc( m_env, (dvoid**)&m_srv, OCI_HTYPE_SERVER, 0, 0 );
		OCIHandleAlloc( m_env, (dvoid**)&m_svc, OCI_HTYPE_SVCCTX, 0, 0 );
		OCIHandleAlloc( m_env, (dvoid**)&m_auth, OCI_HTYPE_SESSION, 0, 0 );

		/* Connect/Attach to the Server */
		oraCheckErr ( m_err, OCIServerAttach( m_srv, m_err, (text *) dbName.c_str(), dbName.length(), 0) );
		
		/* set username and password */
		oraCheckErr ( m_err, OCIAttrSet ( m_svc, OCI_HTYPE_SVCCTX, m_srv, 0, OCI_ATTR_SERVER, m_err ) );
		
		oraCheckErr ( m_err, OCIAttrSet ( m_auth, OCI_HTYPE_SESSION,
		                                  const_cast<char*>(user.c_str()), user.length(),
		                                  OCI_ATTR_USERNAME, m_err ) );
		
		oraCheckErr ( m_err, OCIAttrSet ( m_auth, OCI_HTYPE_SESSION,
		                                  const_cast<char*>(pass.c_str()), pass.length(),
		                                  OCI_ATTR_PASSWORD, m_err ));
		
		/* Authenticate / Start session */
		int ret = OCISessionBegin ( m_svc, m_err, m_auth, OCI_CRED_RDBMS, OCI_DEFAULT);
		oraCheckErr ( m_err, ret );
	
		m_connected = ret == OCI_SUCCESS;

		oraCheckErr ( m_err, OCIAttrSet((dvoid *)m_svc, OCI_HTYPE_SVCCTX,
                   (dvoid *)m_auth, 0, OCI_ATTR_SESSION, m_err));
	}
	
	return m_connected;
}

String OracleGenerator::getBind(const ListElements::iterator & _item, int _index)
{
	std::stringstream result;

	result <<
			"oraCheckErr( m_conn->err, OCIBindByName ( m_selectStmt, &m_bnd" << _item->name << ", m_conn->err, ( text * ) :" << _item->name
			<< ", -1, ( dvoid * ) &_" << _item->name << ", ";

	switch ( _item->type )
	{
		case stUnknown:
		{
			FATAL("BUG BUG BUG! " << __FILE__ << __LINE__);
		}
		case stInt:
		{
			result << "sizeof( int ), SQLT_INT";
			break;
		}
		case stFloat:
		case stDouble:
		{
			result << "double";
			break;
		}
		case stTimeStamp:
		case stTime:
		case stDate:
		case stText:
		{
			result << "text";
			break;
		}

	}
	result << ", ( dvoid * ) 0, ( ub2 * ) 0, ( ub2 * ) 0, ( ub4 ) 0, ( ub4 * ) 0, OCI_DEFAULT ));";

	return result.str();
}

String OracleGenerator::getReadValue(const ListElements::iterator & _item, int _index)
{
	std::stringstream result;
	
	result << "m_currentRow->m_" << _item->name << " = m_" << _item->name << ";";

	return result.str();
}

void OracleGenerator::addSelect(SelectElements _elements)
{
	checkConnection();

	/* Allocate and prepare SQL statement */
	OCIStmt *_stmt = 0;
	
	oraCheckErr( m_err, OCIHandleAlloc ( (dvoid*) m_env, ( dvoid ** ) &_stmt, OCI_HTYPE_STMT, 0, 0));

	oraCheckErr( m_err, OCIStmtPrepare ( _stmt, m_err, (const OraText * )_elements.sql.c_str(),
						_elements.sql.length(), OCI_NTV_SYNTAX, OCI_DEFAULT ));

	oraCheckErr( m_err, OCIStmtExecute( m_svc, _stmt, m_err, 0, 0, 0, 0, OCI_DESCRIBE_ONLY));

	/* Get the number of columns in the query */
	ub4 colCount = 0;
	oraCheckErr( m_err, OCIAttrGet((dvoid *)_stmt, OCI_HTYPE_STMT, (dvoid *)&colCount,
						(ub4 *)0, OCI_ATTR_PARAM_COUNT, m_err));

	ub2 type = 0;
	OCIParam *col = 0;

	ub4 nameLen, colWidth, charSemantics;
	text *name;
	
	for (ub4 i = 1; i <= colCount; i++)
	{
		/* get parameter for column i */
		oraCheckErr( m_err, OCIParamGet((dvoid *)_stmt, OCI_HTYPE_STMT, m_err, (dvoid**)&col, i));

		/* get data-type of column i */
		type = 0;
		oraCheckErr( m_err, OCIAttrGet((dvoid *)col, OCI_DTYPE_PARAM,
				(dvoid *)&type, (ub4 *)0, OCI_ATTR_DATA_TYPE,  m_err));

		/* Retrieve the column name attribute */
		nameLen = 0;
		oraCheckErr( m_err, OCIAttrGet((dvoid*)col, OCI_DTYPE_PARAM,
				(dvoid**) &name, &nameLen, OCI_ATTR_NAME, m_err ));

		/* Retrieve the length semantics for the column */
		charSemantics = 0;
		oraCheckErr( m_err, OCIAttrGet((dvoid*)col, OCI_DTYPE_PARAM,
				(dvoid*) &charSemantics,(ub4 *) 0, OCI_ATTR_CHAR_USED, m_err ));
		
		colWidth = 0;
		if (charSemantics)
			/* Retrieve the column width in characters */
			oraCheckErr( m_err, OCIAttrGet((dvoid*)col, OCI_DTYPE_PARAM,
					(dvoid*) &colWidth, (ub4 *) 0, OCI_ATTR_CHAR_SIZE, m_err ));
		else
			/* Retrieve the column width in bytes */
			oraCheckErr( m_err, OCIAttrGet((dvoid*)col, OCI_DTYPE_PARAM,
					(dvoid*) &colWidth,(ub4 *) 0, OCI_ATTR_DATA_SIZE, m_err ));

		std::cout << "Col: " << name << " type:" << type << " len:" << colWidth << std::endl;
	}
	
	OCIHandleFree ( (dvoid*) _stmt, OCI_HTYPE_STMT );
}

}

