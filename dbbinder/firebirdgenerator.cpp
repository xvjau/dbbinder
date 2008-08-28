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

namespace DBBinder
{

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
		}
		return true;
	}
	return false;
}
	
FirebirdGenerator::FirebirdGenerator()
 : AbstractGenerator(), m_conn(0)
{
}


FirebirdGenerator::~FirebirdGenerator()
{
	if ( m_connected )
	{
		ISC_STATUS err[32];
		isc_detach_database( err, &m_conn );
	}
}

bool FirebirdGenerator::checkConnection()
{
	if ( !m_connected )
	{
		ISC_STATUS err[32];
    	char dpb [2048];
		char *p = dpb;
		
		#define READ_PARAM(NAME) \
		String NAME = m_dbParams[# NAME].value; \
		if ( NAME.empty() ) \
			FATAL( "MySQL: '" # NAME "' db parameter is empty." );
		
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
		
		m_connected = m_conn;
	}
	
	return m_connected;
}

String FirebirdGenerator::getBind(const ListElements::iterator& _item, int _index)
{
}

String FirebirdGenerator::getReadValue(const ListElements::iterator& _item, int _index)
{
}

void FirebirdGenerator::addSelect(SelectElements _elements)
{
	ISC_STATUS		err[32];
	isc_tr_handle	tr = 0;
	isc_stmt_handle	stmt = 0;
	XSQLDA			*buffInput = 0, *buffOutput = 0;
	
	// Start Transaction
	{
		char dpb[] = { isc_tpb_version3, isc_tpb_read, isc_tpb_read_committed, isc_tpb_no_rec_version, isc_tpb_wait};
		isc_start_transaction( err, &tr, 1, &m_conn, sizeof(dpb), dpb );
		checkFBError( err );
	}
	
	// statement	
	isc_dsql_alloc_statement2( err, &m_conn, &stmt );
	checkFBError( err );
	
	isc_dsql_prepare( err, &tr, &stmt, _elements.sql.length(), const_cast<char*>(_elements.sql.c_str()), 3, buffOutput );
	checkFBError( err );
	
	isc_dsql_free_statement( err, &stmt, DSQL_drop );
	checkFBError( err );
	
	isc_rollback_transaction( err, &tr );
	checkFBError( err );
			
	
    AbstractGenerator::addSelect(_elements);
}

}
