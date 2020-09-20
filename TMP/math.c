#include <math.h>
#include <stdio.h>

#define RAND_MAX 077777

double log(double x) {
    if (x > 2) return -log(1 / x);
    double t = 1.0 - x;
    double res = 0.0;

    int n = 1;
    for (; n <= 20; ++n) {
        res = res - pow(t, n) / n;
    }

    return res;
}

double pow(double x, int y) {
    if (y == 0) return 1.0;

    if (y % 2 == 0) {
        return pow(x * x, y / 2);
    } else {
        return x * pow(x * x, (y - 1) / 2);
    }
}

double expdev(double lambda) {
    double dummy;
    do {
        dummy = (double) rand() / RAND_MAX;
    } while (dummy == 0.0);

    return -log(dummy) / lambda;
}