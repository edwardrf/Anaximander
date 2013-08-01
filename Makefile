CFLAGS=`pkg-config --cflags --libs opencv` -g -Wall

all: test01 test02 test03 test04 test05 measure console_measure

test01: test01.cpp
	g++ -otest01 test01.cpp $(CFLAGS)
test02: test02.cpp
	g++ -otest02 test02.cpp $(CFLAGS)
test03: test03.cpp
	g++ -otest03 test03.cpp $(CFLAGS)
test04: test04.cpp
	g++ -otest04 test04.cpp $(CFLAGS)
test05: test05.cpp
	g++ -otest05 test05.cpp $(CFLAGS)
measure: measure.o laser.o
	g++ -omeasure measure.o laser.o $(CFLAGS) -lgsl -lgslcblas
measure.o: measure.cpp laser.h
	g++ -c -g -omeasure.o measure.cpp
laser.o: laser.cpp laser.h
	g++ -c -g -olaser.o laser.cpp -lgsl -lgslcblas
console_measure: console_measure.o laser.o
	g++ -oconsole_measure console_measure.o laser.o $(CFLAGS) -lgsl -lgslcblas
measure.o: console_measure.cpp laser.h
	g++ -c -g -oconsole_measure.o console_measure.cpp
clean:
	rm test?? measure *.o