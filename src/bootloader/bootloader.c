#include "../uart.h"

int boot()
{
    char *kernel_addr = (char *)0x80000;
    uart_init();
    uart_puts("Waiting for kernel image...\n");
    
    int kernel_size = 0;
    for (int i = 0; i < 4; i++) {
        kernel_size <<= 8;
        kernel_size |= uart_getc();
    }
    uart_puts("The kernel Size is : \n");
    uart_putx(kernel_size);
    uart_putc('\n');

    // for (int i = 0; i < kernel_size; i++) {
    //     kernel_addr[i] = uart_getc();   
    // }
    while (kernel_size--) {
        // char b = uart_getc();
        // *kernel_addr++ = b;
        *kernel_addr++ = uart_getc();
        // uart_putc(b);
    }
    uart_puts("Read kernel done !! \n");
    asm volatile (
        // we must force an absolute address to branch to
        "mov x30, 0x80000; ret" // X30 is used as the Link Register and can be referred to as LR.
        // "ret x30" // Return from subroutine, branches unconditionally to an address in a register, with a hint that this is a subroutine return.
    );
    
    return 0;
}