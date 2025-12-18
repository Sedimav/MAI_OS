#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contract.h"

int main() {
    printf("Программа №1\n");
    printf("0 — информация\n1 A B e — интеграл sin\n2 x — перевод\n");

    char line[256];
    while (fgets(line, sizeof(line), stdin)) {
        if (strncmp(line, "exit", 4) == 0) break;

        int cmd;
        if (sscanf(line, "%d", &cmd) != 1) continue;

        if (cmd == 0) {
            printf("sin_integral: прямоугольники\nconvert: двоичная\n");
        } else if (cmd == 1) {
            float a, b, e;
            if (sscanf(line, "%*d %f %f %f", &a, &b, &e) == 3) {
                float r = sin_integral(a, b, e);
                printf("∫sin = %.6f\n", r);
            }
        } else if (cmd == 2) {
            int x;
            if (sscanf(line, "%*d %d", &x) == 1) {
                char* s = convert(x);
                printf("convert(%d) = %s\n", x, s);
                free(s);
            }
        }
    }
    return 0;
}
