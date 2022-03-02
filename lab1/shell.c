#include "shell.h"

#include "command.h"
#include "uart.h"

int parse(char input_char, int buffer_counter) {
    if ((input_char > 31 && input_char < 127) || input_char == 9) {
        buffer_counter++;
        uart_send(input_char);
    }
    else if (buffer_counter != 0 && (input_char == 8 || input_char == 127)) {
        // backspace
        buffer_counter--;
        uart_send(8);
        uart_send(' ');
        uart_send(8);
    }
    else if (input_char == 10 || input_char == 13) {
        // Enter
        uart_send(10);
    }

    return buffer_counter;
}

void shell() {
    int buffer_counter = 0;
    char input_char;
    char buffer[MAX_BUFFER_LEN] = {0};

    parse_command("mbox_board_revision");
    parse_command("mbox_arm_memory");
    // new line head
    uart_puts("# ");

    // read input
    while (1) {
        input_char = uart_getc();

        buffer[buffer_counter] = input_char;
        buffer_counter = parse(input_char, buffer_counter);

        if (input_char == '\n') {
            // Enter
            buffer[buffer_counter] = 0;
            buffer_counter = 0;
            // continue;
            parse_command(buffer);
            uart_puts("# ");
        }
    }
}
