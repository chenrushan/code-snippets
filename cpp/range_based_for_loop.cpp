#include <iostream>
#include <memory>

template<typename T>
class DumbArray {
private:
    // 用 template 为了避免 const 版本和 non-const 版本间的代码重复
    template<typename DumbArrayType, typename ElementType>
    class IteratorImpl {
        using itertype = IteratorImpl<DumbArrayType, ElementType>;
    public:
        IteratorImpl(DumbArrayType &array, int idx=0) : array_(array), idx_(idx) {}

        // ----------------------------------------------------------------------
        // 以下三个函数是必须定义的，才能使用 for(auto xx : xxs) {} 的语法
        // ----------------------------------------------------------------------
        itertype &operator++() {
            idx_ += 1;
            return *this;
        }
        // XXX: 参数里的 const 不要漏了，否则 it != da.end() 不能调用，因为
        // da.end() 在这里是个 rvalue
        bool operator!=(const itertype &other) const {
            return idx_ != other.idx_;
        }
        ElementType &operator*() const {
            return array_[idx_];
        }

    private:
        DumbArrayType &array_;
        int idx_ = 0;
    };

public:
    using Iterator = IteratorImpl<DumbArray, T>;
    using ConstIterator = IteratorImpl<const DumbArray, const T>;

    DumbArray(size_t sz) : size_(sz), array_(new int[sz]) {}

    const T& operator[](int i) const { return array_.get()[i]; }
    T& operator[](int i) {
        return const_cast<T&>(const_cast<const DumbArray&>(*this).operator[](i));
    }

    Iterator begin() {
        std::cout << "non-const begin" << std::endl;
        return Iterator(*this);
    }
    Iterator end() {
        return Iterator(*this, size_);
    }

    ConstIterator begin() const {
        std::cout << "const begin" << std::endl;
        return ConstIterator(*this);
    }
    ConstIterator end() const {
        return ConstIterator(*this, size_);
    }

private:
    std::unique_ptr<T> array_;
    size_t size_ = 0;
};

int main(int argc, char *argv[])
{
    DumbArray<int> da(3);
    da[0] = 0; da[1] = 1; da[2] = 2;

    // 三种 non-const 版本的遍历方式
    for (auto it = da.begin(); it != da.end(); ++it) {
        std::cout << *it << std::endl;
    }
    for (auto &i : da) {
        i *= 2;
    }
    for (auto i : da) {
        std::cout << i << std::endl;
    }

    // const 版本的遍历方式
    const auto &cda = da;
    for (auto &i : cda) {
        std::cout << i << std::endl;
    }
    return 0;
}

