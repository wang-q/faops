CC=gcc
CFLAGS=-c -Wall -O2 -g -std=c99 -Wno-unused-function
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

readme:
	echo "# faops: Operate fasta files\n" > README.md
	echo '```' >> README.md
	./faops help >> README.md
	echo '```' >> README.md

.c.o:
	$(CC) ${COPTS} $(CFLAGS) $< -o $@

clean:
	$(RM) $(OBJECTS) $(EXECUTABLE)
