#include "mini_uart.h"
#include "StringUtils.h"
#include "reboot.h"
#include "initrd.h"
#include "dtb.h"
#include "timer.h"
#include "page.h"
#include "allocator.h"
#include "task.h"
#include "printf.h"
#include "mailbox.h"
#include <stddef.h>

#define BUFFER_MAX_SIZE 128u

// extern void *_dtb_ptr;

static void printPrompt(void) {
    uart_send_string("\r");
    uart_send_string("> ");
}

void readCommand(char *buffer) {
    size_t size = 0u;
    while (size < BUFFER_MAX_SIZE) {
        buffer[size] = uart_recv();
        
        // echo back
        uart_send(buffer[size]);

        if (buffer[size++] == '\n') {
            break;
        }
    }
    
    buffer[size] = '\0';
    Enter2Null(buffer);
}

static void help(void) {
    uart_send_string("\rshell menu :\r\n");
    uart_send_string("help : print this help menu\r\n");
    uart_send_string("hello : print Hello World\r\n");
    uart_send_string("reboot : power off then on\r\n");
    uart_send_string("Info : use mailbox\r\n");
    uart_send_string("ls   :   list file\r\n");
    uart_send_string("cat   :   print file context\r\n");
    uart_send_string("load_prog   :   execute user program\r\n");
    uart_send_string("async_uart  :   test async uart \r\n");
    uart_send_string("test_timer  :   test timer \r\n");
    uart_send_string("test_page  :    use page reserve to test \r\n");
    uart_send_string("test_dyn  :    dynamic allocator \r\n");
    uart_send_string("test_fork  :    lab5 \r\n");
    uart_send_string("video_player  :    lab5 \r\n");
}

static void hello(void) {
    uart_send_string("Hello World!\r\n");
}

static void reboot(void) {
    reset(100);
    while(1){
        uart_send_string("...\r\n");
    }
}

static void Info(void) {
    get_board_revision();
    get_ARM_memory();
}

static void loadimg(){

	unsigned long k_addr=0,k_size=0;
	uart_send_string("Please enter kernel load address (Hex): ");

    char buffer[BUFFER_MAX_SIZE];

    readCommand(buffer);
    k_addr = getHexFromString(buffer);
    uart_send_string("\n");
	uart_send_string("Please enter kernel size (Dec): ");
    readCommand(buffer);
    k_size = getIntegerFromString(buffer);
    uart_send_string("\n");

	uart_send_string("Please send kernel image now...\n");
		unsigned char* target=(unsigned char*)k_addr;
		while(k_size--){
			*target=uart_getb();
			target++;
			uart_send('.');
		}

		uart_send_string("loading...\n");
		asm volatile("br %0\n"::"r"(k_addr)); // (assembler template:output operand:input operand: clobber??) %0 is number of operand "r" means register
}

static void parseCommand(char *buffer) {
    // remove newline
    stripString(buffer);

    if (*buffer == '\0') {
        uart_send_string("\r");
        return;
    }

    if (compareString("help", buffer) == 0) {
        help();
    } else if (compareString("hello", buffer) == 0) {
        hello();
    } else if (compareString("log_mode", buffer) == 0) {
        log_mode = 1;
    }else if (compareString("reboot", buffer) == 0) {
        reboot();
    } else if (compareString("Info", buffer) == 0) {
        Info();
    } else if (compareString("loadimg", buffer) == 0) {
        loadimg();
    }else if(compareString("ls",buffer) == 0){
        initrd_ls();
    }else if(compareString("cat",buffer) == 0){
        char f_buffer[BUFFER_MAX_SIZE];
        uart_send_string("\r");
        uart_send_string("Filename : ");
        readCommand(f_buffer);
        initrd_cat(f_buffer);
    }else if(compareString("dtb",buffer) == 0){
        // traverse_device_tree( _dtb_ptr , print_dtb);
    }else if (compareString(buffer, "load_prog") == 0){
        load_program("user_prog.img");
    }else if (compareString(buffer, "async_uart") == 0){
        test_uart_async();
    }else if (compareString(buffer, "test_timer") == 0){
        test_timer();
    }else if (compareString(buffer, "clear_timer") == 0){
        clear_timer();
    }else if (compareString(buffer, "test_page") == 0){
        test_page();
    } else if (compareString(buffer, "test_dyn") == 0){
        test_dyn();
    }else if (compareString(buffer, "test_fork") == 0) {
        run_user_program("fork_test.img", NULL);
    }else if (compareString(buffer, "mailbox_test") == 0) {
        run_user_program("mailbox_test.img", NULL);
    }else if (compareString(buffer, "video_player") == 0) {
        run_user_program("syscall.img", NULL);
    } else {
        uart_send_string("\rcommand not found: ");
        uart_send_string(buffer);
        uart_send_string("\r\n");
    }
}

void shell(void) {
    char buffer[BUFFER_MAX_SIZE];
    while (1) {
        uart_send_string("\r\n");
        printPrompt();
        readCommand(buffer);
        stripString(buffer);
        parseCommand(buffer);
    }
}
