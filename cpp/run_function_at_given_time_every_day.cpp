#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/date_time/local_time_adjustor.hpp>
#include <thread>
#include <chrono>
#include <functional>

// TODO: 这里时间是写死的
class RunEverydayTimer {
    using io_service_t = boost::asio::io_service;
    using ptime_t = boost::posix_time::ptime;
    using ptime_adjustor_t = boost::date_time::c_local_adjustor<ptime_t>;
    using time_duration_t = boost::posix_time::time_duration;
    using date_t = boost::gregorian::date;

public:

    using cb_func_t = std::function<void()>;
    RunEverydayTimer(io_service_t *io_service, cb_func_t func)
        : io_service_(io_service), timer_(*io_service), cb_func_(func) {
        timer_.expires_at(get_today_utc_time_at(hms_));
        timer_.async_wait(std::bind(&RunEverydayTimer::on_time_out,
                                    this, std::placeholders::_1));
    }

private:

    void on_time_out(const boost::system::error_code&) {
        cb_func_();

        // 重新注册第二天的 timer
        // 睡到第二天
        std::this_thread::sleep_for(std::chrono::seconds(10));
        timer_.expires_at(get_today_utc_time_at(hms_));
        timer_.async_wait(std::bind(&RunEverydayTimer::on_time_out,
                                    this, std::placeholders::_1));
    }

    time_duration_t utc_offset(const ptime_t& utc_time) {
        auto local_time = ptime_adjustor_t::utc_to_local(utc_time);
        return local_time - utc_time;
    }

    ptime_t get_today_utc_time_at(std::string hms) {
        date_t today = boost::gregorian::day_clock::local_day();
        auto usertime = boost::posix_time::duration_from_string(hms);
        ptime_t expirationtime(today, usertime);
        expirationtime -= utc_offset(expirationtime);
        return expirationtime;
    }

private:
    boost::asio::io_service *io_service_ = nullptr;
    boost::asio::deadline_timer timer_;
    ptime_t expirationtime_;
    cb_func_t cb_func_;
    std::string hms_ = "23:59:59";
};

void foo()
{
    namespace pt = boost::posix_time;
    std::cout << pt::to_iso_string(pt::second_clock::local_time()) << std::endl;
    std::cout << "hello world!" << std::endl;
}

int main(int argc, char *argv[])
{
    boost::asio::io_service io_service;
    RunEverydayTimer timer(&io_service, &foo);
    io_service.run();
    return 0;
}

