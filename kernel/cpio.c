#include "cpio.h"

cpio_newc_header* header;

char *syscall_file_start_addr;
unsigned int syscall_file_size;

void cpio_init() {
    header = 0;
}

void cpio_newc_parser(cpio_callback_t callback, char* param) {
    char *cpio_ptr = INITRD_ADDR;
    char *file_name, *file_data;
    unsigned int name_size, data_size;
    cpio_newc_header* header = simple_alloc(CPIO_NEWC_HEADER_SIZE);

    while (1) {
        // cpio_ptr move 0x6e
        cpio_newc_parse_header(&cpio_ptr, &header);
        // name size contain null byte, ex : ./k1a => 6
        name_size = hex_to_uint(header->c_namesize, sizeof(header->c_namesize));
        // data size without null byte, ex : k44a\nk1a => 8
        data_size = hex_to_uint(header->c_filesize, sizeof(header->c_filesize));
        // path align with header
        cpio_newc_parse_data(&cpio_ptr, &file_name, name_size, CPIO_NEWC_HEADER_SIZE);
        // data align itself
        cpio_newc_parse_data(&cpio_ptr, &file_data, data_size, 0);

        // cpio archive must have last path_name = TRAILER!!!
        if (!strcmp(file_name, "TRAILER!!!"))
            break;

        // callback function
        callback(param, header, file_name, name_size, file_data, data_size);
    }
}

void cpio_newc_parse_header(char** cpio_ptr, cpio_newc_header** header) {
    *header = (cpio_newc_header *)*cpio_ptr;
    *cpio_ptr += sizeof(cpio_newc_header);
}

void cpio_newc_parse_data(char** cpio_ptr, char** buf, unsigned int size, unsigned int offset) {
    *buf = *cpio_ptr;
    while ((size + offset) % 4 != 0)
        size += 1;
    *cpio_ptr += size;
}

void cpio_ls_callback(char* param, cpio_newc_header* header, char* file_name, unsigned int name_size, char* file_data, unsigned int data_size) {
    uart_printf_async("%s\r\n", file_name);
}

void cpio_cat_callback(char* param, cpio_newc_header* header, char* file_name, unsigned int name_size, char* file_data, unsigned int data_size) {
    if (strcmp(param, file_name))
        return;
    uart_printf_async("Filename: %s\r\n", file_name);
    while (data_size--) {
        // if there is newline, use CRLF
        if (*file_data == '\n')
            uart_write_string_async("\r\n");
        else
            uart_write_char_async(*file_data);
        file_data++;
    }
    uart_printf_async("\r\n");
}

void cpio_prog_callback(char* param, cpio_newc_header* header, char* file_name, unsigned int name_size, char* file_data, unsigned int data_size) {
    if (strcmp(file_name, "./prog.img"))
        return;
    char *prog_stack = simple_alloc(0x10000);
    asm (
        // enable interrupt in EL0, by setting spsr_el1 to 0 before returning to EL0.
        "msr spsr_el1, xzr\n\t"
        "msr elr_el1, %0\n\t"
        "msr sp_el0, %1\n\t"
        "eret\n\t"
        :: "r" (file_data), "r" (prog_stack + 0x10000)
    );
}

void cpio_get_file_start_callback(char* param, cpio_newc_header* header, char* file_name, unsigned int name_size, char* file_data, unsigned int data_size) {
    if (strcmp(file_name, param))
        return;
    syscall_file_start_addr = file_data;
}

void cpio_get_file_size_callback(char* param, cpio_newc_header* header, char* file_name, unsigned int name_size, char* file_data, unsigned int data_size) {
    if (strcmp(file_name, param))
        return;
    syscall_file_size = data_size;
}

char* get_file_start(char* path) {
    cpio_newc_parser(cpio_get_file_start_callback, path);
    return syscall_file_start_addr;
}

unsigned int get_file_size(char* path) {
    cpio_newc_parser(cpio_get_file_size_callback, path);
    return syscall_file_size;
}

void cpio_exec_callback(char* param, cpio_newc_header* header, char* file_name, unsigned int name_size, char* file_data, unsigned int data_size) {
    if (strcmp(file_name, param))
        return;
    exec_thread(file_data, data_size);
}
