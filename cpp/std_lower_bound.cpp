// ============================================================
// File Name :
// Creation Date : 2016-10-26
// Last Modified : Wed 26 Oct 2016 05:54:24 PM CST
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
    // XXX: 要非常注意它的返回值，对于下面的调用
    //    std::lower_bound(arr, arr + 10, 10)
    // 返回值是 arr + 10，这是表示 lower_bound 没有找到你想要的元素，就像
    // 如果你用 vector，则返回的是 vector.end()
    int arr[10];
    for (auto i = 0u; i < 10; ++i) {
        arr[i] = i;
    }
    auto it2 =
        std::lower_bound(arr, arr + 10, 8, [](int i, int j) { return i < j; });
    std::cout << *it2 << std::endl;
    // }}}

    return 0;
}
