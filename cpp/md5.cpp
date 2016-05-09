// link with -lcrypto
#include <openssl/md5.h>
#include <string.h>

#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;

int main(int argc, char *argv[])
{
    char buf[28] = "hello world";

    // ------
    // result 中直接存二进制结果
    unsigned char result[MD5_DIGEST_LENGTH];
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, buf, strlen(buf));
    MD5_Final(result, &ctx);
    // ------

    // 以 16 进制打印
    for(auto c: result) {
        printf("%02x", c);
    }
    printf("\n");
    return 0;
}

