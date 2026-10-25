// SPDX-License-Identifier: MIT
#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"check failed: %s at line %d\n",#x,__LINE__); return EXIT_FAILURE; } } while (0)
int main(void) {

    sl_fan f={.last=10,.command_time=10,.requested=100};
    fan_advance(&f,10.5);
    CHECK(f.applied > 50 && f.applied < 60);
    CHECK(f.rpm > 500 && f.rpm < 1000);
    fan_advance(&f,11.1);
    CHECK(f.requested == 0); // expiry even if nobody read at exactly one second
    fan_advance(&f,30);
    CHECK(f.applied < 1e-9 && f.rpm < 1e-6);
    // Advancing with subdivisions must match the analytical interval solution.
    sl_fan a={.last=10,.command_time=10,.requested=70}, b=a;
    fan_advance(&a,12);
    for(int i=1;i<=200;i++) fan_advance(&b,10+i*0.01);
    CHECK(fabs(a.applied-b.applied)<1e-8 && fabs(a.rpm-b.rpm)<1e-8);

    puts("model tests passed"); return 0;
}
