#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <time.h>
#include "common.h"

void cleanup(sem_t* sem, const char* sem_name, SharedBuffer* buf, int shm_fd, const char* shm_name) {
    if (sem != SEM_FAILED) {
        sem_close(sem);
        sem_unlink(sem_name);
    }
    
    if (buf != MAP_FAILED && buf != NULL) {
        munmap(buf, sizeof(SharedBuffer));
    }
    
    if (shm_fd != -1) {
        close(shm_fd);
    }
    
    shm_unlink(shm_name);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        const char* msg = "Usage: ./parent <file1> <file2>\n";
        write(STDERR_FILENO, msg, 30);
        _exit(1);
    }
    
    char* file1 = argv[1];
    char* file2 = argv[2];
    pid_t my_pid = getpid();
    
    char shm_name[64];
    char sem_name[64];
    
    strcpy(shm_name, "/lab3_shm_");
    char pid_str[16];
    char* p = pid_str;
    int temp = my_pid;
    do {
        *p++ = '0' + (temp % 10);
        temp /= 10;
    } while (temp > 0);
    *p = '\0';
    
    char* start = pid_str;
    char* end = p - 1;
    while (start < end) {
        char tmp = *start;
        *start = *end;
        *end = tmp;
        start++;
        end--;
    }
    
    strcat(shm_name, pid_str);
    
    strcpy(sem_name, "/lab3_sem_");
    strcat(sem_name, shm_name + 10);
    
    shm_unlink(shm_name);
    
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        write(STDERR_FILENO, "shm_open error\n", 15);
        _exit(1);
    }
    
    if (ftruncate(shm_fd, sizeof(SharedBuffer)) == -1) {
        write(STDERR_FILENO, "ftruncate error\n", 16);
        close(shm_fd);
        shm_unlink(shm_name);
        _exit(1);
    }
    
    SharedBuffer* buf = mmap(NULL, sizeof(SharedBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (buf == MAP_FAILED) {
        write(STDERR_FILENO, "mmap error\n", 11);
        close(shm_fd);
        shm_unlink(shm_name);
        _exit(1);
    }
    
    sem_unlink(sem_name);
    
    sem_t* sem = sem_open(sem_name, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        write(STDERR_FILENO, "sem_open error\n", 15);
        munmap(buf, sizeof(SharedBuffer));
        close(shm_fd);
        shm_unlink(shm_name);
        _exit(1);
    }
    
    sem_wait(sem);
    buf->ready = 0;
    buf->target = 0;
    buf->len = 0;
    sem_post(sem);
    
    pid_t child1 = fork();
    if (child1 == 0) {
        execl("./child", "./child", file1, shm_name, sem_name, "1", NULL);
        write(STDERR_FILENO, "exec error\n", 11);
        _exit(1);
    }
    
    pid_t child2 = fork();
    if (child2 == 0) {
        execl("./child", "./child", file2, shm_name, sem_name, "2", NULL);
        write(STDERR_FILENO, "exec error\n", 11);
        _exit(1);
    }
    
    sleep(1);
    
    srand(time(NULL) + getpid());
    
    char line[SHARED_BUFFER_SIZE];
    ssize_t bytes;
    
    while ((bytes = read(STDIN_FILENO, line, sizeof(line) - 1)) > 0) {
        if (line[bytes - 1] == '\n') {
            line[bytes - 1] = '\0';
            bytes--;
        }
        
        if (bytes > 0) {
            int r = rand() % 100;
            int target = (r < 80) ? 1 : 2;
            
            sem_wait(sem);
            
            size_t copy_len = bytes;
            if (copy_len >= SHARED_BUFFER_SIZE) copy_len = SHARED_BUFFER_SIZE - 1;
            
            memcpy(buf->data, line, copy_len);
            buf->data[copy_len] = '\0';
            buf->len = copy_len;
            buf->target = target;
            buf->ready = 1;
            
            sem_post(sem);
            
            int done = 0;
            while (!done) {
                usleep(1000);
                sem_wait(sem);
                if (buf->ready == 0) done = 1;
                sem_post(sem);
            }
        }
    }
    
    sem_wait(sem);
    buf->target = -1;
    buf->ready = 1;
    sem_post(sem);
    
    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);
    
    cleanup(sem, sem_name, buf, shm_fd, shm_name);
    
    return 0;
}
