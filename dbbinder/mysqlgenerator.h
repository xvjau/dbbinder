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

#ifndef DBBUILDERMYSQLGENERATOR_H
#define DBBUILDERMYSQLGENERATOR_H

#include "abstractgenerator.h"

namespace DBBuilder {

/**
	@author 
*/
class MySQLGenerator : public AbstractGenerator
{
	public:
		MySQLGenerator();
		virtual ~MySQLGenerator();

		virtual void addInsert(InsertElements _elements);
		virtual void addSelect(SelectElements _elements);
		virtual void addUpdate(UpdateElements _elements);

	protected:
		virtual bool checkConnection();
		virtual String getBind(const ListElements::iterator& _item, int _index);
		virtual String getReadValue(const ListElements::iterator& _item, int _index);
};

}

#endif
