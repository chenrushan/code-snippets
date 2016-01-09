#include <stdio.h>
#include <sys/time.h>
#include <chrono>
#include <iostream>

using namespace std;

typedef unsigned long long timestamp_t;

static timestamp_t get_timestamp()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
}

static void output_diff_time(timestamp_t beg, timestamp_t end)
{
    // time elapsed in microseconds
    double secs = (end - beg) / 1000000.0L;
    printf("[time elapsed]: %lf\n", secs);
}

int main(int argc, char *argv[])
{
    // begin
    timestamp_t t0 = get_timestamp();

    // do something
    for (int i = 0; i < 2000000; ++i) {
        printf("hello, world\n");
    }

    // end
    timestamp_t t1 = get_timestamp();
    output_diff_time(t0, t1);

    // {{{ 微秒级的 timing
    chrono::high_resolution_clock::time_point ct0, ct1;

    ct0 = chrono::high_resolution_clock::now();

    // do something

    ct1 = chrono::high_resolution_clock::now();

    cout << chrono::duration<double, micro>(ct1-ct0).count() << "\u00B5s\n";
    // }}}

    return 0;
}

