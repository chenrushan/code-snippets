#include <vector>
#include <iostream>
#include <iterator>

typedef std::vector<int> Vi;
typedef std::vector<Vi> Vvi;

// Seems like you'd want a vector of iterators
// which iterate over your individual vector<int>s.
struct Digits {
    Vi::const_iterator begin;
    Vi::const_iterator end;
    Vi::const_iterator me;
};

typedef std::vector<Digits> Vd;
void cart_product(
    Vvi& out,  // final result
    Vvi& in)  // final result
{
    Vd vd;

    // Start all of the iterators at the beginning.
    for(Vvi::const_iterator it = in.begin();
        it != in.end();
        ++it) {
        Digits d = {(*it).begin(), (*it).end(), (*it).begin()};
        vd.push_back(d);
    }


    while(1) {

        // Construct your first product vector by pulling 
        // out the element of each vector via the iterator.
        Vi result;
        for(Vd::const_iterator it = vd.begin();
            it != vd.end();
            it++) {
            result.push_back(*(it->me));
        }
        out.push_back(result);

        // Increment the rightmost one, and repeat.

        // When you reach the end, reset that one to the beginning and
        // increment the next-to-last one. You can get the "next-to-last"
        // iterator by pulling it out of the neighboring element in your
        // vector of iterators.
        for(Vd::iterator it = vd.begin(); ; ) {
            // okay, I started at the left instead. sue me
            ++(it->me);
            if(it->me == it->end) {
                if(it+1 == vd.end()) {
                    // I'm the last digit, and I'm about to roll
                    return;
                } else {
                    // cascade
                    it->me = it->begin;
                    ++it;
                }
            } else {
                // normal
                break;
            }
        }
    }
}

