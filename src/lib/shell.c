#include "shell.h"

extern void from_EL1_to_EL0(unsigned long prog, unsigned long sp);


void exec (cpio_new_header *header, char *file_name, int enable_timer)
{
    void (*prog)();
    unsigned int current_el;
    char *stack_top;
    
    stack_top = malloc(0x2000);
    stack_top = stack_top + 0x2000;

    prog = cpio_load(header, file_name);

    if (prog == 0)
    {
        uart_puts("User program not found!\n");
        return;
    }

    // Get current EL
    asm volatile ("mrs %0, CurrentEL" : "=r" (current_el));
    current_el = current_el >> 2;

    // get current exception level
    uart_puts("Current EL: 0x");
    uart_hex(current_el);
  
    uart_puts("\n");
    uart_puts("-----------------Entering user program-----------------\n");

    if (enable_timer) 
    {
        core_timer_enable();
    }

    from_EL1_to_EL0((unsigned long)prog, (unsigned long)stack_top);


    return;
}

void print_sys_info() {
    unsigned int revision;
    unsigned int mem_base;
    unsigned int mem_size;

    get_board_revision(& revision);
    uart_puts("\rboard revision: 0x");
    uart_hex(revision);

    get_arm_memory(& mem_base, &mem_size);
    uart_puts("\nmemory base: ");
    uart_hex(mem_base);
    uart_puts("\nmemory size: ");
    uart_hex(mem_size);
    uart_puts("\n");
}

void welcome_msg() {
    char *welcome = "\rwelcome to my shell, type 'help' for more details!\n";
    uart_puts(welcome);
}

void helper() {
    uart_puts("-----------------------------------------------------\r\n");
    uart_puts("* mm        : test buddy system.\n");
    uart_puts("* aput      : test asynchronous puts.\n");
    uart_puts("* asend     : test asynchronous send.\n");
    uart_puts("* exec      : load and execute user program.\n");
    uart_puts("* core_time : load and execute user program with core timer enable.\n");
    uart_puts("* help      : print this help menu\n");
    uart_puts("* hello     : print hello world!\n");
    uart_puts("* reboot    : reboot the device\n");
    uart_puts("* hwinfo    : print the hardware information\n");
    uart_puts("* ls        : print files in cpio archieve\n");
    uart_puts("* cat       : print content in cpio archieve\n");
    uart_puts("-----------------------------------------------------\r\n");
}

// handle all commands
void cmd_handler(char *cmd) {
    uart_puts("\r");
    if (strcmp(cmd, "help") == 0) {
        helper();
    }
    else if (strcmp(cmd, "hello") == 0) {
        uart_puts("Hello World!\n");
    }
    else if (strcmp(cmd, "reboot") == 0) {
        uart_puts("rebooting................\n");
        reboot(100);
    }
    else if (strcmp(cmd, "\0") == 0) {
        uart_puts("\r");
    }
    else if (strcmp(cmd, "hwinfo") == 0) {
        print_sys_info();
    }
    else if (strcmp(cmd, "ls") == 0) {
        cpio_ls((cpio_new_header *)CPIO_BASE);
    }
    else if (strcmp(cmd, "cat") == 0) {
        uart_puts("Filename: ");
        char input[MAX_BUFFER_SIZE];
        cmd_reader(input);
        uart_puts("\r\n");
        cpio_cat((cpio_new_header *)CPIO_BASE, input);
    }
    else if (strcmp(cmd, "exec") == 0) {
        uart_puts("Name: ");
        char input[MAX_BUFFER_SIZE];
        cmd_reader(input);
        uart_puts("\r\n");
        exec((cpio_new_header *)CPIO_BASE, input, 0);
    }
    else if (strcmp(cmd, "core_time") == 0) {
        uart_puts("Name: ");
        char input[MAX_BUFFER_SIZE];
        cmd_reader(input);
        uart_puts("\r\n");
        exec((cpio_new_header *)CPIO_BASE, input, 1);
    }
    else if (strcmp(cmd, "aput") == 0) {

        uart_puts("test\r\n");

        asyn_uart_puts("test asyn put\r\n");

    }
    else if (strcmp(cmd, "asend") == 0) {
        char c = async_uart_getc();
        uart_send(c);

        uart_puts("\n");

    }
    else if (strcmp(cmd, "mm") == 0) {
        mm();
    }
    else 
    {
        uart_puts("invalid command!");
    }
    uart_puts("\n");
    return;
}

// read command
void cmd_reader(char *cmd) {
    unsigned int idx = 0;
    char c;

    while (1) {
        c = uart_getc();

        if (c == '\r') continue;
        uart_send(c);
        cmd[idx++] = c;
        idx = idx < MAX_BUFFER_SIZE ? idx : MAX_BUFFER_SIZE - 1;
        if (c == '\n') {
            cmd[idx - 1] = '\0';
            break;
        }
    }
}

// do shell
void exe_shell() {
    print_sys_info();
    welcome_msg();

    char cmd[MAX_BUFFER_SIZE];

    while (1) {
        uart_puts("->");
        cmd_reader(cmd);
        cmd_handler(cmd);
    }
}
