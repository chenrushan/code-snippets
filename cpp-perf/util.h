#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
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
#include <string.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include <boost/algorithm/string.hpp>

class Timing {
public:
    ~Timing() {
        std::cout << std::chrono::duration<double, std::micro>(
            std::chrono::high_resolution_clock::now()-now).count() << "\u00B5s\n";
    }
    Timing() {
        now = std::chrono::high_resolution_clock::now();
    }

private:
    std::chrono::high_resolution_clock::time_point now;
};

#endif

