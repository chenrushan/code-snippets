// ============================================================
// File Name :
// Creation Date : 2016-10-26
// Last Modified : Wed 26 Oct 2016 04:16:32 PM CST
// Created By : ChenRushan
// Compile with: g++ -std=c++11 ...
// ============================================================

#include <vector>
#include <iostream>
#include <algorithm>

int main(int argc, char *argv[]) {
    // {{{ 在一个 string 数组对应的下标数组里找到 lower_bound
    std::vector<std::string> strs = {"a", "c", "d", "f", "h", "j"};
    std::vector<int> idxs = {0, 1, 2, 3, 4, 5};

    // comparison function return true if the first argument is less than
    // the second argument
    auto comp = [&](int idx,
                    const std::string &value) { return strs[idx] < value; };

    auto it = std::lower_bound(idxs.begin(), idxs.end(), "e", comp);
    std::cout << strs[*it] << std::endl;
    // }}}

    // {{{ 在数组上使用 lower_bound
    int arr[50];
    for (auto i = 0u; i < 50; ++i) {
        arr[i] = i;
    }
    int *p = arr + 1, *q = arr + 10;
    auto it2 = std::lower_bound(p, q, 10, [](int i, int j) { return i < j; });
    std::cout << *it2 << std::endl;
    // }}}

    return 0;
}
