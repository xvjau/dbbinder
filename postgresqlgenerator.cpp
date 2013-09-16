/*
    Copyright 2013 Gianni Rossi

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

#include "postgresqlgenerator.h"

/* These defines were extracted from postgresql/server/catalog/pg_type.y by
 * executing this command:
 *
 * grep '#define ' pg_type.h | grep OID
 *
 * TODO find the 'proper' way of getting these
 * No .h include seems to work.
 */
#define BOOLOID                 16
#define BYTEAOID                17
#define CHAROID                 18
#define NAMEOID                 19
#define INT8OID                 20
#define INT2OID                 21
#define INT2VECTOROID           22
#define INT4OID                 23
#define REGPROCOID              24
#define TEXTOID                 25
#define OIDOID                  26
#define TIDOID                  27
#define XIDOID                  28
#define CIDOID                  29
#define OIDVECTOROID            30
#define JSONOID                 114
#define XMLOID                  142
#define PGNODETREEOID           194
#define POINTOID                600
#define LSEGOID                 601
#define PATHOID                 602
#define BOXOID                  603
#define POLYGONOID              604
#define LINEOID                 628
#define FLOAT4OID               700
#define FLOAT8OID               701
#define ABSTIMEOID              702
#define RELTIMEOID              703
#define TINTERVALOID            704
#define UNKNOWNOID              705
#define CIRCLEOID               718
#define CASHOID                 790
#define MACADDROID              829
#define INETOID                 869
#define CIDROID                 650
#define INT4ARRAYOID            1007
#define TEXTARRAYOID            1009
#define FLOAT4ARRAYOID          1021
#define ACLITEMOID              1033
#define CSTRINGARRAYOID         1263
#define BPCHAROID               1042
#define VARCHAROID              1043
#define DATEOID                 1082
#define TIMEOID                 1083
#define TIMESTAMPOID            1114
#define TIMESTAMPTZOID          1184
#define INTERVALOID             1186
#define TIMETZOID               1266
#define BITOID                  1560
#define VARBITOID               1562
#define NUMERICOID              1700
#define REFCURSOROID            1790
#define REGPROCEDUREOID         2202
#define REGOPEROID              2203
#define REGOPERATOROID          2204
#define REGCLASSOID             2205
#define REGTYPEOID              2206
#define REGTYPEARRAYOID         2211
#define TSVECTOROID             3614
#define GTSVECTOROID            3642
#define TSQUERYOID              3615
#define REGCONFIGOID            3734
#define REGDICTIONARYOID        3769
#define INT4RANGEOID            3904
#define RECORDOID               2249
#define RECORDARRAYOID          2287
#define CSTRINGOID              2275
#define ANYOID                  2276
#define ANYARRAYOID             2277
#define VOIDOID                 2278
#define TRIGGEROID              2279
#define LANGUAGE_HANDLEROID     2280
#define INTERNALOID             2281
#define OPAQUEOID               2282
#define ANYELEMENTOID           2283
#define ANYNONARRAYOID          2776
#define ANYENUMOID              3500
#define FDW_HANDLEROID          3115
#define ANYRANGEOID             3831

namespace DBBinder
{

class PQResult
{
private:
    PGresult    *m_res;

public:
    PQResult(): m_res(0) {}
    PQResult(PGresult *_res): m_res(_res) {}
    ~PQResult() { if (m_res) PQclear(m_res); }

    operator const PGresult*() const { return m_res; }

    void operator=(PGresult *_res)
    {
        if (m_res)
            PQclear(m_res);

        m_res = _res;
    }
};

#ifndef NDEBUG
#define PGCommandCheck(CONN, RES) do { __pqCheckErr(CONN, RES, PGRES_COMMAND_OK, __FILE__, __LINE__); } while (false)
#define PGResultCheck(CONN, RES) do { __pqCheckErr(CONN, RES, PGRES_TUPLES_OK, __FILE__, __LINE__); } while (false)
#else
#define PGCommandCheck(CONN, RES) do { __pqCheckErr(CONN, RES, PGRES_COMMAND_OK); } while (false)
#define PGResultCheck(CONN, RES) do { __pqCheckErr(CONN, RES, PGRES_TUPLES_OK); } while (false)
#endif

void __pqCheckErr(PGconn *_conn, const PQResult &_res, ExecStatusType _statusCheck
#ifndef NDEBUG
    , const char* _file, int _line
#endif
    )
{
    if (PQresultStatus(_res) != _statusCheck)
    {
#ifndef NDEBUG
        FATAL(_file << ':' << _line << " - PostgreSQL: " << PQerrorMessage(_conn));
#else
        FATAL("PostgreSQL: " << PQerrorMessage(_conn));
#endif
        abort();
    }
}

struct PGTypePair
{
    PGTypePair(std::string _lang, std::string _pg):
        lang(_lang),
        pg(_pg)
    {}

