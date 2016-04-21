// compile with -lboost_thread
#include <string>
#include <mutex>

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

mutex mtx;

void run(const string &msg)
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
}

int main(int argc, char *argv[])
{
    ThreadPool threadpool(10);

    for (size_t i = 0; i < 100; ++i) {
        string msg = "foo" + to_string(i);
        threadpool.execute(boost::bind(run, msg));
    }

    sleep(10);
    return 0;
}

