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

int __base = 0;
int __base_offset = 100000;
int __base_index_offset = 1000;
int __base__step = 2;
int __base__step_with_offset = 3;

void sig_func(int sig, siginfo_t *info, void *ctx) {
    ucontext_t *uc = (ucontext_t *) ctx;
    static int atmpt = 0;
    switch (*(uint16_t *) uc->uc_mcontext.gregs[REG_RIP])
    {
        case 0x138b:
        {
            uc->uc_mcontext.gregs[REG_RDX] = __base_offset + (++atmpt);
            uc->uc_mcontext.gregs[REG_RIP] += __base__step;
        }
            break;
            
        case 0x538b:
        {
            int i = ((*(uint32_t *) uc->uc_mcontext.gregs[REG_RIP]) & 0x00ff0000) >> 18;
            uc->uc_mcontext.gregs[REG_RDX] = __base_offset + __base_index_offset * i + (++atmpt);
            uc->uc_mcontext.gregs[REG_RIP] += __base__step_with_offset;
        }
            break;
        
        case 0x558b:
        {
            int i = (((*(uint32_t *) uc->uc_mcontext.gregs[REG_RIP]) & 0x00ff0000) >> 18) + \
			(((int)(uc->uc_mcontext.gregs[REG_RBP]) - __base) >> 2);
            uc->uc_mcontext.gregs[REG_RDX] = __base_offset + __base_index_offset * i + (++atmpt);
            uc->uc_mcontext.gregs[REG_RIP] += __base__step_with_offset;
        }
            break;

        case 0x0b8b:
        {
            uc->uc_mcontext.gregs[REG_RCX] = __base_offset + (++atmpt);
            uc->uc_mcontext.gregs[REG_RIP] += __base__step;
        }
            break;
        
        case 0x4d8b:
        {
            int i = (((*(uint32_t *) uc->uc_mcontext.gregs[REG_RIP]) & 0x00ff0000) >> 18) + \
			(((int)(uc->uc_mcontext.gregs[REG_RBP]) - __base) >> 2);
            uc->uc_mcontext.gregs[REG_RCX] = __base_offset + __base_index_offset * i + (++atmpt);
            uc->uc_mcontext.gregs[REG_RIP] += __base__step_with_offset;
        }
            break;
            
        default:
            break;
    } 
}

void init(void *base) {
    __base = base;
	struct sigaction sigact = {
        .sa_sigaction = sig_func,
        .sa_flags = SA_RESTART,
    };
    sigemptyset(&sigact.sa_mask);
    sigaction(SIGSEGV, &sigact, NULL);
}

