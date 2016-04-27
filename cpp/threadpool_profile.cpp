// compile with -lboost_thread
#include <string>
#include <mutex>
#include <future>
#include <vector>

#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

using namespace std;

class ThreadPool {
public:
    ThreadPool(size_t nthreads);
    ~ThreadPool();

    template<class F> void execute(F f);

private:
    // boost::asio::io_service for task/work dispatching.
    boost::asio::io_service service;
    // boost::thread_group for managing lifetime of threads
    boost::thread_group threadpool;
    boost::asio::io_service::work work;
};

ThreadPool::ThreadPool(size_t nthreads)
    // This will start the ioService processing loop. All tasks 
    // assigned with ioService.post() will start executing. 
    : work(service)
{
    // 创建 thread，每个 thread 都做相同的事，就是运行 io_service
    // io_service::run() is a kind of "message loop", so it should block the calling thread.
    for (size_t i = 0; i < nthreads; ++i) {
        threadpool.create_thread(boost::bind(&boost::asio::io_service::run, &service));
    }
}

ThreadPool::~ThreadPool()
{
    // This will stop the ioService processing loop. Any tasks
    // you add behind this point will not execute. 也就是说即便现在
    // io_service 的队列里有没有执行完的任务，它也会停止，所有还没有执行
    // 的任务就不会执行了，这个例子里如果 main() 没有 sleep 则基本上只有
    // @nthreads 个任务会被执行
    service.stop();

    // Will wait till all the treads in the thread pool are finished with 
    // their assigned tasks and 'join' them. Just assume the threads inside
    // the threadpool will be destroyed by this method.
    threadpool.join_all();
}

template<class F>
void ThreadPool::execute(F f)
{
    service.post(f);
}

// ======================================================================

// ======================================================================
// 这里构造了一组实验，主要目的是看看 boost thread pool 的 overhead 大概
// 是什么情况，实验内容是浮点数加和，首先是单线程浮点数加和，然后是使用
// 线程池进行浮点数加和
// 
// Output1 (加和 10亿次)
// ----------------------------------------------------------------------
//   single thread (1, 1.20284e+06µs)
//   multithreaded (1, 1.38383e+06µs)
//   multithreaded (10, 166591µs)
//   multithreaded (20, 85137.2µs)
//   multithreaded (30, 80019.3µs)
//   multithreaded (40, 65797.5µs)
//   multithreaded (50, 65719.7µs)
//   multithreaded (60, 64492.6µs)
//   multithreaded (70, 61568µs)
//   multithreaded (80, 61751µs)
//   multithreaded (90, 61758.8µs)
//   multithreaded (100, 59084.9µs)
//   
// Output2 (加和 10000 次)
// ----------------------------------------------------------------------
//   single thread (1, 13.561µs)
//   multithreaded (1, 264.493µs)
//   multithreaded (10, 603.259µs)
//   multithreaded (20, 875.298µs)
//   multithreaded (30, 1217.46µs)
//   multithreaded (40, 1394.33µs)
//   multithreaded (50, 1655.02µs)
//   multithreaded (60, 2107.26µs)
//   multithreaded (70, 2388.54µs)
//   multithreaded (80, 2781µs)
//   multithreaded (90, 3417.43µs)
//   multithreaded (100, 3304.91µs)
//
// 结论：
//   1. 基本上 thread pool 的代价还是比较小的
//   2. 在 thread pool 包含少于 20 个 thread 前，加更多的 thread 有一个
//      比较线性的结果
//   3. output2 里加 10000 次实际代价是很小的，所有可以认为 multithreaded
//      给出的实际就是 thread pool 多线程切换之类的工作消耗的时间，所以
//      如果你需要完成的任务需要时间超过 thread pool 自身消耗时间很多，
//      那用 thread pool 就会带来好处
// ======================================================================

typedef unsigned long long llu_t;

double add_to(llu_t total)
{
    double sum = 0;
    for (llu_t i = 0; i < total; ++i) {
        sum += 1.1;
    }
    return sum;
}

void run_single_thread(llu_t total, shared_ptr<promise<double>> prom)
{
    double sum = 0;
    for (llu_t i = 0; i < total; ++i) {
        sum += 1.1;
    }
    prom->set_value(sum);
}

double run_in_thread_pool(uint32_t nthreads, size_t nloops)
{
    ThreadPool threadpool(nthreads);
    vector<future<double>> results;
    llu_t total_for_each_thread = nloops/nthreads;

    for (uint32_t t = 0; t < nthreads; ++t) {
        shared_ptr<promise<double>> prom(new promise<double>());
        results.push_back(prom->get_future());
        threadpool.execute(boost::bind(run_single_thread, total_for_each_thread, prom));
    }

    double total = 0;
    for (uint32_t t = 0; t < nthreads; ++t) {
        total += results[t].get();
    }
    return total;
}

int main(int argc, char *argv[])
{
    size_t nloops = 10000;
    chrono::high_resolution_clock::time_point ct0, ct1;

    ct0 = chrono::high_resolution_clock::now();
    double s = add_to(nloops);
    ct1 = chrono::high_resolution_clock::now();
    fprintf(stderr, "s: %lf\n", s);
    cout << "single thread (1, " << chrono::duration<double, micro>(ct1-ct0).count() << "\u00B5s)\n";

    ct0 = chrono::high_resolution_clock::now();
    s = run_in_thread_pool(1, nloops);
    ct1 = chrono::high_resolution_clock::now();
    cout << "multithreaded (1, " << chrono::duration<double, micro>(ct1-ct0).count() << "\u00B5s)\n";
    fprintf(stderr, "s: %lf\n", s);

    uint32_t nthreads = 10;
    for (uint32_t i = 0; i < 10; ++i) {
        ct0 = chrono::high_resolution_clock::now();
        s = run_in_thread_pool(nthreads, nloops);
        ct1 = chrono::high_resolution_clock::now();
        cout << "multithreaded (" << nthreads << ", "
             << chrono::duration<double, micro>(ct1-ct0).count() << "\u00B5s)\n";
        fprintf(stderr, "s: %lf\n", s);
        nthreads += 10;
    }

    return 0;
}

