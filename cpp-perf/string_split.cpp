#include "util.h"

#include <boost/utility/string_ref.hpp>

using namespace std;
using string_ref = boost::string_ref;

class string_splitter
{
    string_ref src_;
    string_ref sep_;
    mutable int pos_ = 0;
    mutable bool finished_ = false;

public:
    string_splitter(string_ref src, string_ref sep) : src_(src), sep_(sep) {}

    string_ref next() const {
        const int pos = src_.substr(pos_).find_first_of(sep_);
        const int old_pos = pos_;
        if (pos != -1)
            pos_ += pos + 1;
        else
            finished_ = true;
        return src_.substr(old_pos, pos);
    }

    string_ref rest() const { return src_.substr(pos_); }
    bool finished() const noexcept { return finished_; }
};

int set_units_lens(char *ln, size_t len, char **units, size_t *lens, size_t ncols)
{
#define MAX_URI_LEN 4096
    // {{{ seperate line into units
    units[0] = ln;
    uint32_t u = 1, i = 0, j = 0; // j: start of current unit
    for (i = 0; i < len; ++i) {
        if (ln[i] != '\t') {
            continue;
        }
        ln[i] = '\0';
        if (u >= ncols) {
            return -1;
        }
        units[u] = ln + i + 1;
        lens[u-1] = i - j;
        // check if uri too long
        if (units[u-1][0] == '<' && lens[u-1] > MAX_URI_LEN) {
            return -1;
        }
        j = i + 1;
        u += 1;
    }
    lens[u-1] = i - j;
    // check if uri too long
    if (units[u-1][0] == '<' && lens[u-1] > MAX_URI_LEN) {
        return -1;
    }
    if (u < ncols) {
        return -1;
    }
    // }}}

    return 0;
}

// ----------------------------------------------------------------------
// 这里比较读入一个文件并对行做切分的性能，比较三种切分
// 
//   * 基于 boost::string_ref 写一个 string_splitter
//   * 用 boost::split()
//   * 用自己实现的 set_units_lens()
//
// typical result:
// 
//   number of lines: 115265
//   979983µs
//   nunits (from string_splitter): 71003240
//   1.16864e+07µs
//   nunits (from boost::split): 71003240
//   1.1978e+06µs
//   nunits (from set_units_lens): 71003240
//   
// 结论：
//
//   * string_splitter 是最快的
//   * set_units_lens 性能和 string_splitter comparable
//   * boost::split() 一般慢十倍
// ----------------------------------------------------------------------

int main(int argc, char *argv[])
{
    ifstream inf("/home/data/engine/source-data/newkg/from_zhonghua/bigtable/company_reports.txt");
    string line;
    size_t nlines = 0, ncols = 0;

    while (getline(inf, line)) {
        nlines += 1;
        if (ncols == 0) {
            vector<string> units;
            boost::split(units, line, boost::is_any_of("\t"));
            ncols = units.size();
        }
    }
    cout << "number of lines: " << nlines << endl;

    size_t nunits = 0;
    {
        nunits = 0;
        inf.clear();
        inf.seekg(0);
        Timing start_timing;
        while (getline(inf, line)) {
            string_splitter sp(line, "\t");
            while (!sp.finished()) {
                sp.next();
                nunits += 1;
            }
        }
    }
    cout << "nunits (from string_splitter): " << nunits << endl;

    {
        nunits = 0;
        inf.clear();
        inf.seekg(0);
        Timing start_timing;
        while (getline(inf, line)) {
            vector<string> units;
            boost::split(units, line, boost::is_any_of("\t"));
            nunits += units.size();
        }
    }

    cout << "nunits (from boost::split): " << nunits << endl;

    auto units = unique_ptr<char *[]>(new char*[ncols]);
    auto lens = unique_ptr<size_t[]>(new size_t[ncols]);

    {
        nunits = 0;
        inf.clear();
        inf.seekg(0);
        Timing start_timing;
        while (getline(inf, line)) {
            set_units_lens((char*)line.c_str(), line.length(),
                    units.get(), lens.get(), ncols);
            nunits += ncols;
        }
    }

    cout << "nunits (from set_units_lens): " << nunits << endl;

    inf.clear();
    inf.seekg(0);
}

