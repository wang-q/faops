CC=gcc
CFLAGS=-c -Wall -O0 -g
#recommended options: -ffast-math -ftree-vectorize -march=core2 -mssse3 -O3
COPTS=
LDFLAGS= -L. -lz
SOURCES=faops.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=faops

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) ${COPTS} $(OBJECTS) $(LDFLAGS) -o $@

install: all
	-cp $(EXECUTABLE) $(HOME)/bin

.c.o:
	$(CC) ${COPTS} $(CFLAGS) $< -o $@

clean:
	-rm $(OBJECTS) $(EXECUTABLE)

