#ifndef _TIMING_H_
#define _TIMING_H_

#include <iostream>
#include <chrono>

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

