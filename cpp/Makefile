all: split_string boost_string timing mmap_usage date_usage

split_string: split_string.cpp
	g++ -o $@ $^

boost_string: boost_string.cpp
	g++ -o $@ $^

rapidjson_usage: rapidjson_usage.cpp
	g++ -o $@ $^

timing: timing.cpp
	g++ -o $@ $^

mmap_usage: mmap_usage.cpp
	g++ -o $@ $^

date_usage: boost_date_arithmatic.cpp
	g++ -o $@ $^ -lboost_date_time
