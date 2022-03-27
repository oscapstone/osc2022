#ifndef	IRQ_H_
#define	IRQ_H_

#include <gpio.h>

#define IRQ_BASIC_PENDING	((volatile unsigned int*)(MMIO_BASE+0x0000B200))
#define IRQ_PENDING_1		((volatile unsigned int*)(MMIO_BASE+0x0000B204))
#define IRQ_PENDING_2		((volatile unsigned int*)(MMIO_BASE+0x0000B208))
#define FIQ_CONTROL		    ((volatile unsigned int*)(MMIO_BASE+0x0000B20C))
#define ENABLE_IRQS_1		((volatile unsigned int*)(MMIO_BASE+0x0000B210))
#define ENABLE_IRQS_2		((volatile unsigned int*)(MMIO_BASE+0x0000B214))
#define ENABLE_BASIC_IRQS	((volatile unsigned int*)(MMIO_BASE+0x0000B218))
#define DISABLE_IRQS_1		((volatile unsigned int*)(MMIO_BASE+0x0000B21C))
#define DISABLE_IRQS_2		((volatile unsigned int*)(MMIO_BASE+0x0000B220))
#define DISABLE_BASIC_IRQS	((volatile unsigned int*)(MMIO_BASE+0x0000B224))
#define CORE0_TIMER_IRQ_CTRL	(volatile unsigned int*)0x40000040
#define CORE0_IRQ_SOURCE	    (volatile unsigned int*)0x40000060

#define SYSTEM_TIMER_IRQ_0	(1 << 0)
#define SYSTEM_TIMER_IRQ_1	(1 << 1)
#define SYSTEM_TIMER_IRQ_2	(1 << 2)
#define SYSTEM_TIMER_IRQ_3	(1 << 3)
#define NON_SECURE_TIMER_IRQ	(1 << 1)

#endif  