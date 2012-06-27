TARGET        = example_firebird

default_target: $(TARGET)

CXX           = g++
LINK          = g++
CFLAGS        = -pipe -g -DDEBUG $$(fb_config --cflags)
INCPATH       = -I/usr/include -I/usr/local/include
LIBS          = $$(fb_config --libs)

test.o: test.xml ../../dbbinder
	../../dbbinder -i test.xml -d ../../templates
	$(CXX) -c $(CFLAGS) $(INCPATH) -o test.o -c test.cpp

main.o: main.cpp test.o
	$(CXX) -c $(CFLAGS) $(INCPATH) -o main.o -c main.cpp

$(TARGET): main.o test.o
	$(LINK) $(LFLAGS) -o $(TARGET) main.o test.o $(LIBS)

clean:
	rm main.o test.o test.h test.cpp $(TARGET)
