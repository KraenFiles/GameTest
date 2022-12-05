CC=g++
CFLAG=-c -Wall
SOURCES=main.cpp field.cpp service.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=main

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAG) $< -o $@

clear:
	rm -rf $(OBJECTS) $(EXECUTABLE)
