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

#include <iostream>
#include <list>

#include "test.h"

using namespace std;
using namespace TestApp;
	
int main()
{
	{
		cout << "\nTest 1\n------\nEmployee ID:\t\tFull Name:\t\tProject:\t\tHire Date:\t\tSalary:\n";
		
		selFestival stmt(2, 0);
		
		for( selFestival::iterator it = stmt.begin(); it != stmt.end(); ++it )
		{
			cout << it->getidEmployee() << "\t\t"
					<< it->getfullName() << "\t\t"
					<< it->getprojName() << "\t\t"
					<< it->gethireDate() << "\t\t"
					<< it->getsalary() << endl;
		}
	}

	cout << "\n\nTest 2\n------\nEmployee ID:\t\tFull Name:\t\tProject:\t\tHire Date:\t\tSalary:\n";
	
	{
		selFestival stmt(1, 0);
		
		typedef std::list<selFestival::row> MyListType;
		MyListType myList;
		
		myList.assign( stmt.begin(), stmt.end() );
		
		for( MyListType::iterator it = myList.begin(); it != myList.end(); ++it )
		{
			cout << (*it)->getidEmployee() << "\t\t"
					<< (*it)->getfullName() << "\t\t"
					<< (*it)->getprojName() << "\t\t"
					<< (*it)->gethireDate() << "\t\t"
					<< (*it)->getsalary() << endl;
		}
	}

	cout << endl;
}
