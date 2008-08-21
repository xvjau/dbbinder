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

namespace DBBinder {

FirebirdGenerator::FirebirdGenerator()
 : AbstractGenerator()
{
}


FirebirdGenerator::~FirebirdGenerator()
{
}


bool FirebirdGenerator::checkConnection()
{
    return AbstractGenerator::checkConnection();
}

String FirebirdGenerator::getBind(const ListElements::iterator& _item, int _index)
{
}

String FirebirdGenerator::getReadValue(const ListElements::iterator& _item, int _index)
{
}

void FirebirdGenerator::addSelect(SelectElements _elements)
{
    AbstractGenerator::addSelect(_elements);
}

}
