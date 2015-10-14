all: split_string

split_string: split_string.o
	g++ -o $@ $^

split_string.o: split_string.cpp
	g++ -c -o $@ $^

