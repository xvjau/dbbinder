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

#ifndef DBBINDERMYSQLGENERATOR_H
#define DBBINDERMYSQLGENERATOR_H

#include "abstractgenerator.h"
#include <mysql/mysql.h>

namespace DBBinder
{

class MySQLGenerator : public AbstractGenerator
{
	friend class AbstractGenerator; //wtf??

	protected:
		MySQLGenerator();
		virtual ~MySQLGenerator();

		MYSQL *m_conn;

		virtual bool checkConnection();
		virtual String getBind(SQLStatementTypes _type, const ListElements::iterator& _item, int _index);
		virtual String getReadValue(SQLStatementTypes _type, const ListElements::iterator& _item, int _index);

		virtual bool   needIOBuffers() const;

		virtual void addSelInBuffers(const SelectElements* _select);
		virtual void addSelOutBuffers(const SelectElements* _select);

	public:
		virtual void addInsert(InsertElements _elements);
		virtual void addSelect(SelectElements _elements);
		virtual void addUpdate(UpdateElements _elements);

};

}

#endif
