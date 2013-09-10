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

#ifndef DBBINDERSQLITEGENERATOR_H
#define DBBINDERSQLITEGENERATOR_H

#include "abstractgenerator.h"
#include <sqlite3.h>

namespace DBBinder
{

class SQLiteGenerator : public AbstractGenerator
{
friend class AbstractGenerator; //wtf??

protected:
    SQLiteGenerator();
    virtual ~SQLiteGenerator();

    virtual bool checkConnection() __C11_OVERRIDE;

    sqlite3 *m_db;

    sqlite3_stmt *execSQL(AbstractElements &_elements);

    virtual std::string getBind(SQLStatementTypes _type, const ListElements::iterator& _item, int _index) __C11_OVERRIDE;
    virtual std::string getReadValue(SQLStatementTypes _type, const ListElements::iterator& _item, int _index) __C11_OVERRIDE;
    virtual std::string getIsNull(SQLStatementTypes _type, const ListElements::iterator& _item, int _index) __C11_OVERRIDE;
public:
    virtual void addSelect(SelectElements _elements) __C11_OVERRIDE;
    virtual void addUpdate(UpdateElements _elements) __C11_OVERRIDE;
    virtual void addInsert(InsertElements _elements) __C11_OVERRIDE;
    virtual void addDelete(DeleteElements _elements) __C11_OVERRIDE;
};

}

#endif
