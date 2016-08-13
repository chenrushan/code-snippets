#include "objectpool.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>

int main(int argc, char *argv[])
{
    OneWriterMultiReaderObjectPool<std::string> string_pool;

    // string_pool.add("hello");
    // string_pool.add("world");
    // string_pool.add("foo");
    // string_pool.add("bar");

    // string_pool.remove("world");
    // string_pool.remove("xxx");
    // string_pool.remove("hello");
    // string_pool.remove("foo");

    // string_pool.add("zee");
    // string_pool.remove("zee");
    // string_pool.add("kar");
    // string_pool.remove("kar");
    // string_pool.add("wee");
    // string_pool.add("shi");

    auto print_string = [](const std::string &str) {
        std::cout << str << std::endl;
        return 0;
    };
    auto print_string_pool = [&]() {
        while (true) {
            std::cerr << "=========" << std::endl;
            string_pool.for_each(print_string);
            std::this_thread::sleep_for(std::chrono::milliseconds(17));
        }
    };

    std::thread(print_string_pool).detach();

    // string_pool.for_each(print_string);

    for (auto i = 0; i < 2000; ++i) {
        std::ostringstream ss;
        ss << "ninja " << i;
        string_pool.add(ss.str());
        std::this_thread::sleep_for(std::chrono::milliseconds(13));
    }
    for (auto i = 0; i < 2000; ++i) {
        if (i % 3 == 0) {
            continue;
        }
        std::ostringstream ss;
        ss << "ninja " << i;
        string_pool.remove(ss.str());
    }
    for (auto i = 2000; i < 2500; ++i) {
        std::ostringstream ss;
        ss << "ninja " << i;
        string_pool.add(ss.str());
        std::this_thread::sleep_for(std::chrono::milliseconds(13));
    }
    std::cerr << "major thread out" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    return 0;
}

