#include <iostream>

#include <ibase.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

static __thread ISC_STATUS err[32];

static void printError()
{
	const ISC_STATUS *p = err;
	char str[512];

	while ( fb_interpret( str, 512, &p ) )
	{
		std::cerr << str << std::endl;
	}

	abort();
};

inline void checkFBError()
{
	if ( err[0] == 1 && err[1] )
		printError();
};

template<typename P, typename T> void add_dbd_param( P *&_dpb, T _param )
{
	*_dpb++ = _param;
}

template<typename P, typename T> void add_dbd_string( P *&_dpb, T _param, const char *_str )
{
	if ( _str )
	{
		unsigned int _strLength = strlen( _str );

		if ( _strLength < 256 )
		{
			*_dpb++ = _param;
			*_dpb++ = _strLength;

			for ( const char *q = _str; *q; )
				*_dpb++ = *q++;
		}
	}
}

int main( int argc, char **argv )
{
	const char *s_dbparam_db = "marconi/3050:/var/db/firebird/teste.fdb";
	const char *s_dbparam_user = "sysdba";
	const char *s_dbparam_password = "masterkey";
	const char *s_dbparam_role = 0;
	const char *s_dbparam_charset = "UTF8";

	const char *s_selectSQL = "SELECT a.IDUSER, a.LOGIN, a.\"PASSWORD\", a.SKIN FROM USERS a WHERE a.IDUSER >= ?";
	const unsigned int s_selectSQL_len = strlen( s_selectSQL );
	const unsigned int s_selectFieldCount = 4;
	const unsigned int s_selectParamCount = 1;

	isc_db_handle m_conn = 0;
	isc_stmt_handle m_selectStmt = 0;
	isc_tr_handle m_tr = 0;

	{
		char dpb [1024];
		char *p = dpb;

		add_dbd_param( p, isc_dpb_version1 );
		add_dbd_string( p, isc_dpb_user_name, s_dbparam_user );
		add_dbd_string( p, isc_dpb_password, s_dbparam_password );
		add_dbd_string( p, isc_dpb_sql_role_name, s_dbparam_role );
		add_dbd_string( p, isc_dpb_lc_ctype, s_dbparam_charset );

		int dpbLength = p - dpb;

		isc_attach_database( err, strlen( s_dbparam_db ), const_cast<char*>( s_dbparam_db ), &m_conn, dpbLength, dpb );
		checkFBError();
	}

	/************* TRANS *************/

	{
		char dpb[] = { isc_tpb_version3, isc_tpb_read, isc_tpb_read_committed, isc_tpb_no_rec_version, isc_tpb_wait};

		isc_start_transaction( err, &m_tr, 1, &m_conn, sizeof( dpb ), dpb );
		checkFBError();
	}

	/************* SELECT *************/

	isc_dsql_alloc_statement2( err, &m_conn, &m_selectStmt );
	checkFBError();

	/************** OUTPUT *******************/
	XSQLDA *m_selOutBuffer;
	m_selOutBuffer = 0;

	ISC_LONG f_IDUSER = 0;
	ISC_SHORT f_IDUSER_ISNULL = 0;
	ISC_SCHAR *f_LOGIN;
	ISC_SHORT f_LOGIN_ISNULL = 0;
	ISC_SCHAR *f_PASSWORD;
	ISC_SHORT f_PASSWORD_ISNULL = 0;
	ISC_SCHAR *f_SKIN;
	ISC_SHORT f_SKIN_ISNULL = 0;

	if ( !m_selOutBuffer )
	{
		m_selOutBuffer = ( XSQLDA * )malloc( XSQLDA_LENGTH( s_selectFieldCount ) );
		memset( m_selOutBuffer, 0, XSQLDA_LENGTH( s_selectFieldCount ) );
		m_selOutBuffer->version = SQLDA_VERSION1;
		m_selOutBuffer->sqln = s_selectFieldCount;
	}

	/************** PREPARE *******************/

	isc_dsql_prepare( err, &m_tr, &m_selectStmt, s_selectSQL_len, const_cast<char*>( s_selectSQL ), 3, m_selOutBuffer );
	checkFBError();

	/************** OUTPUT ******************/

	m_selOutBuffer->sqlvar[0].sqldata = reinterpret_cast<ISC_SCHAR*>(&f_IDUSER);
	m_selOutBuffer->sqlvar[0].sqltype = SQL_LONG;
	m_selOutBuffer->sqlvar[0].sqlind = &f_IDUSER_ISNULL;

	f_LOGIN = reinterpret_cast<ISC_SCHAR*>(malloc(m_selOutBuffer->sqlvar[1].sqllen));
	m_selOutBuffer->sqlvar[1].sqldata = f_LOGIN;
	m_selOutBuffer->sqlvar[1].sqltype = SQL_VARYING + 1;
	m_selOutBuffer->sqlvar[1].sqlind = &f_LOGIN_ISNULL;

	f_PASSWORD = reinterpret_cast<ISC_SCHAR*>(malloc(m_selOutBuffer->sqlvar[2].sqllen));
	m_selOutBuffer->sqlvar[2].sqldata = f_PASSWORD;
	m_selOutBuffer->sqlvar[2].sqltype = SQL_VARYING + 1;
	m_selOutBuffer->sqlvar[2].sqlind = &f_PASSWORD_ISNULL;

	f_SKIN = reinterpret_cast<ISC_SCHAR*>(malloc(m_selOutBuffer->sqlvar[3].sqllen));
	m_selOutBuffer->sqlvar[3].sqldata = f_SKIN;
	m_selOutBuffer->sqlvar[3].sqltype = SQL_VARYING + 1;
	m_selOutBuffer->sqlvar[3].sqlind = &f_SKIN_ISNULL;

	/************** INPUT ******************/
	XSQLDA *m_selInBuffer;
	m_selInBuffer = 0;

	if ( !m_selInBuffer )
	{
		m_selInBuffer = ( XSQLDA * )malloc( XSQLDA_LENGTH( s_selectParamCount ) );
		memset( m_selInBuffer, 0, XSQLDA_LENGTH( s_selectParamCount ) );
		m_selInBuffer->version = SQLDA_VERSION1;
		m_selInBuffer->sqln = s_selectParamCount;
	}

	isc_dsql_describe_bind( err, &m_selectStmt, s_selectParamCount, m_selInBuffer );
	checkFBError();

	int i;
	XSQLVAR *var;
	for ( i = 0, var = m_selInBuffer->sqlvar; i < m_selInBuffer->sqld; i++, var++ )
	{
		/* Process each XSQLVAR parameter structure here.
		Var points to the parameter structure. */
		ISC_SHORT dtype = ( var->sqltype & ~1 ); /* drop NULL flag for now */

		std::cout << "Param: " << var->sqlname << "\tType: " << dtype;

		switch ( dtype )
		{
			case SQL_VARYING: /* coerce to SQL_TEXT */
				var->sqltype = SQL_TEXT;
				/* allocate local variable storage */
				var->sqldata = ( char * )malloc( sizeof( char ) * var->sqllen );
				std::cout << "\tset: " << 5;
				break;
			case SQL_TEXT:
				var->sqldata = ( char * )malloc( sizeof( char ) * var->sqllen );
				std::cout << "\tset: " << 5;
				/* provide a value for the parameter */
				break;
			case SQL_LONG:
				var->sqldata = ( char * )malloc( sizeof( long ) );
				/* provide a value for the parameter */
				*( long * )( var->sqldata ) = -1;
				std::cout << "\tset: " << -1;
				break;
		} /* end of switch statement */

		if ( var->sqltype & 1 )
		{
			/* allocate variable to hold NULL status */
			var->sqlind = ( short * )malloc( sizeof( short ) );
			*(var->sqlind) = 0;
			std::cout << "\tis null";
		}

		std::cout << std::endl;
	}

	/****************** EXEC ****************/

	isc_dsql_execute(err, &m_tr, &m_selectStmt, s_selectParamCount, m_selInBuffer);
	checkFBError();

	/********************* FETCH *****************/
	std::cout << "IDUSER\tLOGIN\tPASSWORD\tSKIN" << std::endl;
	std::cout << "------\t-----\t--------\t----" << std::endl;

	ISC_STATUS ret;
	while ((ret = isc_dsql_fetch(err, &m_selectStmt, SQLDA_VERSION1, m_selOutBuffer)) == 0)
	{
		//std::cout << "\n" << m_selOutBuffer->sqlvar[2].sqlscale << std::endl;

		if ( f_IDUSER_ISNULL ) std::cout << "(null)" << "\t"; else std::cout << f_IDUSER << "\t";

		if ( f_LOGIN_ISNULL ) std::cout << "(null)" << "\t"; else std::cout << (f_LOGIN+2) << "\t";
		if ( f_PASSWORD_ISNULL ) std::cout << "(null)" << "\t"; else std::cout << (f_PASSWORD+2) << "\t";
		if ( f_SKIN_ISNULL ) std::cout << "(null)" << "\t"; else std::cout << (f_SKIN+2) << "\t";

		std::cout << std::endl;
	}

	if (ret != 100L)
	{
		checkFBError();
	}
	std::cout << "------\t-----\t--------\t----" << std::endl;

	/******************* free *************/
	isc_dsql_free_statement( err, &m_selectStmt, DSQL_drop );
	checkFBError();

	isc_rollback_transaction( err, &m_tr );
	checkFBError();

	isc_detach_database( err, &m_conn );
	checkFBError();

	return 0;
}
// kate: indent-mode cstyle; replace-tabs off; tab-width 4;
