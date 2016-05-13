#include "util.h"

using namespace std;

// ----------------------------------------------------------------------
// 比较一下不同方式遍历 vector 的性能, typical resutl:
// 
// (1). 50519.9µs
// (2). 79243.7µs
// (3). 50520.5µs
// (4). 50406.2µs
//
// 没想到 (2) 这么慢
// ----------------------------------------------------------------------
int main(int argc, char *argv[])
{
    size_t sz = 100000000;
    vector<int> rows(sz);
    uint32_t i = 0;

    // (1)
    {
        Timing t;
        for_each(rows.begin(), rows.end(), [&i](int &e){e=i++;});
    }
    cout << i << endl;

    // (2)
    {
        Timing t;
        i = 0;
        for (auto j = 0; j < rows.size(); ++j) {
            rows[j] = i++;
        }
    }
    cout << i << endl;

    // (3)
    {
        Timing t;
        i = 0;
        for (auto &r : rows) {
            r = i++;
        }
    }
    cout << i << endl;

    // (4)
    {
        Timing t;
        i = 0;
        for (auto it = rows.begin(); it != rows.end(); ++it) {
            *it = i++;
        }
    }
    cout << i << endl;

    return 0;
}

