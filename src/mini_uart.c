#include "utils.h"
#include "peripherals/mini_uart.h"
#include "peripherals/gpio.h"
#include "string.h"
#include "printf.h"
typedef unsigned long uint32_t;
void uart_send( char c )
{
	//core_timer_disable();
	while(1) {
		if(get32(AUX_MU_LSR_REG)&0x20) 
			break;
	}
	put32(AUX_MU_IO_REG,c);
	//core_timer_enable();
}

char uart_recv( void )
{
	//core_timer_disable();
	while(1) {
		if(get32(AUX_MU_LSR_REG)&0x01) 
			break;
	}
	return(get32(AUX_MU_IO_REG)&0xFF);
	//core_timer_enable();

}

void uart_puts_width(unsigned char* str, int width){
	for (int i = 0; i < width ; ++i){
		uart_send(*(str+ i));
	}
}

void uart_send_string(char* str)
{
	for (int i = 0; str[i] != '\0'; i ++) {
		uart_send((char)str[i]);
	}
}

void uart_send_string_int2hex(int value)
{	
	char buf[8 + 1];
	char *p = int2hex(value,buf);
	for (int i = 0; p[i] != '\0'; i ++) {
		uart_send((char)p[i]);
	}
}

void uart_send_string_longlong2hex(long long value)
{	
	char buf[8 + 1];
	char *p = int2hex(value,buf);
	for (int i = 0; p[i] != '\0'; i ++) {
		uart_send((char)p[i]);
	}
}

void uart_print_long(long long l){
	char str[128] = {0};
	ltoxstr(l, str);
	uart_send_string(str);

}

void uart_print_int(int i){
	char str[128] = {0};
	itoxstr(i, str);
	uart_send_string(str);
	str[0] = '\0';
}

void uart_print_uint(unsigned int i){
	char str[128] = {0};
	uitoxstr(i, str);
	uart_send_string(str);
}

void uart_print_uint32_t(uint32_t i){
	char str[128] = {0};
	uitoxstr(i, str);
	uart_send_string(str);
}

void uart_printf(char *fmt, ...){
	__builtin_va_list args;
	__builtin_va_start(args, fmt);

	extern volatile unsigned char _va_start; // defined in linker
	char* s = (char*) &_va_start;
	vsprintf(s, fmt, args);
	
	while(*s){
		if (*s == '\n') uart_send('\r');
		uart_send(*s++);
	} 
}

void uart_init ( void )
{
	unsigned int selector;

	selector = get32(GPFSEL1);
	selector &= ~(7<<12);                   // clean gpio14
	selector |= 2<<12;                      // set alt5 for gpio14
	selector &= ~(7<<15);                   // clean gpio15
	selector |= 2<<15;                      // set alt5 for gpio15
	put32(GPFSEL1,selector);

	put32(GPPUD,0);
	delay(150);
	put32(GPPUDCLK0,(1<<14)|(1<<15));
	delay(150);
	put32(GPPUDCLK0,0);

	put32(AUX_ENABLES,1);                   //Enable mini uart (this also enables access to its registers)
	put32(AUX_MU_CNTL_REG,0);               //Disable auto flow control and disable receiver and transmitter (for now)
	put32(AUX_MU_IER_REG,0);                //Disable receive and transmit interrupts
	put32(AUX_MU_LCR_REG,3);                //Enable 8 bit mode
	put32(AUX_MU_MCR_REG,0);                //Set RTS line to be always high
	put32(AUX_MU_BAUD_REG,270);             //Set baud rate to 115200

	put32(AUX_MU_CNTL_REG,3);               //Finally, enable transmitter and receiver
}
