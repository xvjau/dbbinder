default_target: testapp

CXX           = g++
LINK          = g++
CFLAGS        = -pipe -Wall -W
INCPATH       = -I/usr/include -I/usr/local/include
LFLAGS        = 
LIBS          = -lsqlite3
SQLITE        = sqlite3

test.db: test.sql
	$(SQLITE) -init test.sql test.db

test.o: test.xml test.db
	../dbbinder -i test.xml -d ../templates
	$(CXX) -c $(CFLAGS) $(INCPATH) -o test.o -c test.cpp

main.o: main.cpp test.o
	$(CXX) -c $(CFLAGS) $(INCPATH) -o main.o -c main.cpp

testapp: main.o test.o
	$(LINK) $(LFLAGS) -o testapp main.o test.o $(LIBS)
