// SPDX-License-Identifier: MIT
#include "sl-common.h"
#include "model.h"
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static sl_fan fan;
static json_object *snapshot_locked(double now) {
    fan_advance(&fan, now);
    json_object *j = json_object_new_object();
    sl_double(j, "requested", fan.requested);
    sl_double(j, "applied", fan.applied);
    sl_double(j, "rpm", fan.rpm);
    sl_double(j, "monotonic_s", now);
    sl_bool(j, "watchdog_expired", now - fan.command_time >= 1.0);
    return j;
}
static void read_fan(afb_req_t req, unsigned n, afb_data_t const p[]) {
    if (!sl_empty(n,p)) { sl_error(req,-EINVAL,"read expects an empty object"); return; }
    pthread_mutex_lock(&lock);
    json_object *j = snapshot_locked(sl_now());
    pthread_mutex_unlock(&lock);
    sl_reply(req,0,j);
}
static void apply(afb_req_t req, unsigned n, afb_data_t const p[]) {
    json_object *j = sl_args(n,p);
    double duty;
    bool ok = j && json_object_object_length(j) == 1 && sl_number(j,"duty",0,100,&duty);
    json_object_put(j);
    if (!ok) { sl_error(req,-EINVAL,"expected {duty: number between 0 and 100}"); return; }
    pthread_mutex_lock(&lock);
    double now = sl_now();
    fan_advance(&fan,now);
    fan.requested = duty; fan.command_time = now;
    j = snapshot_locked(now);
    pthread_mutex_unlock(&lock);
    sl_reply(req,0,j);
}
static int ctl(afb_api_t api, afb_ctlid_t id, afb_ctlarg_t arg, void *data) {
    (void)api; (void)arg; (void)data;
    if (id == afb_ctlid_Init) fan.last = fan.command_time = sl_now();
    return 0;
}
static const afb_verb_t verbs[] = {
    {.verb="read", .callback=read_fan, .info="Read simulated fan feedback"},
    {.verb="apply", .callback=apply, .info="Internal controller command, duty 0..100"},
    {0}
};
const afb_binding_t afbBindingV4 = {
    .api="sameloop-fan", .info="SameLoop simulated fan", .verbs=verbs,
    .mainctl=ctl, .noconcurrency=1
};
