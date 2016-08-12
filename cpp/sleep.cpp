#include <thread> // for this_thread
#include <chrono> // for chrono time

int main(int argc, char *argv[])
{
    // sleep for 1 second
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // sleep for 500 milli-second
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // sleep for 500 micro-second
    std::this_thread::sleep_for(std::chrono::microseconds(500));
    return 0;
}

