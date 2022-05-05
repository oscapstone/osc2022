#ifndef TPF_H
#define TPF_H

typedef struct trapframe {
    unsigned long x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10;
    unsigned long x11, x12, x13, x14, x15, x16, x17, x18, x19, x20;
    unsigned long x21, x22, x23, x24, x25, x26, x27, x28, x29, x30;
    unsigned long spsr_el1, elr_el1, sp_el0;
} trapframe_t;

#endif