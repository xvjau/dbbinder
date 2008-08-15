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

#ifndef DBBUILDERSQLITEGENERATOR_H
#define DBBUILDERSQLITEGENERATOR_H

#include "abstractgenerator.h"
#include <sqlite3.h>

namespace DBBuilder
{

class SQLiteGenerator : public AbstractGenerator
{
	friend class AbstractGenerator; //wtf??
	
	protected:
		SQLiteGenerator();
		virtual ~SQLiteGenerator();

		virtual bool checkConnection();

		sqlite3 *m_db;

		virtual String getBind(const ListElements::iterator& _item, int _index);
		virtual String getReadValue(const ListElements::iterator& _item, int _index);
	public:
		virtual void addSelect(SelectElements _elements);
};

}

#endif
