default_target: testapp

CXX           = g++
LINK          = g++
CFLAGS        = -pipe -g -DDEBUG
INCPATH       = -I/usr/include -I/usr/local/include -I$(ORACLE_HOME)/include
LFLAGS        = -L$(ORACLE_HOME)/lib
LIBS          = -lsqlite3 -lmysqlclient -lociei
SQLITE        = sqlite3

test.db: test.sql
	$(SQLITE) -init test.sql test.db ".quit"

test.o: test.xml test.db ../dbbinder
	../dbbinder -i test.xml -d ../templates
	$(CXX) -c $(CFLAGS) $(INCPATH) -o test.o -c test.cpp

main.o: main.cpp test.o
	$(CXX) -c $(CFLAGS) $(INCPATH) -o main.o -c main.cpp

testapp: main.o test.o
	$(LINK) $(LFLAGS) -o testapp main.o test.o $(LIBS)

clean:
	rm main.o test.o test.db test.h test.cpp testapp
