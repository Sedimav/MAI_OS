#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "common.h"

int is_english_vowel(char c) {
    if (c >= 'A' && c <= 'Z') c = c + ('a' - 'A');
    return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' || c == 'y');
}

int is_russian_vowel_utf8(const char* str, size_t pos, size_t len) {
    if (pos + 1 >= len) return 0;
    unsigned char b1 = (unsigned char)str[pos];
    unsigned char b2 = (unsigned char)str[pos + 1];
    
    if (b1 == 0xD0) {
        if ((b2 == 0x90 || b2 == 0xB0) ||
            (b2 == 0x95 || b2 == 0xB5) ||
            (b2 == 0x81 || b2 == 0x91) ||
            (b2 == 0x98 || b2 == 0xB8) ||
            (b2 == 0x9E || b2 == 0xBE) ||
            (b2 == 0xA3) ||
            (b2 == 0xAB || b2 == 0xBB) ||
            (b2 == 0xAD || b2 == 0xBD) ||
            (b2 == 0xAE) ||
            (b2 == 0xAF || b2 == 0xBF)) {
            return 1;
        }
    }
    return 0;
}

int is_vowel_check(const char* str, size_t pos, size_t len, size_t* char_len) {
    if (pos >= len) {
        *char_len = 0;
        return 0;
    }
    
    unsigned char fb = (unsigned char)str[pos];
    
    if (fb < 0x80) {
        *char_len = 1;
        return is_english_vowel(str[pos]);
    }
    else if ((fb & 0xE0) == 0xC0) {
        if (pos + 1 < len) {
            *char_len = 2;
            return is_russian_vowel_utf8(str, pos, len);
        }
        *char_len = 1;
        return 0;
    }
    
    *char_len = 1;
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        write(STDERR_FILENO, "Usage: ./child <file> <shm> <sem> <id>\n", 40);
        _exit(1);
    }
    
    char* outfile = argv[1];
    char* shm_name = argv[2];
    char* sem_name = argv[3];
    int my_id = (argv[4][0] == '1') ? 1 : 2;
    
    int shm_fd = shm_open(shm_name, O_RDWR, 0666);
    if (shm_fd == -1) {
        write(STDERR_FILENO, "shm_open failed\n", 16);
        _exit(1);
    }
    
    SharedBuffer* buf = mmap(NULL, sizeof(SharedBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (buf == MAP_FAILED) {
        write(STDERR_FILENO, "mmap failed\n", 12);
        close(shm_fd);
        _exit(1);
    }
    
    sem_t* sem = sem_open(sem_name, 0);
    if (sem == SEM_FAILED) {
        write(STDERR_FILENO, "sem_open failed\n", 16);
        munmap(buf, sizeof(SharedBuffer));
        close(shm_fd);
        _exit(1);
    }
    
    int out_fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd == -1) {
        write(STDERR_FILENO, "open failed\n", 12);
        sem_close(sem);
        munmap(buf, sizeof(SharedBuffer));
        close(shm_fd);
        _exit(1);
    }
    
    while (1) {
        sem_wait(sem);
        
        if (buf->ready && buf->target == my_id) {
            char result[SHARED_BUFFER_SIZE];
            size_t out_idx = 0;
            
            for (size_t i = 0; i < buf->len; ) {
                size_t clen = 0;
                if (is_vowel_check(buf->data, i, buf->len, &clen)) {
                    i += clen;
                } else {
                    if (clen == 0) clen = 1;
                    size_t to_copy = clen;
                    if (i + to_copy > buf->len) to_copy = buf->len - i;
                    for (size_t j = 0; j < to_copy; j++) {
                        result[out_idx++] = buf->data[i++];
                    }
                }
            }
            
            if (out_idx > 0) {
                result[out_idx] = '\n';
                write(out_fd, result, out_idx + 1);
            }
            
            buf->ready = 0;
        }
        
        int exit_flag = (buf->target == -1);
        sem_post(sem);
        
        if (exit_flag) break;
        usleep(1000);
    }
    
    close(out_fd);
    sem_close(sem);
    munmap(buf, sizeof(SharedBuffer));
    close(shm_fd);
    
    return 0;
}
