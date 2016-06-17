#include <vector>
#include <string>
#include <utility>
#include <iostream>

using namespace std;

// 注意下面两版 wrapper 的区别, wrapper 在使用上要更方便，具体看下面的调用方式
template<typename... Argst>
void wrapper2(void (*func)(Argst...), Argst&&... args)
{
    (*func)(std::forward<Argst>(args)...);
}

template<typename... Argst, typename... Argst2>
void wrapper(void (*func)(Argst...), Argst2&&... args)
{
    (*func)(std::forward<Argst2>(args)...);
}

void print_string_vector(const vector<string> &strs)
{
    for (const auto &s : strs) {
        cout << s << endl;
    }
}

void add(int i, int j, int k)
{
    cout << "add: " << i + j + k << endl;
}

void modify_int(int &i)
{
    i = 10;
}

int main(int argc, char *argv[])
{
    vector<string> strs{"hello", "world", "123456", "haha"};

    // 注意到，调用 wrapper2 需要显式把 strs 转换成 (const vector<string> &)
    // 否则编译会提示错误
    wrapper(print_string_vector, strs);
    wrapper2(print_string_vector, (const vector<string> &)strs);

    wrapper(add, 1, 2, 3);
    int i = 3, j = 4, k = 5;
    wrapper(add, i, j, k);

    wrapper(modify_int, i);
    cout << "new i: " << i << endl;

    return 0;
}

