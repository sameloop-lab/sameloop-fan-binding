// SPDX-License-Identifier: MIT
#ifndef SAMELOOP_FAN_MODEL_H
#define SAMELOOP_FAN_MODEL_H
#include <math.h>
typedef struct { double requested, applied, rpm, last, command_time; } sl_fan;
static inline void fan_interval(sl_fan *f, double dt, double target) {
    if (dt <= 0) return;
    /* Exact solution of two cascaded first-order responses: tau=0.6, 0.8 s. */
    double a = exp(-dt / 0.6), b = exp(-dt / 0.8);
    double previous = f->applied;
    f->applied = target + (previous - target) * a;
    f->rpm = 40 * target + (f->rpm - 40 * target) * b
           + 40 * (previous - target) * 0.6 / (0.6 - 0.8) * (a - b);
}
static inline void fan_advance(sl_fan *f, double now) {
    if (now <= f->last) return;
    double expiry = f->command_time + 1.0;
    double mid = fmin(now, fmax(f->last, expiry));
    fan_interval(f, mid - f->last, f->requested);
    if (now >= expiry) {
        f->requested = 0;
        fan_interval(f, now - mid, 0);
    }
    f->last = now;
}
#endif
