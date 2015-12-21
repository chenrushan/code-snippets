#include <vector>
#include <stdio.h>

#include "hashtable.h"

using namespace chenrs;

// ---------------------------------------------------------------------------

#include <sys/time.h>

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

#include <iostream>
#include <fstream>

using namespace std;

// ---------------------------------------------------------------------------

class kv_printer_t : public hashtable_t::kvfunc {
public:
    int operator()(void *key, size_t klen, void *val, size_t vlen) {
        printf("%s ==> %u\n", (char *)key, *(uint32_t *)val);
        return 0;
    }
};

class id2str_mapper_t : public hashtable_t::kvfunc {
public:
    id2str_mapper_t(vector<const char *> *id2str) : id2str(id2str) {}

    int operator()(void *key, size_t klen, void *val, size_t vlen) {
        (*id2str)[*(uint64_t*)val] = (const char *)key;
        return 0;
    }

private:
    vector<const char *> *id2str;
};

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

void *mmap_file(const char *file, int flags, size_t *len)
{
    assert(len != NULL);
    // 以 O_CREAT 方式打开，*len 必须大于 0
    assert(!(flags & O_CREAT) || *len > 0);
    // 以读方式打开, *len 必须为 0
    assert(!(flags & O_RDONLY) || *len == 0);

    // open file
    int fd = open(file, flags, 0660);
    if (fd == -1) {
        fprintf(stderr, "fail to open %s\n", file);
        return NULL;
    }

    // truncate if needed
    if (*len != 0 && ftruncate(fd, *len) != 0) {
        fprintf(stderr, "fail to truncate %s to %lu\n", file, *len);
        close(fd);
        return NULL;
    }

    // set mmap length
    size_t mmaplen = 0;
    struct stat statbuf;
    if (fstat(fd, &statbuf) != 0) {
        close(fd);
        return NULL;
    }
    mmaplen = statbuf.st_size;

    // mmap in file
    int mmapflags = PROT_READ | (flags & (O_WRONLY | O_RDWR) ? PROT_WRITE : 0);
    void *mem = mmap(NULL, mmaplen, mmapflags, MAP_SHARED, fd, 0);  
    if (mem == MAP_FAILED) {
        fprintf(stderr, "fail to map in file %s\n", file);
        close(fd);
        return NULL;
    }

    // return
    if (*len == 0) {
        *len = mmaplen;
    }
    return mem;
}

void create_bin(const char *file)
{
    hashtable_t htab;
    htab.init();
    ifstream inf(file);
    string line;
    uint64_t id = 0;
    uint32_t nline = 0;

    timestamp_t t0 = get_timestamp();
    while (getline(inf, line)) {
        nline += 1;
        if (nline % 50000 == 0) {
            fprintf(stderr, "\r%d\n", nline);
        }
        void *val = htab.search((void *)line.c_str(), line.size() + 1);
        if (val != NULL) {
            continue;
        }
        if (htab.insert((void *)line.c_str(), line.size() + 1, &id, sizeof(id)) != 0) {
            fprintf(stderr, "fail to insert %s\n", line.c_str());
            exit(1);
        }
        id += 1;
    }
    timestamp_t t1 = get_timestamp();
    output_diff_time(t0, t1);

    size_t dsz = htab.dump_size();
    void *mem = mmap_file("out.bin", O_RDWR | O_CREAT, &dsz);
    htab.dump(mem);
}

int main(int argc, char *argv[])
{
    // create_bin(argv[1]);

    hashtable_t htab;
    void *mem = NULL;
    size_t sz = 0;

    timestamp_t t0 = get_timestamp();
    mem = mmap_file("out.bin", O_RDONLY, &sz);
    if (htab.load(mem) != 0) {
        fprintf(stderr, "fail to load\n");
        return -1;
    }
    timestamp_t t1 = get_timestamp();
    output_diff_time(t0, t1);
    // htab.for_all(new kv_printer_t());

    t0 = get_timestamp();
    vector<const char *> id2str(htab.size());
    id2str_mapper_t id2str_mapper(&id2str);
    htab.for_all(&id2str_mapper);
    t1 = get_timestamp();
    output_diff_time(t0, t1);

    for (uint32_t i = 0; i < id2str.size(); ++i) {
        printf("[%u] %s\n", i, id2str[i]);
    }

    return 0;
}

