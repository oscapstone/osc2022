#include "uart.h"
#include "mbox.h"
#include "shell.h"
#include "string.h"

extern char* _code_relocate_place;
extern unsigned long long  __code_size;
extern unsigned long long _start;

void code_relocate(char * addr);

int relocate=1;

void main()
{
    char* reloc_place = (char*)&_code_relocate_place;

    if(relocate) // only do relocate once
    {
        relocate = 0;
        code_relocate(reloc_place);
    }

    // set up serial console
    uart_init();

    shell();
}

// relocate code and jump to there
void code_relocate(char * addr)
{
    unsigned long long size = (unsigned long long)&__code_size;
    char* start = (char *)&_start;
    for(unsigned long long i=0;i<size;i++)
    {
        addr[i] = start[i];
    }

    ((void (*)(void))addr)();  //jump to new place
}
