// 这个 demo 展示了 future promise 的用法，同时展示了他们如何结合 boost thread
// pool 使用
// compile with -lboost_thread
#include <string>
#include <mutex>
#include <future>
#include <thread>

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
    // io_service::run() is a kind of "message loop", so it should block the
    // calling thread.
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

mutex mtx;

void run(const string &msg, shared_ptr<promise<string>> prom)
{
    {
        lock_guard<mutex> guard(mtx);
        // the output console is a shared resource
        cout << boost::this_thread::get_id() << ":" << msg << endl;
    }
    boost::this_thread::sleep(boost::posix_time::milliseconds(2000));
    {
        lock_guard<mutex> guard(mtx);
        // the output console is a shared resource
        cout << boost::this_thread::get_id() << ": complete" << endl;
    }
    ostringstream oss;
    oss << ">>>" << boost::this_thread::get_id() << ":" << msg;
    prom->set_value(oss.str());
}

// ----------------------------------------------------------------------
// promise 和 future 是执行 async call 时进行同步用的
// 
// 1. A std::future object internally stores a value that will be assigned in
//    future and it also provides a mechanism to access that value using get().
//    But if somebody tries to access this associated value of future through
//    get() function before it is available, then get() function will block
//    till value is not available.
//
// 2. 每个 promise object 对应一个 future object，future object 对 caller
//    有意义，而 promise object 对 callee 有意义，callee 会在完成给定任务后
//    调用 set_value() 设置 future 的 value，该 value 由 caller 调用 get()
//    得到
// 
// 使用 promise 和 future 的 work flow 参考 extra/promise.png
//
// ----------------------------------------------------------------------
// facebook 有个不错的 tutorial
// https://code.facebook.com/posts/1661982097368498/futures-for-c-11-at-facebook/
// ----------------------------------------------------------------------
int main(int argc, char *argv[])
{
    ThreadPool threadpool(10);
    vector<future<string>> futs;

    for (size_t i = 0; i < 60; ++i) {
        string msg = "foo" + to_string(i);

        // -----------------------------------------------------------------
        // 两点注意
        // 1. 这里为什么用 shared_ptr 呢？因为 boost::bind() 的参数需要是
        //    copyable 的，而 promise is not copyable，while shared_ptr is
        //    copyable
        // 2. promise 不需要像 future 一样放到 vector 里，因为 promise 主要
        //    是对 callee thread 有意义，对 caller 意义并不大
        // -----------------------------------------------------------------
        shared_ptr<promise<string>> prom(new promise<string>());
        futs.push_back(prom->get_future());

        threadpool.execute(boost::bind(run, msg, prom));
    }

    for (auto &f : futs) {
        // get() will block the caller until future value is set
        auto res = f.get();
        {
            lock_guard<mutex> guard(mtx);
            cout << res << endl;
        }
    }
    return 0;
}

