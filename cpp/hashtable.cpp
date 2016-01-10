#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"

namespace chenrs {
    
#define CONT_INIT_SIZE 256
#define HTAB_INIT_BUK_SIZE 100

#define MULTILE_OF_128(n) (((n) + 127) & (~0x7F))
#define POS2PTR(pos) ((void*)(content + pos))
#define GROW_THRESH 0.75
#define HTAB_ID "HASH0001"

// ---------------------------------------------------------------------------

class normal_hash_t : public hashtable_t::hashfunc {
public:
    uint64_t operator()(void *key, size_t len) {
        unsigned hval = 0, g = 0;
        const char *str = (const char *)key;
        size_t l = 0;
        while (l < len) {
            l++;
            hval <<= 4;
            hval += (unsigned long) *str++;
            g = hval & ((unsigned long) 0xf << (32 - 4));
            if (g != 0) {
                hval ^= g >> (32 - 8);
                hval ^= g;
            }
        }
        return hval;
    }
};

// ---------------------------------------------------------------------------

class murmur_hash_t : public hashtable_t::hashfunc {
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

// ---------------------------------------------------------------------------

static int is_prime(size_t candidate)
{
    size_t divisor = 3;
    size_t square = divisor * divisor;
    while (square < candidate && (candidate % divisor)) {
        divisor++;
        square += 4 * divisor;
        divisor++;
    }
    return (candidate % divisor ? 1 : 0);
}

// ---------------------------------------------------------------------------

static size_t next_prime(size_t candidate)
{
    if (candidate < 10) {
        candidate = 10;
    }
    candidate |= 1;
    while (((size_t)-1) != candidate && !is_prime (candidate)) {
        candidate += 2;
    }
    return candidate;
}

// ---------------------------------------------------------------------------

hashtable_t::hashtable_t()
    : buckets(NULL), bucksz(0),
      contsz(0), content(NULL), contlen(0),
      ismmap(false), nnodes(0)
{
    hfunc = new murmur_hash_t();
    bucksz = next_prime(HTAB_INIT_BUK_SIZE);
}

// ---------------------------------------------------------------------------

hashtable_t::hashtable_t(size_t sz)
    : buckets(NULL), bucksz(0),
      contsz(0), content(NULL), contlen(0),
      ismmap(false), nnodes(0)
{
    hfunc = new murmur_hash_t();
    bucksz = next_prime(sz);
}

// ----------------------------------------------------------------------

hashtable_t::~hashtable_t()
{
    if (!ismmap) {
        free(buckets);
        free(content);
    }
    delete hfunc;
}

// ---------------------------------------------------------------------------

int hashtable_t::grow()
{
    size_t newbucksz = next_prime(bucksz << 1);
    // fprintf(stderr, "new bucket size: %lu\n", newbucksz);
    pos_t *newbucks = (pos_t *)calloc(sizeof(*newbucks), newbucksz);
    if (newbucks == NULL) {
        return -1;
    }
    memset(newbucks, 0xFF, sizeof(*newbucks) * newbucksz);

    for (off_t b = 0; b < bucksz; ++b) {
        for (pos_t nd = buckets[b]; nd != -1; ) {
            node_t *pn = (node_t*)POS2PTR(nd);
            pos_t next = pn->next;
            pos_t bkt = (*hfunc)(POS2PTR(pn->key), pn->klen) % newbucksz;
            pn->next = newbucks[bkt];
            newbucks[bkt] = nd;
            nd = next;
        }
    }

    free(buckets);
    buckets = newbucks;
    bucksz = newbucksz;
    return 0;
}

// ---------------------------------------------------------------------------

int hashtable_t::init()
{
    buckets = (pos_t *)calloc(sizeof(pos_t), bucksz);
    if (buckets == NULL) {
        return -1;
    }
    memset(buckets, 0xFF, bucksz * sizeof(*buckets));

    contsz = MULTILE_OF_128(CONT_INIT_SIZE);
    content = (char *)calloc(sizeof(*content), contsz);
    if (content == NULL) {
        return -1;
    }

    return 0;
}

// ---------------------------------------------------------------------------

int hashtable_t::insert(void *key, size_t klen, void *val, size_t vlen)
{
    if (key == NULL || val == NULL || klen == 0 || vlen == 0) {
        return -1;
    }
    // insert into a mmap'ed hashtable is not allowed currently
    if (ismmap) {
        return -1;
    }
    size_t nodesz = sizeof(node_t) + klen + vlen;
    size_t bkt = (*hfunc)(key, klen) % bucksz;

    // resize content if needed
    if (contsz < contlen + nodesz) {
        contsz = MULTILE_OF_128(nodesz + (contsz << 1));
        // fprintf(stderr, "new content size: %lu\n", contsz);
        content = (char *)realloc(content, contsz);
        if (content == NULL) {
            return -1;
        }
    }

    // create node_t
    node_t *nd = (node_t *)POS2PTR(contlen);
    nd->klen = klen;
    nd->vlen = vlen;
    nd->key = contlen + sizeof(node_t);
    nd->val = nd->key + klen;
    // fprintf(stderr, "key pos: %d contsz: %lu\n", nd->key, contsz);
    memcpy(POS2PTR(nd->key), key, klen);
    memcpy(POS2PTR(nd->val), val, vlen);

    // append to bucket
    nd->next = buckets[bkt];
    buckets[bkt] = contlen;

    contlen += nodesz;
    nnodes += 1;

    if ((double)(nnodes)/bucksz > GROW_THRESH) {
        if (grow() != 0) {
            return -1;
        }
    }

    return 0;
}

// ---------------------------------------------------------------------------

void *hashtable_t::search(void *key, size_t klen) const
{
    if (key == NULL || klen == 0) {
        return NULL;
    }
    size_t bkt = (*hfunc)(key, klen) % bucksz;
    pos_t nd = 0;

    for (nd = buckets[bkt]; nd != -1; nd = ((node_t*)(content+nd))->next) {
        node_t *pn = (node_t*)POS2PTR(nd);
        if (pn->klen != klen) {
            continue;
        }
        if (memcmp(POS2PTR(pn->key), key, klen) == 0) {
            return POS2PTR(pn->val);
        }
    }

    return NULL;
}

// ---------------------------------------------------------------------------

size_t hashtable_t::size() const
{
    return nnodes;
}

// ---------------------------------------------------------------------------

int hashtable_t::for_all(kvfunc *kvf) const
{
    for (off_t i = 0; i < bucksz; ++i) {
        if (buckets[i] == -1) {
            continue;
        }
        for (pos_t p = buckets[i]; p != -1; p = ((node_t*)(content+p))->next) {
            node_t *pn = (node_t*)POS2PTR(p);
            if ((*kvf)(POS2PTR(pn->key), pn->klen, POS2PTR(pn->val), pn->vlen) != 0) {
                return -1;
            }
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------

size_t hashtable_t::dump_size() const
{
    size_t sz = 0;
    sz += sizeof(meta_t);
    sz += sizeof(pos_t) * bucksz;
    sz += contlen;
    return sz;
}

// ---------------------------------------------------------------------------

void hashtable_t::dump(void *mem, off_t base) const
{
    meta_t *meta = (meta_t *)((char *)mem + base);

    // set meta
    sprintf(meta->id, "%s", HTAB_ID);
    meta->nnodes = nnodes;
    meta->bucksz = bucksz;
    meta->contlen = contlen;
    meta->buck_off = base + sizeof(meta_t);
    meta->cont_off = meta->buck_off + sizeof(pos_t) * bucksz;

    // copy buckets
    memcpy((char *)mem+meta->buck_off, buckets, sizeof(pos_t) * bucksz);

    // copy content
    memcpy((char *)mem+meta->cont_off, content, contlen);
}

// ---------------------------------------------------------------------------

int hashtable_t::load(void *mem, off_t base)
{
    meta_t *meta = (meta_t *)((char *)mem + base);

    if (memcmp(meta->id, HTAB_ID, strlen(HTAB_ID)) != 0) {
        return -1;
    }
    ismmap = true;
    nnodes = meta->nnodes;
    contlen = meta->contlen;
    bucksz = meta->bucksz;

    buckets = (pos_t *)((char *)mem + meta->buck_off);
    content = (char *)mem + meta->cont_off;

    return 0;
}

}

