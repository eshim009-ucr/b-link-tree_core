SRC=$(wildcard *.c)
OBJ=$(subst .c,.o,$(SRC)) main.o
CFLAGS=-Wall
LDLIBS=-lgtest


all: test

tree.o: tree.c tree.h
io.o: io.c tree.h
%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

main.o: main.cpp test.hpp
	$(CXX) -c $(CFLAGS) -o $@ $<

test: $(OBJ)
	g++ -o $@ $^ $(LDLIBS)

clean:
	rm -f $(OBJ) test
