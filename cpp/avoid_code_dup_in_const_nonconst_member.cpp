#include <iostream>
#include <vector>

class DumbArray {
public:
    DumbArray(std::vector<int> array) : array_(array) {}

    // 为避免 const 版本和 non-const 版本间代码的重复（如果函数逻辑复杂，
    // 同时维护两个版本会将来更新的话将来很容易带来问题），首先实现一个
    // const 版本，然后在 non-const 版本中调用 const 版本
    const int& operator[](int i) const {
        return array_[i];
    }
    int& operator[](int i) {
        // 这里有两个 const_cast
        //  1. const_cast<const DumbArray&>(*this) 将 this 转换为 const，这样
        //     调用 operator[] 时会是 const 版本的 operator[]
        //  2. const_cast<int&> 将 const 版本的 operator[] 的返回结果的
        //     const 去掉
        return const_cast<int&>(const_cast<const DumbArray&>(*this).operator[](i));
    }

private:
    std::vector<int> array_;
};

