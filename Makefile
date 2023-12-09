.DEFAULT_GOAL := all
SHELL := /bin/bash

EXEC = run
CXX = g++
ARGS = -O3 -pthread

all: main.o
	$(CXX) -o $(EXEC) main.o $(ARGS)

%.o: %.cpp
	$(CXX) -c $< $(ARGS)

clean:
	rm -f *.o $(EXEC)