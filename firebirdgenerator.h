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

#ifndef DBBINDERFIREBIRDGENERATOR_H
#define DBBINDERFIREBIRDGENERATOR_H

#include "abstractgenerator.h"
#include <ibase.h>

namespace DBBinder {

/**
    @author
*/
class FirebirdGenerator : public AbstractGenerator
{
    public:
        FirebirdGenerator();
        virtual ~FirebirdGenerator();

        virtual void addSelect(SelectElements _elements);

    private:
        isc_db_handle	m_conn;

    protected:
        virtual bool checkConnection();
        virtual std::string getBind(SQLStatementTypes _type, const ListElements::iterator& _item, int _index);
        virtual std::string getReadValue(SQLStatementTypes _type, const ListElements::iterator& _item, int _index);
        virtual std::string getIsNull(SQLStatementTypes _type, const ListElements::iterator& _item, int _index);

        virtual bool   needIOBuffers() const;

        virtual void addInBuffers(SQLStatementTypes _type, const AbstractElements* _elements);
        virtual void addOutBuffers(SQLStatementTypes _type, const AbstractIOElements* _elements);

};

}

#endif
