#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include "contract.h"

static float (*sin_integral_ptr)(float, float, float) = NULL;
static char* (*convert_ptr)(int) = NULL;

static void *integral_lib = NULL;
static void *conv_lib = NULL;

const char *integral_libs[2] = {"./libsin1.so", "./libsin2.so"};
const char *conv_libs[2] = {"./libconv1.so", "./libconv2.so"};

int load_version(int version) {
    if (integral_lib) dlclose(integral_lib);
    if (conv_lib) dlclose(conv_lib);

    integral_lib = dlopen(integral_libs[version], RTLD_LAZY);
    conv_lib = dlopen(conv_libs[version], RTLD_LAZY);

    sin_integral_ptr = (float (*)(float, float, float)) dlsym(integral_lib, "sin_integral");
    convert_ptr = (char* (*)(int)) dlsym(conv_lib, "convert");

    printf("Реализация №%d\n", version + 1);
    return 0;
}

int main() {
    printf("Программа №2\n0 — переключить\n1 A B e — интеграл\n2 x — перевод\n");

    int current_version = 0;
    load_version(current_version);

    char line[256];
    while (fgets(line, sizeof(line), stdin)) {
        line[strcspn(line, "\n")] = '\0';
        if (strcmp(line, "exit") == 0) break;

        int cmd;
        if (sscanf(line, "%d", &cmd) != 1) continue;

        if (cmd == 0) {
            current_version = 1 - current_version;
            load_version(current_version);
        } else if (cmd == 1) {
            float a, b, e;
            if (sscanf(line, "%*d %f %f %f", &a, &b, &e) == 3) {
                float res = sin_integral_ptr(a, b, e);
                printf("sin = %.6f (реал. №%d)\n", res, current_version + 1);
            }
        } else if (cmd == 2) {
            int x;
            if (sscanf(line, "%*d %d", &x) == 1) {
                char *s = convert_ptr(x);
                if (s) {
                    printf("convert(%d) = %s (реал. №%d)\n", x, s, current_version + 1);
                    free(s);
                }
            }
        }
    }

    if (integral_lib) dlclose(integral_lib);
    if (conv_lib) dlclose(conv_lib);
    return 0;
}
