#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void write_all(int fd, const char *buf, size_t len) {
    while (len > 0) {
        ssize_t w = write(fd, buf, len);
        if (w <= 0) _exit(1);
        buf += w;
        len -= w;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        write(STDERR_FILENO, "Usage: ./parent <file1> <file2>\n", 34);
        _exit(1);
    }

    int pipe1[2], pipe2[2];
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        write(STDERR_FILENO, "pipe failed\n", 12);
        _exit(1);
    }

    srand((unsigned int)time(NULL));

    pid_t pid1 = fork();
    if (pid1 == 0) {
        close(pipe1[1]); close(pipe2[0]); close(pipe2[1]);
        dup2(pipe1[0], STDIN_FILENO);
        close(pipe1[0]);
        execl("./child", "./child", argv[1], (char *)NULL);
        write(STDERR_FILENO, "exec child1 failed\n", 20);
        _exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        close(pipe2[1]); close(pipe1[0]); close(pipe1[1]);
        dup2(pipe2[0], STDIN_FILENO);
        close(pipe2[0]);
        execl("./child", "./child", argv[2], (char *)NULL);
        write(STDERR_FILENO, "exec child2 failed\n", 20);
        _exit(1);
    }

    close(pipe1[0]); close(pipe2[0]);

    char buf[1024];
    ssize_t n;
    while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
        if (rand() % 100 < 80) {
            write_all(pipe1[1], buf, n);
        } else {
            write_all(pipe2[1], buf, n);
        }
    }

    close(pipe1[1]); close(pipe2[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return 0;
}
