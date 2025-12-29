#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

static int* g_arr = NULL;
static size_t g_size = 0;
static int g_max_threads = 1;
static int g_active_threads = 0;
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_cond = PTHREAD_COND_INITIALIZER;

typedef struct {
    size_t left;
    size_t right;
} task_t;

static inline void swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

static void* quicksort_worker(void* arg) {
    task_t* t = (task_t*)arg;
    size_t left = t->left;
    size_t right = t->right;

    typedef struct { size_t l, r; } stack_item_t;
    stack_item_t* stack = malloc(sizeof(stack_item_t) * 64);
    int stack_top = 0;
    stack[stack_top++] = (stack_item_t){left, right};

    while (stack_top > 0) {
        stack_item_t curr = stack[--stack_top];
        left = curr.l;
        right = curr.r;

        if (left >= right) continue;

        int pivot = g_arr[right];
        size_t i = left;
        for (size_t j = left; j < right; j++) {
            if (g_arr[j] <= pivot) {
                swap(&g_arr[i], &g_arr[j]);
                i++;
            }
        }
        swap(&g_arr[i], &g_arr[right]);

        if ((right - left) > 10000 && g_max_threads > 1) {
            pthread_mutex_lock(&g_mutex);
            if (g_active_threads < g_max_threads - 1) {
                g_active_threads++;
                pthread_mutex_unlock(&g_mutex);

                task_t* left_task = malloc(sizeof(task_t));
                left_task->left = left;
                left_task->right = i - 1;

                pthread_t left_thread;
                if (pthread_create(&left_thread, NULL, quicksort_worker, left_task) == 0) {
                    stack[stack_top++] = (stack_item_t){i + 1, right};
                    pthread_join(left_thread, NULL);

                    pthread_mutex_lock(&g_mutex);
                    g_active_threads--;
                    pthread_cond_signal(&g_cond);
                    pthread_mutex_unlock(&g_mutex);
                    continue;
                }
                free(left_task);
            }
            pthread_mutex_unlock(&g_mutex);
        }

        stack[stack_top++] = (stack_item_t){left, i - 1};
        stack[stack_top++] = (stack_item_t){i + 1, right};
    }

    free(stack);
    return NULL;
}

static void write_double(double val) {
    char tmp[64];
    int i = 0;
    long whole = (long)val;
    long frac = (long)((val - whole) * 100);

    if (whole == 0) tmp[i++] = '0';
    else {
        char rev[32]; int k = 0; long w = whole;
        while (w > 0) { rev[k++] = '0' + (w % 10); w /= 10; }
        while (k-- > 0) tmp[i++] = rev[k];
    }
    tmp[i++] = '.';
    tmp[i++] = '0' + (frac / 10);
    tmp[i++] = '0' + (frac % 10);
    tmp[i++] = '\n';
    write(STDOUT_FILENO, tmp, i);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        write(STDERR_FILENO, "Usage: ./quicksort <size> <max_threads>\n", 42);
        _exit(1);
    }

    size_t size = 0;
    for (const char* p = argv[1]; *p; p++) if (*p >= '0' && *p <= '9') size = size * 10 + (*p - '0');
    g_max_threads = 0;
    for (const char* p = argv[2]; *p; p++) if (*p >= '0' && *p <= '9') g_max_threads = g_max_threads * 10 + (*p - '0');
    if (g_max_threads < 1) g_max_threads = 1;

    g_arr = malloc(sizeof(int) * size);
    srand((unsigned int)time(NULL));
    for (size_t i = 0; i < size; i++) {
        g_arr[i] = rand() % 1000000;
    }
    g_size = size;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    task_t root = {0, size - 1};
    quicksort_worker(&root);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double t_par = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1e6;
    write_double(t_par);

    free(g_arr);
    return 0;
}
