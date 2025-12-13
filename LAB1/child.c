#include <unistd.h>
#include <fcntl.h>
#include <string.h>

static const char* vowels[] = {
    "a", "e", "i", "o", "u",
    "A", "E", "I", "O", "U",
    "а", "е", "ё", "и", "о", "у", "ы", "э", "ю", "я",
    "А", "Е", "Ё", "И", "О", "У", "Ы", "Э", "Ю", "Я"
};
static const int num_vowels = sizeof(vowels) / sizeof(vowels[0]);

static int is_vowel_at(const char* buf, ssize_t len, ssize_t pos, ssize_t* out_len) {
    for (int i = 0; i < num_vowels; i++) {
        size_t vlen = strlen(vowels[i]);
        if (pos + (ssize_t)vlen <= len) {
            if (memcmp(&buf[pos], vowels[i], vlen) == 0) {
                *out_len = (ssize_t)vlen;
                return 1;
            }
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) _exit(1);

    int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) _exit(1);

    char buf[1024];
    char out[1024];
    ssize_t n;

    while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
        ssize_t i = 0;
        size_t j = 0;
        while (i < n) {
            ssize_t vlen = 0;
            if (is_vowel_at(buf, n, i, &vlen)) {
                i += vlen;
            } else {
                out[j++] = buf[i];
                i++;
            }
        }
        if (j > 0) write(fd, out, j);
    }

    close(fd);
    return 0;
}
