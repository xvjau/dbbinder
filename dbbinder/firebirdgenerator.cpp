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

#include "firebirdgenerator.h"
#include <stdio.h>

namespace DBBinder
{

static const int DEFAULT_COL_COUNT = 10;

template<typename P, typename T> void add_dbd_param(P *&_dpb, T _param)
{
	*_dpb++ = _param;
}

template<typename P, typename T> void add_dbd_string(P *&_dpb, T _param, const String& _str)
{
	if ( _str.length() < 256 )
	{
		*_dpb++ = _param;
		*_dpb++ = _str.length();

		for ( const char *q = _str.c_str(); *q; )
			*_dpb++ = *q++;
	}
}

inline bool checkFBError( const ISC_STATUS *_status )
{
	if ( _status[0] == 1 && _status[1] )
	{
		char str[512];

		while ( fb_interpret( str, 512, &_status ) )
		{
			std::cerr << str << std::endl;
			#ifdef DEBUG
			assert(false);
			#else
			abort();
			#endif
		}
		return true;
	}
	return false;
}

SQLTypes fbtypeToSQLType(const int _fbtype)
{
		switch( _fbtype & ~1 )
		{
			case SQL_TEXT:
			case SQL_VARYING:
				return stText;
			case SQL_SHORT:
			case SQL_LONG:
				return stInt;
			case SQL_FLOAT:
				return stFloat;
			case SQL_DOUBLE:
			case SQL_D_FLOAT:
				return stDouble;
			case SQL_TIMESTAMP:
				return stTimeStamp;
			case SQL_BLOB:
			case SQL_ARRAY:
				return stText;
			case SQL_QUAD:
				return stInt64;
			case SQL_TYPE_TIME:
				return stTime;
			case SQL_TYPE_DATE:
				return stDate;
			case SQL_INT64:
				return stInt64;
			default:
				return stText;
		}
}

void getFirebirdTypes( SQLTypes _type, String &_lang, String &_fb )
{
	switch ( _type )
	{
		case stUnknown:
			FATAL("BUG BUG BUG! " << __FILE__ << __LINE__);
		case stInt:
		case stUInt:
			_lang = "ISC_LONG";
			_fb = "SQL_LONG";
			break;
		case stInt64:
		case stUInt64:
			_lang = "ISC_INT64";
			_fb = "SQL_INT64";
			break;
		case stFloat:
		case stUFloat:
			_lang = "float";
			_fb = "SQL_FLOAT";
			break;
		case stDouble:
		case stUDouble:
			_lang = "double";
			_fb = "SQL_DOUBLE";
			break;
		case stTimeStamp:
			_lang = "ISC_TIMESTAMP";
			_fb = "SQL_LONG";
			break;
		case stTime:
			_lang = "ISC_TIME";
			_fb = "SQL_LONG";
			break;
		case stDate:
			_lang = "ISC_DATE";
			_fb = "SQL_LONG";
			break;
		case stText:
			_lang = "ISC_SCHAR";
			_fb = "SQL_VARYING";
			break;
	}
}


FirebirdGenerator::FirebirdGenerator()
 : AbstractGenerator(), m_conn(0)
{
	m_dbengine = "firebird";
}


FirebirdGenerator::~FirebirdGenerator()
{
	if ( m_connected )
	{
		ISC_STATUS err[32];
		isc_detach_database( err, &m_conn );
	}
}

static void fbversionCallback(void* _param, const char* _ver)
{
	if ( (*static_cast<int*>(_param))++ == 1 )
		std::cout << "Using Firebird version: " << _ver << std::endl;
}

bool FirebirdGenerator::checkConnection()
{
	if ( !m_connected )
	{
		ISC_STATUS err[32];
    	char dpb [2048];
		char *p = dpb;

		#define READ_PARAM(NAME) \
		String NAME = m_dbParams[# NAME].value;

		READ_PARAM(db);
		READ_PARAM(user);
		READ_PARAM(password);
		READ_PARAM(role);
		READ_PARAM(charset);

		add_dbd_param(p, isc_dpb_version1);
		add_dbd_string(p, isc_dpb_user_name, user);
		add_dbd_string(p, isc_dpb_password, password);
		add_dbd_string(p, isc_dpb_sql_role_name, role);
		add_dbd_string(p, isc_dpb_lc_ctype, charset);

		int dpbLength = p - dpb;

		isc_attach_database( err, db.length(), const_cast<char*>(db.c_str()), &m_conn, dpbLength, dpb );
		checkFBError( err );

		int param = 0;
		isc_version(&m_conn, fbversionCallback, &param);

		m_connected = m_conn;
	}

	return m_connected;
}

String FirebirdGenerator::getBind(SQLStatementTypes _type, const ListElements::iterator& _item, int _index)
{
	// TODO Abstract this
	String var("inBuffer->sqlvar[");

	{
		char buff[7] = {0,0,0,0,0,0,0};
		snprintf(buff, 6, "%d", _index);
		var += buff;
	}

	var += "]";

	std::stringstream str;

	if ( _index == 0 )
	{
		str << "ISC_SHORT __ZERO = 0;\n\n";

		String type;
		switch ( _type )
		{
			case sstSelect:
				type = "select";
				break;
			case sstInsert:
				type = "insert";
				break;
			case sstUpdate:
				type = "update";
				break;
			case sstDelete:
				type = "delete";
				break;
			default:
				FATAL(__FILE__  << ':' << __LINE__ << ": Invalide statement type.");
		};

		str << "XSQLDA inBuffer[ XSQLDA_LENGTH( s_" << type << "ParamCount ) ];\n";
		str << "memset(inBuffer, 0, XSQLDA_LENGTH( s_" << type << "ParamCount ));\n";
		str << "inBuffer->version = SQLDA_VERSION1;\n";
		str << "inBuffer->sqln = s_" << type << "ParamCount;\n";
		str << "isc_dsql_describe_bind( err, &m_" << type << "Stmt, s_" << type << "ParamCount, inBuffer );\n\n";
	}

	String type, size;

	getFirebirdTypes(_item->type, size, type);

	str << var << ".sqltype = " << type << " + 1;\n";

	if ( _item->type != stText )
	{
		str << var << ".sqldata = reinterpret_cast<ISC_SCHAR*>(&(_" << _item->name << "));\n";
	}
	else
	{
		str << var << ".sqldata = reinterpret_cast<ISC_SCHAR*>(malloc(strlen(_" << _item->name << ") + 1 + sizeof(short)));\n";
		str << "*(reinterpret_cast<short*>( " << var << ".sqldata )) = strlen(_" << _item->name << ");\n";
		str << "strcpy(" << var << ".sqldata + sizeof(short), _" << _item->name << ");\n";
	}

	str << var << ".sqlind = &(__ZERO);\n";

	return str.str();
}

String FirebirdGenerator::getReadValue(SQLStatementTypes _type, const ListElements::iterator& _item, int _index)
{
	std::stringstream str;

	if ( _item->type == stText )
		str << "m_" << _item->name << " = _parent->m_selOutBuffer->sqlvar[" << _index << "].sqldata + sizeof(short);";
	else
		str << "m_" << _item->name << " = *(_parent->m_selOutBuffer->sqlvar[" << _index << "].sqldata);";

	return str.str();
}

String FirebirdGenerator::getIsNull(SQLStatementTypes _type, const ListElements::iterator& _item, int _index)
{
	// TODO Abstract this
	String result("*(m_selOutBuffer->sqlvar[");

	{
		char buff[7] = {0,0,0,0,0,0,0};
		snprintf(buff, 6, "%d", _index);
		result += buff;
	}

	result += "].sqlind) == -1";

	return result;
}

void FirebirdGenerator::addSelect(SelectElements _elements)
{
	checkConnection();

	ISC_STATUS		err[32];
	isc_tr_handle	tr = 0;
	isc_stmt_handle	stmt = 0;
	XSQLDA			*buffOutput = 0;

	// Start Transaction
	{
		char dpb[] = { isc_tpb_version3, isc_tpb_read, isc_tpb_read_committed, isc_tpb_no_rec_version, isc_tpb_wait};
		isc_start_transaction( err, &tr, 1, &m_conn, sizeof(dpb), dpb );
		checkFBError( err );
	}

	// statement
	isc_dsql_alloc_statement2( err, &m_conn, &stmt );
	checkFBError( err );

	buffOutput = (XSQLDA *)malloc( XSQLDA_LENGTH( DEFAULT_COL_COUNT ) );
	memset(buffOutput, 0, XSQLDA_LENGTH( DEFAULT_COL_COUNT ));
	buffOutput->version = SQLDA_VERSION1;
	buffOutput->sqln = DEFAULT_COL_COUNT;

	isc_dsql_prepare( err, &tr, &stmt, _elements.sql.length(), const_cast<char*>(_elements.sql.c_str()), 3, buffOutput );
	checkFBError( err );

	// Check to see if the buffer is big enough
	if ( buffOutput->sqld > buffOutput->sqln )
	{
		// Resize and re-describe
		int num = buffOutput->sqld;
		buffOutput = (XSQLDA *)realloc( buffOutput, XSQLDA_LENGTH( buffOutput->sqld ) );
		memset(buffOutput, 0, XSQLDA_LENGTH( buffOutput->sqld ));
		buffOutput->version = SQLDA_VERSION1;
		buffOutput->sqln = num;

		isc_dsql_describe( err, &stmt, SQLDA_VERSION1, buffOutput );
		checkFBError( err );
	}

	XSQLVAR *p = buffOutput->sqlvar;
	for( int i = 0; i < buffOutput->sqld; ++i, p++ )
	{
		_elements.output.push_back( SQLElement( p->aliasname, fbtypeToSQLType( p->sqltype ), i, p->sqllen ));
	}

	//Free, deallocate and rollback everything

	free(buffOutput);

	isc_dsql_free_statement( err, &stmt, DSQL_drop );
	checkFBError( err );

	isc_rollback_transaction( err, &tr );
	checkFBError( err );

    AbstractGenerator::addSelect(_elements);
}

bool FirebirdGenerator::needIOBuffers() const
{
	return true;
}

void FirebirdGenerator::addInBuffers(SQLStatementTypes _type, const AbstractElements* _elements)
{
	/*
		Nothing to see here.. please move on to:
		FirebirdGenerator::getBind
			-> if ( _index == 0 )
	*/
}

void FirebirdGenerator::addOutBuffers(SQLStatementTypes _type, const AbstractIOElements* _elements)
{
	templatens::TemplateDictionary *subDict;
	subDict = m_dict->AddSectionDictionary(tpl_SEL_OUT_FIELDS_BUFFERS);
	subDict->SetValue(tpl_BUFFER_DECLARE, "XSQLDA *m_selOutBuffer;" );
	subDict->SetValue(tpl_BUFFER_INITIALIZE,
											"m_selOutBuffer = (XSQLDA *)malloc( XSQLDA_LENGTH( s_selectFieldCount ) );\n"
											"memset(m_selOutBuffer, 0, XSQLDA_LENGTH( s_selectFieldCount ));\n"
											"m_selOutBuffer->version = SQLDA_VERSION1;\n"
											"m_selOutBuffer->sqln = s_selectFieldCount;\n\n"
											);

	std::stringstream bufAlloc, bufFree;

	bufAlloc << "{\n";


	bufFree << "if (m_selOutBuffer)\n{\n";

	String fb, lang;

	char idx[9];
	idx[8] = 0;
	for(ListElements::const_iterator it = _elements->output.begin(); it != _elements->output.end(); ++it)
	{
		snprintf(idx, 8, "%d", it->index);
		getFirebirdTypes( it->type, lang, fb );

		bufAlloc << "m_selOutBuffer->sqlvar[" << idx << "].sqldata = ";
		bufAlloc << "reinterpret_cast<ISC_SCHAR*>(malloc( ";

		if ( it->type == stText )
		{
			bufAlloc << "m_selOutBuffer->sqlvar[" << idx << "].sqllen + 1 ));\n";
			bufAlloc << "m_selOutBuffer->sqlvar[" << idx << "].sqltype = SQL_VARYING + 1;";
		}
		else
		{
			bufAlloc << "sizeof(" << lang << ") ));\n";
			bufAlloc << "m_selOutBuffer->sqlvar[" << idx << "].sqltype = " << fb << " + 1;";
		}

		bufAlloc << "m_selOutBuffer->sqlvar[" << idx << "].sqlind = reinterpret_cast<ISC_SHORT*>(malloc(sizeof(ISC_SHORT)));\n";

		bufFree << "free( m_selOutBuffer->sqlvar[" << idx << "].sqldata );";
		bufFree << "free( m_selOutBuffer->sqlvar[" << idx << "].sqlind );";
	}

	bufAlloc << "}\n";

	bufFree << "free(m_selOutBuffer);\n";
	bufFree << "m_selOutBuffer = NULL;\n}";

	subDict->SetValue(tpl_BUFFER_ALLOC, bufAlloc.str() );


	subDict->SetValue(tpl_BUFFER_FREE, bufFree.str() );
}

}

