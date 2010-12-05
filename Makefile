CC=g++
CFLAGS=-O0 -ggdb -Wall -c `wx-config --cppflags` `pkg-config --cflags gstreamer-0.10`
LDFLAGS=`wx-config --libs` `pkg-config --libs gstreamer-0.10`

SRC=./src
BIN=./bin
DOC=./doc

# Object files
OBJECTS=$(BIN)/main.o\
        $(BIN)/audio.o\
        $(BIN)/filetree.o\
        $(BIN)/tracktable.o

# Following targets build the source files.
.PHONY: all
all: init $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $(BIN)/navi

$(BIN)/main.o: $(SRC)/main.cpp $(SRC)/main.hpp
	$(CC) $(CFLAGS) $(SRC)/main.cpp -o $@

$(BIN)/audio.o: $(SRC)/audio.cpp $(SRC)/audio.hpp
	$(CC) $(CFLAGS) $(SRC)/audio.cpp -o $@

$(BIN)/filetree.o: $(SRC)/filetree.cpp $(SRC)/filetree.hpp
	$(CC) $(CFLAGS) $(SRC)/filetree.cpp -o $@

$(BIN)/tracktable.o: $(SRC)/tracktable.cpp $(SRC)/tracktable.hpp
	$(CC) $(CFLAGS) $(SRC)/tracktable.cpp -o $@


.PHONY: init
init:
	@mkdir -p $(BIN)

# Target: clean
# Purpose: cleans up generated binaries
#
.PHONY: clean
clean:
	rm -rf $(BIN)/*
