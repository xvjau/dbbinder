//
// C++ Implementation: select.cpp
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "select.h"

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

const char * const select_blob::s_selectSQL = "select \
    `id`, \
    `comment` \
from \
    dbtest";
const int select_blob::s_selectSQL_len = 46;

select_blob::select_blob(MYSQL* _conn):
    m_conn( _conn )

    ,m_selectStmt(0)
    ,m_selectIsActive( false )
    ,m_iterator(0)

{
    ASSERT_MSG(m_conn, "Connection must not be null!");

    m_selectStmt = mysql_stmt_init(m_conn);
    mysqlCheckStmtErr(m_selectStmt, mysql_stmt_prepare(m_selectStmt, s_selectSQL, s_selectSQL_len));

}

select_blob::~select_blob()
{

    select_blob::close();

}

const int select_blob::s_selectFieldCount = 2;
const int select_blob::s_selectParamCount = 0;
select_blob::iterator select_blob::s_endIterator(0);

void select_blob::open(

)
{
    if ( m_selectIsActive )
    {
        mysqlCheckStmtErr(m_selectStmt, mysql_stmt_reset(m_selectStmt));
    }

    memset(selOutBuffer, 0, sizeof(selOutBuffer));

    selOutBuffer[0].buffer_length = sizeof(int);
    selOutBuffer[0].buffer_type = MYSQL_TYPE_LONG;
    selOutBuffer[0].buffer = reinterpret_cast<void *>(&m_buffid);
    selOutBuffer[0].is_null = &m_idIsNull;
    selOutBuffer[0].length = &m_idLength;

    selOutBuffer[1].buffer_type = MYSQL_TYPE_BLOB;
    selOutBuffer[1].is_null = &m_commentIsNull;
    selOutBuffer[1].length = &m_commentLength;

    mysqlCheckStmtErr(m_selectStmt, mysql_stmt_bind_param(m_selectStmt, inBuffer));
    mysqlCheckStmtErr(m_selectStmt, mysql_stmt_execute(m_selectStmt));
    mysqlCheckStmtErr(m_selectStmt, mysql_stmt_bind_result(m_selectStmt, selOutBuffer));

    m_selectIsActive = true;
}

void select_blob::close()
{
    if ( m_selectIsActive )
    {
        m_selectIsActive = false;

        if (m_selectStmt)
        {
            mysql_stmt_close(m_selectStmt);
            m_selectStmt = 0;
        }

        delete m_iterator;
        m_iterator = NULL;
    }
}

bool select_blob::fetchRow()
{

    int ret = mysql_stmt_fetch(m_selectStmt);

    switch( ret )
    {
    case MYSQL_DATA_TRUNCATED:
        LOG_MSG("MySQL data truncated!");
    case 0:
        m_currentRow.reset( new _row_type( this ));
        return true;
    case MYSQL_NO_DATA:
        m_currentRow.reset();
        mysql_stmt_reset(m_selectStmt);
        break;
    default:
        mysqlCheckStmtErr( m_selectStmt, ret );
        break;
    }

    return false;

}

select_blob::iterator & select_blob::begin()
{
    ASSERT_MSG(m_selectIsActive, "Select is not active.  Ensure open() was called.");

    if ( m_iterator )
        return *m_iterator;

    if ( fetchRow() )
    {
        m_iterator = new select_blob::iterator(this);
        return *m_iterator;
    }
    else
        return s_endIterator;
}

