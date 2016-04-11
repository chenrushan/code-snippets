BINS = split_string boost_string timing mmap_usage getopt_usage hash_profile random_number
all: $(BINS)

%: %.cpp
	g++ -O2 -std=c++11 -o $@ $^

.PHONY: clean
clean:
	rm -rf $(BINS)

