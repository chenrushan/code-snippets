#include "util.h"

#include <openssl/md5.h>

using namespace std;

class murmur_hash_t {
public:
    murmur_hash_t(unsigned int seed = 0) : seed(seed) {}

    uint64_t operator()(void *key, size_t len) {
        const uint64_t m = 0xc6a4a7935bd1e995;
        const int r = 47;
        uint64_t h = seed ^ (len * m);
        const uint64_t * data = (const uint64_t *)key;
        const uint64_t * end = data + (len/8);

        while (data != end) {
            uint64_t k = *data++;
            k *= m; 
            k ^= k >> r; 
            k *= m; 
            h ^= k;
            h *= m; 
        }

        const unsigned char * data2 = (const unsigned char*)data;
        switch(len & 7) {
        case 7: h ^= (uint64_t)(data2[6]) << 48;
        case 6: h ^= (uint64_t)(data2[5]) << 40;
        case 5: h ^= (uint64_t)(data2[4]) << 32;
        case 4: h ^= (uint64_t)(data2[3]) << 24;
        case 3: h ^= (uint64_t)(data2[2]) << 16;
        case 2: h ^= (uint64_t)(data2[1]) << 8;
        case 1: h ^= (uint64_t)(data2[0]);
                h *= m;
        };

        h ^= h >> r;
        h *= m;
        h ^= h >> r;
        return h;
    }

private:
    unsigned int seed;
};

uint32_t md5(char *buf, size_t len)
{
    unsigned char result[MD5_DIGEST_LENGTH];
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, buf, len);
    MD5_Final(result, &ctx);

    char res[4];
    res[0] = result[MD5_DIGEST_LENGTH-1];
    const size_t cnt = sizeof(res)/sizeof(res[0]);
    for (uint32_t i = 0; i < cnt; ++i) {
        res[i] = result[MD5_DIGEST_LENGTH - (cnt - i)];
    }
    return *(uint32_t *)res;
}

// 实现：
//   使用 murmur_hash 然后用 2^n 作为 hash bucket 大小，数据为整数
//
// 结论：
//   1. 总体来讲，在顺序数据和随机数据上都做了测试，最多的 bucket 中装的数大约是
//      最少的２倍，好像分布得还算均匀
//   2. 与 md5 hash 做了比较，效果基本差不多，在网上查了些资料，也说 murmur hash
//      在 balance 上并不比 crypto hash function 差
int main(int argc, char *argv[])
{
    murmur_hash_t mm;
    srand(time(0));

    size_t sz = 1 << 17;
    vector<int> bucks(sz);
    uint32_t num = 10000000;
    for (uint32_t i = 0; i < num; ++i) {
        int r = rand();
        bucks[md5((char *)&r, sizeof(r))&(sz-1)] += 1;
    }

    sort(bucks.begin(), bucks.end());

    for (auto b : bucks) {
        cout << b << endl;
    }

    return 0;
}

