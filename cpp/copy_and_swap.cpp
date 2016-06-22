#include <algorithm>
#include <cstddef>
#include <iostream>

// ----------------------------------------------------------------------
// 下面是我实现 copy and swap 的方式，我没有用 swap 操作，而是用了一个自定义的
// move 操作，因为 swap 需要 3 个赋值，而 move 只需要两个，有人说 compiler 会
// 优化 swap 为两次赋值，但从我写程序打印来看，并没有
// 
// ----------------------------------------------------------------------
// 
// 不过不管怎么实现，需要考虑的核心问题是
//
//   ** copy/move assignment should be exception safe **
//
// 所谓的 exception safe 就是如果 copy/move assignment 的过程中跑异常了，被
// assign 的那个 object 必须还是处于一个 valid state。为什么 copy/move assignment
// 需要特别考虑，而 copy/move ctor 不需要呢？因为在调用 copy/move ctor 时，
// object 还不存在，抛异常就抛异常了，没有任何影响，而在调用 copy/move assignment
// 的时候被 assign 的那个 object 已经存在了，必须保证在抛异常的情况还是处于一个
// valid state
//
// ----------------------------------------------------------------------
// 
// 总结一下下面实现的好处
//
//   1. exception safe
//   2. less type 
//        * by putting work into move_to_this() and free_resource()
//   3. faster than swap version
// ----------------------------------------------------------------------
class DumbArray {
public:
    DumbArray(std::size_t size = 0)
        : size_(size), array_(size_ ? new int[size_]() : nullptr) {
        std::cout << "normal ctor: " << this << " with size " << size_ << std::endl;
    }

    DumbArray(const DumbArray& that)
        : size_(that.size_), array_(size_ ? new int[size_] : nullptr) {
        std::cout << "copy ctor: " << this << std::endl;
        std::copy(that.array_, that.array_ + size_, array_);
    }

    DumbArray(DumbArray&& that) noexcept : DumbArray() {
        std::cout << "move ctor: " << this << std::endl;
        move_to_this(that);
    }

    ~DumbArray() {
        std::cout << "dtor: " << this << std::endl;
        free_resource();
    }

    // 关于 assignment 这里多说两句，之前我把 copy 和 move assignment 合并成
    // 一个函数，即
    //
    //   DumbArray& operator=(DumbArray that) {
    //       std::cout << "copy/move assignment: " << this
    //                 << " with that: " << &that << std::endl;
    //       move_to_this(that);
    //       return *this;
    //   }
    //   
    // 但后来发现有 noexcept 一说，还是把他们分开比较好

    DumbArray& operator=(const DumbArray &that) {
        std::cout << "copy assignment: " << this
                  << " with that: " << &that << std::endl;
        DumbArray temp(that);
        move_to_this(temp);
        return *this;
    }

    DumbArray& operator=(DumbArray &&that) noexcept {
        std::cout << "move assignment: " << this
                  << " with that: " << &that << std::endl;
        move_to_this(that);
        return *this;
    }

private:

    void move_to_this(DumbArray &that) noexcept {
        if (this != &that) {
            free_resource();
        }
        array_ = that.array_;
        size_ = that.size_;
        that.array_ = nullptr;
        that.size_ = 0;
    }

    // move_to_this() 和 dtor() 中同时需要处理 free resource 的功能
    void free_resource() noexcept {
        delete [] array_;
    }

private:
    std::size_t size_;
    int* array_;
};

int main(int argc, char *argv[])
{
    DumbArray arr2;

    std::cout << ">>> assignment\n";
    arr2 = DumbArray(20);

    std::cout << ">>> begin push\n";
    std::vector<DumbArray> dbarr_vec;

    std::cout << ">>> first push\n";
    dbarr_vec.push_back(DumbArray(20));
    std::cout << ">>> second push\n";
    dbarr_vec.push_back(DumbArray(10));
    std::cout << ">>> third push\n";
    dbarr_vec.push_back(DumbArray(30));

    std::cout << ">>> vector assignment\n";
    std::vector<DumbArray> dbarr_vec2 = dbarr_vec;

    std::cout << ">>> leave" << std::endl;

    return 0;
}

