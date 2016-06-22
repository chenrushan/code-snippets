// from http://blogs.microsoft.co.il/sasha/2014/08/08/make-move-constructors-no-throw/
#include <iostream>
#include <vector>
#include <cstring>
#include <utility>

// 可以试试把下面的 noexcept 去掉和不去掉的效果，你会发现，没有 noexcept，
// 则 vector resize 的时候会调用 copy ctor，而有 noexcept，vector resize
// 会调用 move ctor
// 
// 基本原则是，进来把 move constructor 和 move assignment 都申明为 noexcept
class DumbString {
private:
    char* pstr_ = nullptr;
    unsigned len_ = 0;

public:
    DumbString(char const* str) : pstr_(strdup(str)), len_(strlen(str)) {}
    DumbString(DumbString const& rhs) : pstr_(strdup(rhs.pstr_)), len_(rhs.len_) {
        std::cerr << "in DumbString copy constructor" << std::endl;
    }
    DumbString(DumbString&& rhs) noexcept {
        std::cerr << "in DumbString move constructor" << std::endl;
        swap(rhs);
    }
    DumbString& operator=(DumbString const& rhs) {
        auto temp(rhs);
        swap(temp);
        return *this;
    }
    DumbString& operator=(DumbString&& rhs) noexcept {
        swap(rhs);
        return *this;
    }
    ~DumbString() {
        delete[] pstr_;
    }
    void swap(DumbString& other) {
        std::swap(pstr_, other.pstr_);
        std::swap(len_, other.len_);
    }
    char& operator[](unsigned i) { return pstr_[i]; }
    friend std::ostream& operator<<(std::ostream&, DumbString const&);
};

std::ostream& operator<<(std::ostream& os, DumbString const& str) {
    os << str.pstr_;
    return os;
}

int main()
{
    std::vector<DumbString> strings;
    for (int i = 0; i < 3; ++i) {
        std::cout << "push_back #" << i << ", capacity before = "
                  << strings.capacity() << ", size before = "
                  << strings.size() << std::endl;
        strings.push_back(DumbString{ "hello" });
    }
    return 0;
}

