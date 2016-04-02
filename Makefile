CC=gcc
CFLAGS=-c -Wall -O0 -g -std=c99
#recommended options: -ffast-math -ftree-vectorize -march=core2 -mssse3 -O3
COPTS=
LDFLAGS= -L. -lz

ifeq ($(OS),Windows_NT)
RM = del /Q /F
CP = copy /Y
EXECUTABLE=faops.exe
ifdef ComSpec
SHELL := $(ComSpec)
endif
ifdef COMSPEC
SHELL := $(COMSPEC)
endif
else
RM = rm -rf
CP = cp -f
EXECUTABLE=faops
endif

SOURCES=faops.c
OBJECTS=$(SOURCES:.c=.o)


all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) ${COPTS} $(OBJECTS) $(LDFLAGS) -o $@

install: all
	$(CP) $(EXECUTABLE) $(HOME)/bin

.c.o:
	$(CC) ${COPTS} $(CFLAGS) $< -o $@

clean:
	$(RM) $(OBJECTS) $(EXECUTABLE)
