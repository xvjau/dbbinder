default_target: testapp

CXX           = g++
LINK          = g++
CFLAGS        = -pipe -Wall -W
INCPATH       = -I/usr/include -I/home/gianni/Projetos/stb/libloretta/ -I/home/gianni/Projetos/stb/libloretta/include
LFLAGS        = 
LIBS          = -lsqlite3

test.o: test.xml
	../dbbuilder -i test.xml -d /home/gianni/Projetos/DBBuilder/templates
	$(CXX) -c $(CFLAGS) $(INCPATH) -o test.o -c test.cpp

main.o: main.cpp test.o
	$(CXX) -c $(CFLAGS) $(INCPATH) -o main.o -c main.cpp

testapp: main.o test.o
	$(LINK) $(LFLAGS) -o testapp main.o test.o $(LIBS)
