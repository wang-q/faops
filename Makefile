CC=gcc
CFLAGS=-c -Wall -O2 -g -std=gnu99 -Wno-unused-function
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

.c.o:
	$(CC) ${COPTS} $(CFLAGS) $< -o $@

test: all
	@bats test --tap

clean:
	$(RM) $(OBJECTS) $(EXECUTABLE)
