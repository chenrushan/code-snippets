#ifndef _CHENRS_HASHTABLE_H_
#define _CHENRS_HASHTABLE_H_

#include <stdlib.h>
#include <stdint.h>

#include <string>

namespace chenrs {

class hashtable_t {
public:
    class hashfunc {
    public:
        virtual ~hashfunc() {}
        virtual uint64_t operator()(void *key, size_t len) = 0;
    };

    class kvfunc {
    public:
        virtual ~kvfunc() {}
        virtual int operator()(void *key, size_t klen, void *val, size_t vlen) = 0;
    };

public:
    hashtable_t();
    hashtable_t(size_t sz);
    ~hashtable_t();

    int init();
    int insert(void *key, size_t klen, void *val, size_t vlen);
    void *search(void *key, size_t klen) const;
    void dump(void *mem, off_t base=0) const;
    int load(void *mem, off_t base=0);

    // return size of memory needed to dump this hash table
    size_t dump_size() const;
    // return number of elements in hashtable
    size_t size() const;

    int for_all(kvfunc *kvf) const;

private:

    typedef int64_t pos_t;
    typedef uint16_t cont_len_t; // length of key and value of hashtable

    struct node_t {
        pos_t key, val;
        cont_len_t klen, vlen;
        pos_t next;
    };

    struct meta_t {
        char id[8]; // "HASH0001"
        size_t nnodes;
        size_t bucksz;
        size_t contlen;
        off_t buck_off;
        off_t cont_off;
    };

private:

    int grow();

private:
    hashfunc *hfunc;
    pos_t *buckets;
    char *content;
    size_t bucksz; // sizeof of buckets array
    size_t nnodes;
    size_t contsz; // size of content
    size_t contlen; // size of content being used
    bool ismmap; // for mmap hashtable, insert is not allowed
};

}

#endif
