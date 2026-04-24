#pragma once

struct pm_qos_request {
    int dummy;
};

static inline void cpu_latency_qos_add_request(struct pm_qos_request *req,
                                               int value) {
    (void)req;
    (void)value;
}

static inline void cpu_latency_qos_update_request(struct pm_qos_request *req,
                                                  int value) {
    (void)req;
    (void)value;
}

static inline void cpu_latency_qos_remove_request(struct pm_qos_request *req) {
    (void)req;
}
