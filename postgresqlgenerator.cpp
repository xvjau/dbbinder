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
            return { "int32_t", "23" };
        case stInt64:
        case stUInt64:
            return { "int64_t", "20" };
        case stFloat:
        case stUFloat:
            return { "float4", "700" };
        case stDouble:
        case stUDouble:
            return { "float4", "701" };
        case stTimeStamp:
            return { "float4", "1184" };
        case stTime:
            return { "float4", "1083" };
        case stDate:
            return { "float4", "1082" };
        case stText:
        case stBlob:
            return { "string", "25" };
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

        m_connected = m_conn;
    }

    return m_connected;
}

std::string PostgreSQLGenerator::getBind(SQLStatementTypes _type, const ListElements::iterator& _item, int _index)
{
    return std::string();
}

std::string PostgreSQLGenerator::getReadValue(SQLStatementTypes _type, const ListElements::iterator& _item, int _index)
{
    PGTypePair type = getPgTypes(_item->type);
    std::string stmtType = getStmtType(_type);
    std::stringstream str;

    str << "if (!PQgetisnull(m_" << stmtType << "Res, m_row, " << _index << "))\n{";
    str << type.lang << " m_param" << _item->name << " = PQgetvalue(m_res, m_row, " << _index << ");";

    str << "}\n";
    return str.str();
}

std::string PostgreSQLGenerator::getIsNull(SQLStatementTypes _type, const ListElements::iterator& _item, int _index)
{
    std::stringstream str;
    str << "(PQgetisnull(m_" << getStmtType(_type) << "Res, m_row, " << _index << "))";
    return str.str();
}

void PostgreSQLGenerator::addSelect(SelectElements _elements)
{
    checkConnection();

    PQResult res;

    res = PQprepare(m_conn, _elements.name.c_str(), _elements.sql.c_str(), _elements.input.size(), 0);
    PGCommandCheck(m_conn, res);

    res = PQdescribePrepared(m_conn, _elements.name);
    PGCommandCheck(m_conn, res);

    int fields = PQnfields(res);
    for(int i = 0; i != fields; i++)
    {
        std::cout << "Field(" << i << "): '" << PQfname(res, i) << "' = " << PQftype(res, i) << std::endl;

        SQLTypes type;
        switch(PQftype(res, i))
        {
            default:
                FATAL(__FILE__  << ':' << __LINE__ << ": Invalide field type.");
        }

        _elements.output.push_back( SQLElement( PQfname(res, i), type, i, PQfsize(res, i) ));
    }

    AbstractGenerator::addSelect(_elements);
}

bool PostgreSQLGenerator::needIOBuffers() const
{
    return true;
}

void PostgreSQLGenerator::addInBuffers(SQLStatementTypes _type, const AbstractElements* _elements)
{
    if ( _elements->input.empty() )
    {
        TemplateDictionary *subDict;

        switch ( _type )
        {
            case sstSelect:
                subDict = m_dict->AddSectionDictionary(tpl_SEL_IN_FIELDS_BUFFERS);
                break;
            case sstInsert:
                subDict = m_dict->AddSectionDictionary(tpl_INS_IN_FIELDS_BUFFERS);
                break;
            case sstUpdate:
                subDict = m_dict->AddSectionDictionary(tpl_UPD_IN_FIELDS_BUFFERS);
                break;
            case sstDelete:
                subDict = m_dict->AddSectionDictionary(tpl_DEL_IN_FIELDS_BUFFERS);
                break;
            default:
                FATAL(__FILE__  << ':' << __LINE__ << ": Invalide statement type.");
        };

        subDict->SetValue(tpl_BUFFER_ALLOC, "XSQLDA* inBuffer = 0;" );
    }
    else
    {
        std::stringstream str;

        if ( _index == 0 )
        {
            std::string type = getStmtType(_type);

            str << "const char* m_paramValues[s_" << type << "ParamCount];\n";
            str << "int m_paramLengths[s_" << type << "ParamCount];\n";
            str << "int m_paramFormats[s_" << type << "ParamCount];\n";
        }

        PGTypePair type = getPgTypes(_item->type);

        str << type.lang << " m_param" << _item->name << ";\n";

        return str.str();
    }
}

void PostgreSQLGenerator::addOutBuffers(SQLStatementTypes _type, const AbstractIOElements* _elements)
{
    TemplateDictionary *subDict;
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

    std::string fb, lang;

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

