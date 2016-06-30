#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <random>
#include <functional>
#include <vector>
#include <string>
#include <atomic>
#include <memory>
#include <map>
#include <set>
#include <thread>
#include <algorithm>
#include <functional>
#include <future>
#include <string.h>
#include <boost/version.hpp>
#include <boost/utility/string_ref.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>
#include <boost/format.hpp>
#include <openssl/md5.h>

#include <glog/logging.h>

#include <iostream>
#include <iomanip>
#include <sstream>

#include "util.h"

using namespace std;  
 
void fun_throw()
{
    throw "hello";
}

int fun_return()
{
    return -1;
}

// ----------------------------------------------------------------------
// try catch 不是一般的慢啊，-O3 优化输出是这样
// 
//   throw
//   10000
//   40417.9µs
//   return
//   10000
//   6.675µs
//   
// try/catch 10000 次要花 40ms, 这在线上服务中是无法接受的，至少比较深的
// 逻辑是绝对不能用 try/catch 的，为了避免区分的麻烦，还是都不用了
// ----------------------------------------------------------------------
int main(int argc, char *argv[])
{
    {
        cout << "throw" << endl;
        Timing t;
        auto k = 0u;
        for (auto i = 0u; i < 10000; ++i) {
            try {
                fun_throw();
            } catch(...) {
                ++k;
            }
        }
        cout << k << endl;
    }

    {
        cout << "return" << endl;
        Timing t;
        auto k = 0u;
        for (auto i = 0; i < 10000; ++i) {
            if (fun_return() != 0) {
                ++k;
            }
        }
        cout << k << endl;
    }
    return 0;
}

