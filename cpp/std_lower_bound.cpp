// ============================================================
// File Name :
// Creation Date : 2016-10-26
// Last Modified : Wed 26 Oct 2016 04:02:38 PM CST
// Created By : ChenRushan
// Purpose: 这个 demo 演示的是在一个 string 数组对应的下标数组里找到 lower_bound
// Compile with: g++ -std=c++11 ...
// ============================================================

#include <vector>
#include <iostream>
#include <algorithm>

int main(int argc, char *argv[]) {
    std::vector<std::string> strs = {"a", "c", "d", "f", "h", "j"};
    std::vector<int> idxs = {0, 1, 2, 3, 4, 5};

    // comparison function return true if the first argument is less than
    // the second argument
    auto comp = [&](int idx,
                    const std::string &value) { return strs[idx] < value; };

    auto it = std::lower_bound(idxs.begin(), idxs.end(), "e", comp);
    std::cout << strs[*it] << std::endl;

    return 0;
}
