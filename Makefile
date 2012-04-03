CC=g++
CFLAGS=-O0 -ggdb -Wall -c `wx-config --cppflags` `pkg-config --cflags gstreamer-0.10` 
LDFLAGS=`wx-config --libs` `pkg-config --libs gstreamer-0.10`

SRC=./src
BIN=./bin
DOC=./doc

# Object files
OBJECTS=$(BIN)/main.o\
        $(BIN)/audio.o\
        $(BIN)/dirbrowser.o\
        $(BIN)/streambrowser.o\
        $(BIN)/tracktable.o\
        $(BIN)/navigation.o\
		$(BIN)/misc.o

# Following targets build the source files.
.PHONY: all
all: init $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $(BIN)/navi

$(BIN)/main.o: $(SRC)/main.cpp $(SRC)/main.hpp
	$(CC) $(CFLAGS) $(SRC)/main.cpp -o $@

$(BIN)/audio.o: $(SRC)/audio.cpp $(SRC)/audio.hpp
	$(CC) $(CFLAGS) $(SRC)/audio.cpp -o $@

$(BIN)/dirbrowser.o: $(SRC)/dirbrowser.cpp $(SRC)/dirbrowser.hpp
	$(CC) $(CFLAGS) $(SRC)/dirbrowser.cpp -o $@

$(BIN)/streambrowser.o: $(SRC)/streambrowser.cpp $(SRC)/streambrowser.hpp
	$(CC) $(CFLAGS) $(SRC)/streambrowser.cpp -o $@

$(BIN)/tracktable.o: $(SRC)/tracktable.cpp $(SRC)/tracktable.hpp
	$(CC) $(CFLAGS) $(SRC)/tracktable.cpp -o $@

$(BIN)/navigation.o: $(SRC)/navigation.cpp $(SRC)/navigation.hpp
	$(CC) $(CFLAGS) $(SRC)/navigation.cpp -o $@

$(BIN)/misc.o: $(SRC)/misc.cpp $(SRC)/misc.hpp
	$(CC) $(CFLAGS) $(SRC)/misc.cpp -o $@



.PHONY: init
init:
	@mkdir -p $(BIN)

# Target: clean
# Purpose: cleans up generated binaries
#
.PHONY: clean
clean:
	rm -rf $(BIN)/*
