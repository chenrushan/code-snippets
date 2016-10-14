// compile with -lboost_thread
#include <mutex>
#include <string>

#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

using namespace std;

// ===========================================================================
// NOTICE:
// 基于 boost::asio 的 thread pool 总体还是个比较重的工具，因为它涉及到调度，
// 如果你并不需要调度，只是需要几个 thread 跑跑函数，就直接用 std::thread
// ===========================================================================

class ThreadPool {
public:
    ThreadPool(size_t nthreads);
    ~ThreadPool();

    template <class F>
    void execute(F f);

private:
    // boost::asio::io_service for task/work dispatching.
    boost::asio::io_service service;
    // boost::thread_group for managing lifetime of threads
    boost::thread_group threadpool;

    // io_service::work is base class of all works that can posted to an
    // instance of io_service, for example when you are working with a socket
    // and start an asynchronous read, actually you are adding a work to the
    // io_service. So you normally never use work directly, but there is one
    // exception to this.
    // The io_service::run() will run operations as long as there are
    // asynchronous operations to perform. If, at any time, there are no
    // asynchronous operations pending (or handlers being invoked), the run()
    // call will return.
    // By creating the work object (I usually do it on the heap and a
    // shared_ptr),
    // the io_service considers itself to always have something pending, and
    // therefore the run() method will not return.
    boost::asio::io_service::work work;
};

ThreadPool::ThreadPool(size_t nthreads)
    // This will start the ioService processing loop. All tasks
    // assigned with ioService.post() will start executing.
    : work(service) {
    // The io_service functions run(), run_one(), poll() or poll_one() must be
    // called for the io_service to perform asynchronous operations on behalf
    // of a C++ program.
    // io_service::run() is a kind of "message loop", so it should block the
    // calling thread.
    // io_service::run() is thread-safe, when a work is posted to io_service,
    // some (random) thread will be scheduled (I guess it depends on thread
    // scheduling policy of the OS) and get the work and start executing,
    // and all other threads just keep pending there
    for (size_t i = 0; i < nthreads; ++i) {
        threadpool.create_thread(
            boost::bind(&boost::asio::io_service::run, &service));
    }
}

ThreadPool::~ThreadPool() {
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

template <class F>
void ThreadPool::execute(F f) {
    service.post(f);
}

// ======================================================================

mutex mtx;

void run(const string &msg) {
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
}

int main(int argc, char *argv[]) {
    ThreadPool threadpool(10);

    for (size_t i = 0; i < 100; ++i) {
        string msg = "foo" + to_string(i);
        threadpool.execute(boost::bind(run, msg));
    }

    sleep(10);
    return 0;
}
