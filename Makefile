## We call bash
SHELL := /bin/bash

## Library sources
SRCDIR = src
LIBSRC = $(SRCDIR)/jsoncpp.cpp

## Header files
HEADERS = $(SRCDIR)/json/json.h

## Library objects
LIBDIR = lib
LIBOBJ = $(addprefix $(LIBDIR)/,$(notdir $(LIBSRC:.cpp=.o)))
LIBAUX = -lzmq

## Binary (executable)
BINDIR = bin
EXEC = logserver
BIN = $(BINDIR)/$(EXEC)

## Main source
SRC = $(SRCDIR)/$(EXEC).cpp

## Compiler
TIME = TIMEFORMAT="... compilation took %3Rs"; time
CC = $(TIME) g++
CC_FLAGS = -Wall -c -O3 -std=c++11
LD_FLAGS = -Wall -pthread -O3 -std=c++11

## Folder creation
DIRMAKER = @mkdir -p $(@D)

.PHONY: all clean

all: $(BIN)

## The binary depends on the library binaries
$(BIN): $(SRC) $(LIBOBJ)
	$(DIRMAKER)
	@echo "Compiling main code..."
	@$(CC) $(LD_FLAGS) -o $@ $^ $(LIBAUX)\
		&& echo "OK: $@ built!"\
		|| echo "ERROR: $@ fails!!"

$(LIBDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERS)
	$(DIRMAKER)
	@echo "Compiling $@..."
	@$(CC) $(CC_FLAGS) $< -o $@\
		&& echo "OK: $@ built!"\
		|| echo "ERROR: $@ fails!!"

clean:
	@rm -f err $(BIN) $(LIBOBJ)
