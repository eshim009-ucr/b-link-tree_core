
SRC=$(wildcard *.c)
OBJ=$(subst .c,.o,$(SRC)) main.o
LDLIBS=-lgtest


all: test

%.o: %.c
	$(CC) -c -o $@ $^

main.o: main.cpp test.hpp
	$(CXX) -c -o $@ $<

test: $(OBJ)
	g++ -o $@ $^ $(LDLIBS)

clean:
	rm -f $(OBJ) test