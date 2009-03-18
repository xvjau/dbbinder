TARGET        = example_sqlite

default_target: $(TARGET)

CXX           = g++
LINK          = g++
CFLAGS        = -pipe -g -DDEBUG
INCPATH       = -I/usr/include -I/usr/local/include -I/usr/include/boost-1_37
LIBS          = -lsqlite3
SQLITE        = sqlite3

test.db: test.sql
	$(SQLITE) -init test.sql test.db ".quit"

test.o: test.yaml test.db ../../dbbinder
	../../dbbinder --yaml -i test.yaml -d ../../templates
	$(CXX) -c $(CFLAGS) $(INCPATH) -o test.o -c test.cpp

main.o: main.cpp test.o
	$(CXX) -c $(CFLAGS) $(INCPATH) -o main.o -c main.cpp

$(TARGET): main.o test.o
	$(LINK) $(LFLAGS) -o $(TARGET) main.o test.o $(LIBS)

clean:
	rm main.o test.o test.db test.h test.cpp $(TARGET)
