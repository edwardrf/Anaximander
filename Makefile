CC=g++
CFLAGS=`pkg-config --cflags opencv` -g -Wall -I/usr/include/gsl/
LDFLAGS=-lgsl -lgslcblas -lv4l2 `pkg-config --libs opencv` -lboost_thread
SOURCES=$(wildcard *.cpp)
OBJDIR=obj
OBJECTS=$(addprefix $(OBJDIR)/, $(SOURCES:.cpp=.o))
EXECUTABLE=console_measure

all: $(EXECUTABLE)
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
$(OBJDIR)/%.o: %.cpp
	$(CC) -c $< -o $@ $(CFLAGS)
clean:
	rm $(EXECUTABLE) $(OBJECTS)