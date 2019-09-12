#define _GNU_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <ucontext.h>
#include <sys/ucontext.h>

#include "init.h"

void sig_func(int sig, siginfo_t *info, void *ctx) {
    ucontext_t *uc = (ucontext_t *) ctx;
    static int atmpt = 0;
    if (0x338b == *(uint16_t *) uc->uc_mcontext.gregs[REG_RIP]) {
        uc->uc_mcontext.gregs[REG_RSI] = 100000 + (++atmpt);
        uc->uc_mcontext.gregs[REG_RIP] += 2;
    } else if (0x738b == *(uint16_t *) uc->uc_mcontext.gregs[REG_RIP]) {
        int i = ((*(uint32_t *) uc->uc_mcontext.gregs[REG_RIP]) & 0x00ff0000) >> 18;
        uc->uc_mcontext.gregs[REG_RSI] = 100000 + 1000 * i + (++atmpt);
        uc->uc_mcontext.gregs[REG_RIP] += 3;
    } else if (0x758b == *(uint16_t *) uc->uc_mcontext.gregs[REG_RIP]) {
        int i = (((*(uint32_t *) uc->uc_mcontext.gregs[REG_RIP]) & 0x00ff0000) >> 18) + 4;
        uc->uc_mcontext.gregs[REG_RSI] = 100000 + 1000 * i + (++atmpt);
        uc->uc_mcontext.gregs[REG_RIP] += 3;
    } else if (0x138b == *(uint16_t *) uc->uc_mcontext.gregs[REG_RIP]) {
        uc->uc_mcontext.gregs[REG_RDX] = 100000 + (++atmpt);
        uc->uc_mcontext.gregs[REG_RIP] += 2;
    } else if (0x558b == *(uint16_t *) uc->uc_mcontext.gregs[REG_RIP]) {
        int i = (((*(uint32_t *) uc->uc_mcontext.gregs[REG_RIP]) & 0x00ff0000) >> 18) + 4;
        uc->uc_mcontext.gregs[REG_RDX] = 100000 + 1000 * i + (++atmpt);
        uc->uc_mcontext.gregs[REG_RIP] += 3;
    } 
}

void init(void *base) {
	struct sigaction sigact = {
        .sa_sigaction = sig_func,
        .sa_flags = SA_RESTART,
    };
    sigemptyset(&sigact.sa_mask);
    sigaction(SIGSEGV, &sigact, NULL);
}

