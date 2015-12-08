// {{{ necessary headers
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
// }}}

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

// ---------------------------------------------------------------------------
// 可以用在如下 case
// 1. 创建新的 binary file
//    int len = 65536;
//    mem = mmap_file("binary_file", O_CREAT | O_RDWR, &len);
//    len 必须是 > 0 的值
//    如果 "binary_file" 已经存在，则会被 truncate 为 len 指定的长度
// 2. 修改 binary file，不改变文件长度
//    int len = 0;
//    mem = mmap_file("existed_binary_file", O_RDWR, &len);
//    返回后 len 设置为 "existed_binary_file" 当前长度
// 3. 修改 binary file，改变文件长度
//    int len = 32768;
//    mem = mmap_file("existed_binary_file", O_RDWR, &len);
//    返回后 "existed_binary_file" 会被 truncate 成 len 指定的长度
// 4. 仅仅只是以读的方式打开 binary file
//    int len = 0;
//    mem = mmap_file("existed_binary_file", O_RDONLY, &len);
//    返回后 len 设置为 "existed_binary_file" 当前长度
// ---------------------------------------------------------------------------
void *mmap_file(const char *file, int flags, size_t *len)
{
    assert(len != NULL);
    assert((flags & O_CREAT) && *len > 0);
    // 以读方式打开, *len 必须为 0
    assert((flags & O_RDONLY) && *len == 0);

    int fd = open(file, flags, 0660);
    if (fd == -1) {
        fprintf(stderr, "fail to open %s\n", file);
        return NULL;
    }
    int mmapflags = PROT_READ | (flags & (O_WRONLY | O_RDWR) ? PROT_WRITE : 0);
    size_t mmaplen = 0;
    void *mem = NULL;

    if (*len != 0 && ftruncate(fd, *len) != 0) {
        fprintf(stderr, "fail to truncate %s\n", file);
        close(fd);
        return NULL;
    }
    if (flags & O_CREAT) {
        mmaplen = *len;
    } else if (flags & (O_WRONLY | O_RDWR)) {
        if (*len != 0) {
            
        }
    } else {
    }

    mem = mmap(NULL, *len, mmapflags, MAP_SHARED, fd, 0);  
    if (mem == MAP_FAILED) {
        fprintf(stderr, "fail to map in file %s\n", file);
        close(fd);
        return NULL;
    }
    return mem;
}

int main(int argc, char *argv[])
{
    int fd = open("wtf", O_RDWR | O_CREAT);
    if (fd < 0) {
        fprintf(stderr, "fail to open wtf\n");
        exit(1);
    }
    
    return 0;
}

