#include "util.h"

using namespace std;

// ----------------------------------------------------------------------
// 比较一下不同方式遍历 vector 的性能, typical resutl:
// 
// (1). 51450.3µs
// (2). 93288.5µs
// (3). 51779.4µs
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

    // (2)
    {
        Timing t;
        i = 0;
        for (auto j = 0; j < rows.size(); ++j) {
            rows[j] = i++;
        }
    }

    // (3)
    {
        Timing t;
        i = 0;
        for (auto &r : rows) {
            r = i++;
        }
    }

    return 0;
}

