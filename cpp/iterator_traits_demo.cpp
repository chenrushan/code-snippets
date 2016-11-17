#include <iostream>
#include <iterator>
#include <vector>

// 关于 iterator_traits 参考 http://www.sgi.com/tech/stl/iterator_traits.html
template <typename Iterator>
typename std::iterator_traits<Iterator>::value_type sum(Iterator begin,
                                                        Iterator end) {
    using value_type = typename std::iterator_traits<Iterator>::value_type;
    value_type s = value_type();
    for (Iterator it = begin; it != end; it++) {
        s += *it;
    }
    return s;
}

int main(int argc, char *argv[]) {
    std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7};
    std::cout << sum(vec.begin(), vec.end()) << std::endl;
    return 0;
}
