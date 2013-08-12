CC=g++
CFLAGS=`pkg-config --cflags opencv` -g -Wall -I/usr/include/gsl/
LDFLAGS=`pkg-config  --libs opencv` -lgsl -lgslcblas -lv4l2
SOURCES=$(wildcard *.cpp)
OBJDIR=obj
OBJECTS=$(addprefix $(OBJDIR)/, $(SOURCES:.cpp=.o))
EXECUTABLE=console_measure

all: $(EXECUTABLE)
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
$(OBJDIR)/%.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@
clean:
	rm $(EXECUTABLE) $(OBJECTS)