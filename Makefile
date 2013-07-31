CFLAGS=`pkg-config --cflags --libs opencv` -g

all: test01 test02 test03 test04 test05 measure

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
measure: measure.cpp
	g++ -omeasure measure.cpp $(CFLAGS) -lgsl -lgslcblas
clean:
	rm test?? measure