    PGTypePair(std::string _lang, int _pg):
        lang(_lang)
    {
        std::stringstream s;
        s << _pg;
        pg = s.str();
    }

    std::string lang;
    std::string pg;
};

PGTypePair getPgTypes(SQLTypes _type)
{
    switch(_type)
    {
        case stUnknown:
        case stInt:
        case stUInt:
            return PGTypePair( "int32_t", INT4OID );
        case stInt64:
        case stUInt64:
            return PGTypePair( "int64_t", INT8OID );
        case stFloat:
        case stUFloat:
            return PGTypePair( "float4", FLOAT4OID );
        case stDouble:
        case stUDouble:
            return PGTypePair( "float8", FLOAT8OID );
        case stTimeStamp:
            return PGTypePair( "float4", TIMESTAMPTZOID );
        case stTime:
            return PGTypePair( "float4", TIMEOID );
        case stDate:
            return PGTypePair( "float4", DATEOID );
        case stText:
            return PGTypePair( "string", VARCHAROID );
        case stBlob:
            return PGTypePair( "string", TEXTOID );
        default:
            FATAL(__FILE__  << ':' << __LINE__ << ": Invalide field type: " << _type);
    }
}

SQLTypes getSQLTypes(Oid _pgType)
{
    switch(_pgType)
    {
        case INT4OID:
            return stInt;
        case INT8OID:
            return stInt64;
        case FLOAT4OID:
            return stFloat;
        case FLOAT8OID:
            return stDouble;
        case TIMESTAMPTZOID:
        case TIMESTAMPOID:
            return stTimeStamp;
        case TIMEOID:
        case TIMETZOID:
            return stTime;
        case DATEOID:
            return stDate;
        case VARCHAROID:
            return stText;
        case TEXTOID:
            return stBlob;
        default:
            FATAL(__FILE__  << ':' << __LINE__ << ": Invalide field type: " << _pgType);
    }
}

std::string getStmtType(SQLStatementTypes _type)
{
    switch ( _type )
    {
        case sstSelect: return "select";
        case sstInsert: return "insert";
        case sstUpdate: return "update";
        case sstDelete: return "delete";
        default:
            FATAL(__FILE__  << ':' << __LINE__ << ": Invalid statement type.");
    };
}

PostgreSQLGenerator::PostgreSQLGenerator()
: AbstractGenerator(), m_conn(0)
{
    m_dbengine = "postgresql";
}

PostgreSQLGenerator::~PostgreSQLGenerator()
{
    if ( m_connected )
    {
        PQfinish(m_conn);
    }
}

bool PostgreSQLGenerator::checkConnection()
{
    if ( !m_connected )
    {
        std::vector<const char*> keywords, values;

        _dbParams::iterator it = m_dbParams.begin(), end = m_dbParams.end();
        for(; it != end; it++)
        {
            keywords.push_back(it->first.c_str());
            values.push_back(it->second.value.c_str());
        }

        keywords.push_back(0);
        values.push_back(0);

        m_conn = PQconnectdbParams(keywords.data(), values.data(), 0);
    }

    ConnStatusType status = PQstatus(m_conn);
    if(status != CONNECTION_OK)
    {
#ifndef NDEBUG
        FATAL(__FILE__ << ':' << __LINE__ << " - PostgreSQL: " << PQerrorMessage(m_conn));
#else
        FATAL("PostgreSQL: " << PQerrorMessage(m_conn));
#endif
        m_connected = false;
    }
    else
        m_connected = true;

    return m_connected;
}

std::string PostgreSQLGenerator::getBind(SQLStatementTypes /*_type*/, const ListElements::iterator& _item, int _index)
{
    std::stringstream str;

    switch(_item->type)
    {
        case stInt:
        {
            str <<
                "m_buff" << _item->name << " = (int32_t)(htonl(_" << _item->name << "));\n"
                "paramValues[" << _index << "] = (const char*)(&m_buff" << _item->name << ");\n"
                "paramLengths[" << _index << "] = sizeof(m_buff" << _item->name << ");\n"
                "paramFormats[" << _index << "] = PQ_RESULT_FORMAT_BINARY;";
            break;
        }
        case stUInt:
        {
            str <<
                "m_buff" << _item->name << " = htonl(_" << _item->name << ");\n"
                "paramValues[" << _index << "] = (const char*)(&m_buff" << _item->name << ");\n"
                "paramLengths[" << _index << "] = sizeof(m_buff" << _item->name << ");\n"
                "paramFormats[" << _index << "] = PQ_RESULT_FORMAT_BINARY;";
            break;
        }
        case stInt64:
        {
            str <<
                "m_buff" << _item->name << " = (int32_t)(htobe64(_" << _item->name << "));\n"
                "paramValues[" << _index << "] = (const char*)(&m_buff" << _item->name << ");\n"
                "paramLengths[" << _index << "] = sizeof(m_buff" << _item->name << ");\n"
                "paramFormats[" << _index << "] = PQ_RESULT_FORMAT_BINARY;";
            break;
        }
        case stUInt64:
        {
            str <<
                "m_buff" << _item->name << " = htobe64(_" << _item->name << ");\n"
                "paramValues[" << _index << "] = (const char*)(&m_buff" << _item->name << ");\n"
                "paramLengths[" << _index << "] = sizeof(m_buff" << _item->name << ");\n"
                "paramFormats[" << _index << "] = PQ_RESULT_FORMAT_BINARY;";
            break;
        }
        case stFloat:
        case stDouble:
        case stUFloat:
        case stUDouble:

        case stTimeStamp:
        case stTime:
        case stDate:
            FATAL(__FILE__  << ':' << __LINE__ << ": Invalid param type: '" << _item->name << "': " << _item->type);
            break;

        case stText:
        {
            str <<
                "paramValues[" << _index << "] = _" << _item->name << ";\n"
                "paramLengths[" << _index << "] = 0;\n"
                "paramFormats[" << _index << "] = PQ_RESULT_FORMAT_TEXT;";
            break;
        }

        case stBlob:
        default:
            FATAL(__FILE__  << ':' << __LINE__ << ": Invalid param type: '" << _item->name << "': " << _item->type);
            break;
    }

    return str.str();
}

std::string PostgreSQLGenerator::getReadValue(SQLStatementTypes _type, const ListElements::iterator& _item, int _index)
{
    std::stringstream str;
    str << "if (!m_isNull" << _item->name << ")\n{";

    switch(_item->type)
    {
        case stInt:
            str << "m_" << _item->name << " = (int32_t)(ntohl(*((int32_t*)PQgetvalue(_parent->m_" << getStmtType(_type) << "Stmt.get(), _parent->m_rowNum, " << _index << "))));";
            break;
        case stUInt:
            str << "m_" << _item->name << " = ntohl(*((int32_t*)PQgetvalue(_parent->m_" << getStmtType(_type) << "Stmt.get(), _parent->m_rowNum, " << _index << ")));";
            break;

        case stInt64:
            str << "m_" << _item->name << " = (int32_t)(be64toh(*((int32_t*)PQgetvalue(_parent->m_" << getStmtType(_type) << "Stmt.get(), _parent->m_rowNum, " << _index << "))));";
            break;
            
        case stUInt64:
            str << "m_" << _item->name << " = be64toh(*((int32_t*)PQgetvalue(_parent->m_" << getStmtType(_type) << "Stmt.get(), _parent->m_rowNum, " << _index << ")));";
            break;
            
        case stFloat:
        case stDouble:
        case stUFloat:
        case stUDouble:
        case stTimeStamp:
        case stTime:
        case stDate:
            FATAL(__FILE__  << ':' << __LINE__ << ": Invalid param type: '" << _item->name << "': " << sqlTypeToName(_item->type));
            break;

        case stText:
            str << "m_" << _item->name << " = PQgetvalue(_parent->m_" << getStmtType(_type) << "Stmt.get(), _parent->m_rowNum, " << _index << ");";
            break;

        case stBlob:
        default:
            FATAL(__FILE__  << ':' << __LINE__ << ": Invalid param type: '" << _item->name << "': " << sqlTypeToName(_item->type));
            break;
    }

    str << "}\n";
    return str.str();
}

std::string PostgreSQLGenerator::getIsNull(SQLStatementTypes _type, const ListElements::iterator& /*_item*/, int _index)
{
    std::stringstream str;
    str << "(PQgetisnull(_parent->m_" << getStmtType(_type) << "Stmt.get(), _parent->m_rowNum, " << _index << "))";
    return str.str();
}

void PostgreSQLGenerator::addSelect(SelectElements _elements)
{
    checkConnection();

    PQResult res;

    res = PQprepare(m_conn, _elements.name.c_str(), _elements.sql.c_str(), _elements.input.size(), 0);
    PGCommandCheck(m_conn, res);

    res = PQdescribePrepared(m_conn, _elements.name.c_str());
    PGCommandCheck(m_conn, res);

    int fields = PQnfields(res);
    for(int i = 0; i != fields; i++)
    {
#ifndef NDEBUG
        std::cerr << "Field(" << i << "): '" << PQfname(res, i) << "' = " << PQftype(res, i) << std::flush;
#endif
        SQLTypes type = getSQLTypes(PQftype(res, i));
#ifndef NDEBUG
        std::cerr << " - " << sqlTypeToName(type) << std::endl;
#endif

        _elements.output.push_back( SQLElement( PQfname(res, i), type, i, PQfsize(res, i) ));
    }

    AbstractGenerator::addSelect(_elements);
}

bool PostgreSQLGenerator::needIOBuffers() const
{
    return true;
}

void PostgreSQLGenerator::addInBuffers(SQLStatementTypes /*_type*/, TemplateDictionary *_subDict, const AbstractElements* _elements)
{
    if ( !_elements->input.empty() )
    {
        TemplateDictionary *buffDict= _subDict->AddSectionDictionary(tpl_STMT_IN_FIELDS_BUFFERS);

        std::stringstream str;

        int count = _elements->input.size();

        str <<
            "const char *paramValues[" << count << "];\n"
            "int paramLengths[" << count << "];\n"
            "int paramFormats[" << count <<  "];";

        buffDict->SetValue(tpl_BUFFER_ALLOC, str.str() );
    }
}

void PostgreSQLGenerator::addOutBuffers(SQLStatementTypes /*_type*/, TemplateDictionary *_subDict, const AbstractElements* _elements)
{
    TemplateDictionary *buffDict = _subDict->AddSectionDictionary(tpl_STMT_OUT_FIELDS_BUFFERS);
    
    std::stringstream decl, init;
    
    decl << "int m_rowNum;\nint m_rowCount;";
    init << "m_rowNum = -1;\nm_rowCount = -1;";
    
    ListElements::const_iterator it = _elements->input.begin(), end = _elements->input.end();
    for(; it != end; it++)
    {
        switch(it->type)
        {
            case stInt:
            case stUInt:
            case stInt64:
            case stUInt64:
            case stFloat:
            case stDouble:
            case stUFloat:
            case stUDouble:
            {
                decl << "int m_buff" << it->name << ";\n";
                init << "m_buff" << it->name << " = 0;\n";
                break;
            }

            case stTimeStamp:
            case stTime:
            case stDate:
                FATAL(__FILE__  << ':' << __LINE__ << ": Invalid param type: '" << it->name << "': " << it->type);
                break;

            case stText:
                break;

            case stBlob:
            default:
                FATAL(__FILE__  << ':' << __LINE__ << ": Invalid param type: '" << it->name << "': " << it->type);
                break;
        }
    }
    
    buffDict->SetValue(tpl_BUFFER_DECLARE, decl.str());
    buffDict->SetValue(tpl_BUFFER_INITIALIZE, init.str());
}

}
