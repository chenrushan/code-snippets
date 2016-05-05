#include "util.h"

using namespace std;

// 结果:
// 1. 如果 ss << 在前面，则其慢 20 倍
// 2. 如果 ss << 在后面，差不多慢 6 倍
// stringstream 确实慢
int main(int argc, char *argv[])
{
    const char *s1 = "<http://rdf.dinguf.com/ns/";
    const char *s2 = "xxx";
    const char *s3 = ">";

    char buf[4096];
    stringstream ss;

    {
        Timing t;
        ss << s1 << s2 << s3;
    }

    {
        Timing t;
        snprintf(buf, sizeof(buf)/sizeof(buf[0]), "%s%s%s", s1, s2, s3);
    }

    return 0;
}

