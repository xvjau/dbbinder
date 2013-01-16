//
// C++ Implementation: insert.cpp
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "insert.h"

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

const char * const insert_blob::s_insertSQL = "insert into \
    dbtest (`comment`) \
values \
    (?) \
";
const int insert_blob::s_insertSQL_len = 50;

insert_blob::insert_blob(MYSQL* _conn):
    m_conn( _conn )

    ,m_insertStmt(0)

{
    ASSERT_MSG(m_conn, "Connection must not be null!");

    m_insertStmt = mysql_stmt_init(m_conn);
    mysqlCheckStmtErr(m_insertStmt, mysql_stmt_prepare(m_insertStmt, s_insertSQL, s_insertSQL_len) );

}

insert_blob::~insert_blob()
{

    if (m_insertStmt)
    {
        mysql_stmt_close(m_insertStmt);
        m_insertStmt = 0;
    }

}

const int insert_blob::s_insertParamCount = 1;

void insert_blob::insert(
    std::shared_ptr< std::vector<char> > _comment
)
{
    MYSQL_BIND inBuffer[1];

    long unsigned m_paramcommentLength;
    my_bool m_paramcommentIsNull;

    memset(inBuffer, 0, sizeof(inBuffer));

    inBuffer[0].buffer_type = MYSQL_TYPE_BLOB;
    m_paramcommentIsNull = (_comment) ? 0 : 1;
    m_paramcommentLength = (_comment) ? _comment->size() : 0;

    inBuffer[0].buffer = reinterpret_cast<void*>(&((*_comment)[0]));
    inBuffer[0].is_null = &m_paramcommentIsNull;
    inBuffer[0].length = &m_paramcommentLength;

    mysqlCheckStmtErr(m_insertStmt, mysql_stmt_bind_param(m_insertStmt, inBuffer));
    mysqlCheckStmtErr(m_insertStmt, mysql_stmt_execute(m_insertStmt));
    mysqlCheckStmtErr(m_insertStmt, mysql_stmt_reset(m_insertStmt));
}

