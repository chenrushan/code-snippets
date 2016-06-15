// see http://rpg.ifi.uzh.ch/docs/glog.html
#include <glog/logging.h>

using namespace std;

int main(int argc, char *argv[])
{
    FLAGS_log_dir = "/tmp/xxxx/";
    google::InitGoogleLogging(argv[0]);

    LOG(ERROR) << "error happened";

    DLOG(ERROR) << "hello, world";

    return 0;
}

