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

/*
	http://download.oracle.com/docs/cd/B14117_01/appdev.101/b10779/oci03typ.htm#423119
*/
void getOracleTypes(SQLTypes _type, String& _lang, String& _oracle)
{
	switch( _type )
	{
		case stUnknown:
			FATAL("BUG BUG BUG! " << __FILE__ << __LINE__);

		case stInt:
			_lang = "int";
			_oracle = "SQLT_INT";
			break;

		case stFloat:
			_lang = "float";
			_oracle = "SQLT_FLT";
			break;

		case stDouble:
			_lang = "double";
			_oracle = "SQLT_FLT";
			break;

		case stTimeStamp:
		case stTime:
			_lang = "OCIDateTime";
			_oracle = "SQLT_TIMESTAMP";
			break;

		case stDate:
			_lang = "char[7]";
			_oracle = "SQLT_DAT";
			break;

		case stText:
		default:
		{
			_lang = "char";
			_oracle = "SQLT_STR";
			break;
		}
	}
}

SQLTypes getSQLTypes(ub2 _oracleType)
{
	switch( _oracleType )
	{
		case SQLT_INT:
			return stInt;
		case SQLT_FLT:
		case SQLT_BDOUBLE:
			return stDouble;
		case SQLT_BFLOAT:
			return stFloat;
		case SQLT_ODT:
			return stDate;

		case SQLT_DATE:
		case SQLT_TIMESTAMP:
		case SQLT_TIMESTAMP_TZ:
		case SQLT_TIMESTAMP_LTZ:
			return stTimeStamp;

		case SQLT_CHR:
		case SQLT_NUM:
		case SQLT_STR:
		case SQLT_VCS:
		default:
			return stText;
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

String OracleGenerator::getBind(SQLStatementTypes _type, const ListElements::iterator & _item, int _index)
{
	String langType, oraType;
	getOracleTypes( _item->type, langType, oraType );

	std::stringstream result;

	result << "oraCheckErr( m_conn->err, OCIBindByPos( m_selectStmt, &m_param" << _item->name << ", m_conn->err, " << _index << ",\n"
			<< "(dvoid*)&_" << _item->name << ",";

	if ( _item->type != stText )
		result << " sizeof (" << langType << "), " << oraType << ", 0,\n";
	else
		result << " sizeof(" << _item->length << "), " << oraType << ", 0,\n";

	result << "0, 0, 0, 0, OCI_DEFAULT ));\n";

	return result.str();
}

String OracleGenerator::getReadValue(SQLStatementTypes _type, const ListElements::iterator & _item, int _index)
{
	return String("m_") + _item->name + " = _parent->m_buff" + _item->name + ";";
}

String OracleGenerator::getIsNull(SQLStatementTypes _type, const ListElements::iterator& _item, int _index)
{
	FATAL("Not implemented!");
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
						0, OCI_ATTR_PARAM_COUNT, m_err));

	ub2 oraType = 0;
	OCIParam *col = 0;

	ub4 nameLen, colWidth, charSemantics;
	text *name;

	for (ub4 i = 1; i <= colCount; i++)
	{
		/* get parameter for column i */
		oraCheckErr( m_err, OCIParamGet((dvoid *)_stmt, OCI_HTYPE_STMT, m_err, (dvoid**)&col, i));

		/* get data-type of column i */
		oraType = 0;
		oraCheckErr( m_err, OCIAttrGet((dvoid *)col, OCI_DTYPE_PARAM,
				(dvoid *)&oraType, 0, OCI_ATTR_DATA_TYPE,  m_err));

		/* Retrieve the column name attribute */
		nameLen = 0;
		oraCheckErr( m_err, OCIAttrGet((dvoid*)col, OCI_DTYPE_PARAM,
				(dvoid**) &name, &nameLen, OCI_ATTR_NAME, m_err ));

		/* Retrieve the length semantics for the column */
		charSemantics = 0;
		oraCheckErr( m_err, OCIAttrGet((dvoid*)col, OCI_DTYPE_PARAM,
				(dvoid*) &charSemantics,0, OCI_ATTR_CHAR_USED, m_err ));

		colWidth = 0;
		if (charSemantics)
			/* Retrieve the column width in characters */
			oraCheckErr( m_err, OCIAttrGet((dvoid*)col, OCI_DTYPE_PARAM,
					(dvoid*) &colWidth, 0, OCI_ATTR_CHAR_SIZE, m_err ));
		else
			/* Retrieve the column width in bytes */
			oraCheckErr( m_err, OCIAttrGet((dvoid*)col, OCI_DTYPE_PARAM,
					(dvoid*) &colWidth,0, OCI_ATTR_DATA_SIZE, m_err ));

		_elements.output.push_back( SQLElement( String(reinterpret_cast<char*>(name), nameLen), getSQLTypes( oraType ), i, colWidth ));
	}

	OCIHandleFree ( (dvoid*) _stmt, OCI_HTYPE_STMT );

	AbstractGenerator::addSelect(_elements);
}

bool OracleGenerator::needIOBuffers() const
{
	return true;
}

void OracleGenerator::addSelInBuffers(const SelectElements * _select)
{
	String langType, oraType;
	int index = 0;
	ctemplate::TemplateDictionary *subDict;

	foreach(SQLElement field, _select->input)
	{
		getOracleTypes( field.type, langType, oraType );

		subDict = m_dict->AddSectionDictionary(tpl_SEL_IN_FIELDS_BUFFERS);
		subDict->SetValue(tpl_BUFFER_DECLARE, String("OCIBind	*m_param") + field.name + ";" );

		++index;
	}
}

void OracleGenerator::addSelOutBuffers(const SelectElements * _select)
{
	String langType, oraType;
	int index = 0;
	ctemplate::TemplateDictionary *subDict;

	foreach(SQLElement field, _select->output)
	{
		std::stringstream init, decl;

		getOracleTypes( field.type, langType, oraType );

		decl << "OCIDefine*	m_def" << field.name << ";\n";
		init <<	"oraCheckErr( m_conn->err, OCIDefineByPos( m_selectStmt, &m_def" << field.name << ", m_conn->err, " << index + 1 << ", (dvoid*) &m_buff" << field.name << ",\n";

		if ( field.type != stText )
		{
			decl << langType << " m_buff" << field.name << ";\n";
			init << "sizeof(" << langType << ")";
		}
		else
		{
			decl << langType << " m_buff" << field.name << "[" << field.length + 1 << "];\n";
			init << field.length;
		}

		init << "," << oraType << ", 0, 0, 0, OCI_DEFAULT ));";

		subDict = m_dict->AddSectionDictionary(tpl_SEL_OUT_FIELDS_BUFFERS);
		subDict->SetValue(tpl_BUFFER_DECLARE, decl.str() );
		subDict->SetValue(tpl_BUFFER_ALLOC, init.str() );

		++index;
	}
}

}
