// ----------------------------------------------------------------------
// example copied from https://accu.org/index.php/journals/442
// ----------------------------------------------------------------------

#include <iostream>

// ----------------------------------------------------------------------
// algorithm_selector 相当于一个 if-else 的功能，如果不用 algorithm_selector，
// 则 algorithm() 会实现为
// void algorithm(T& object) {
//    if (supports_optimised_implementation<T>::value) {
//    } else {
//    }
// }
// 从效率上讲，应该不差，因为这个 if 判断的结果在 compile time 就能知道，所以
// compiler 应该可以做优化，但是实现起来还是用个 algorithm_selector 更好些，
// 代码显得简介紧凑些
// ----------------------------------------------------------------------
template <bool b>
struct algorithm_selector {
    template <typename T>
    static void implementation(T& object) {
        std::cout << "default algorithm implementation" << std::endl;
    }
};

template <>
struct algorithm_selector<true> {
    template <typename T>
    static void implementation(T& object) {
        object.optimised_implementation();
    }
};

// 这个就是个 type trait 了，表示一个类型 T 是否支持优化的实现，默认为不支持
// 如果某个类型支持优化的实现，则需要专门为该类型定义一个 specialized
// supports_optimised_implementation，比如下面的 ObjectB
template <typename T>
struct supports_optimised_implementation {
    static const bool value = false;
};

// 封装掉选择调用那个实现的逻辑，这样对于用户更友好
template <typename T>
void algorithm(T& object) {
    algorithm_selector<
        supports_optimised_implementation<T>::value>::implementation(object);
}

// ============================================================

class ObjectB {
public:
    void optimised_implementation() {
        std::cout << "ObjectB's optimised_implementation" << std::endl;
    }
};

// 为 ObjectB 定制 supports_optimised_implementation 这个 trait
template <>
struct supports_optimised_implementation<ObjectB> {
    static const bool value = true;
};

class ObjectA {};

int main(int argc, char* argv[]) {
    // calls default implementation
    ObjectA a;
    algorithm(a);

    // calls ObjectB::optimised_implementation();
    ObjectB b;
    algorithm(b);
    return 0;
}
