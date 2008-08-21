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

#include "mysqlgenerator.h"

namespace DBBuilder {

MySQLGenerator::MySQLGenerator()
 : AbstractGenerator()
{
}


MySQLGenerator::~MySQLGenerator()
{
}


bool MySQLGenerator::checkConnection()
{
    return AbstractGenerator::checkConnection();
}

String MySQLGenerator::getBind(const ListElements::iterator& _item, int _index)
{
}

String MySQLGenerator::getReadValue(const ListElements::iterator& _item, int _index)
{
}

void MySQLGenerator::addInsert(InsertElements _elements)
{
    AbstractGenerator::addInsert(_elements);
}

void MySQLGenerator::addSelect(SelectElements _elements)
{
    AbstractGenerator::addSelect(_elements);
}

void MySQLGenerator::addUpdate(UpdateElements _elements)
{
    AbstractGenerator::addUpdate(_elements);
}

}
