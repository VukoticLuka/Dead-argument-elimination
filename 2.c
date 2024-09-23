#include <stdio.h>

int f (int x, int y, int z) {
    return x * z;
}

int g (int a, int b) {
    return a + b;
}

int p (int* a) {
    return 42;
}

int main (int argc, char **argv) {
    printf("%d\n", f(2,3,4));
    printf("%d\n", g(3,3));
    int x;
    printf("%d\n", p(&x));
}