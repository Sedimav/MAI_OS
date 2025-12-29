#ifndef COMMON_H
#define COMMON_H

#define SHARED_BUFFER_SIZE 1024

typedef struct {
    char data[SHARED_BUFFER_SIZE];
    size_t len;
    int target;
    int ready;
} SharedBuffer;

#endif
