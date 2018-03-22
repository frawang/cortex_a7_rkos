/*
 * Copyright 2017 Rockchip Electronics Co., Ltd
 *
 */

#include "serial8250_uart.h"

struct S8250_Uart {
	int irq;
	int baudrate;
	unsigned int *debug_port_base;
	unsigned char break_seen;
};

static struct S8250_Uart g_s8250_uart;

#define UART2_BASE		0x10210000

#define readl(c)        (*(volatile unsigned int *) (c))
#define writel(v, c)    ((*(volatile unsigned int *) (c)) = (v))

__asm void ASMDelay(unsigned int i)
{
ASMDelayxx
	SUBS    R0, R0, #1
	BHI     ASMDelayxx
	BX      LR
}

#define udelay(i)	ASMDelay((i))

static void __uartWrite(struct S8250_Uart *t,
	unsigned int val, unsigned int off)
{
	writel(val, t->debug_port_base + off * 4);
}

static unsigned int __uartRead(struct S8250_Uart *t,
	unsigned int off)
{
	return readl(t->debug_port_base + off * 4);
}

int uartInit(void)
{
	int dll = 0, dlm = 0;
	struct S8250_Uart *pUart = &g_s8250_uart;

	pUart->debug_port_base = (unsigned int *) UART2_BASE;
	pUart->baudrate = 1500000;

	if (__uartRead(pUart, UART_LSR) & UART_LSR_DR)
		(void)__uartRead(pUart, UART_RX);

	switch (pUart->baudrate) {
	case 1500000:
		dll = 0x1;
		break;
	case 115200:
	default:
		dll = 0xd;
		break;
	}

	/* reset uart */
	__uartWrite(pUart, 0x07, UART_SRR);
	udelay(10);

	/* set uart to loop back mode */
	__uartWrite(pUart, 0x10, UART_MCR);

	__uartWrite(pUart, 0x83, UART_LCR);

	/* set baud rate */
	__uartWrite(pUart, dll, UART_DLL);
	__uartWrite(pUart, dlm, UART_DLM);
	__uartWrite(pUart, 0x03, UART_LCR);

	/* disbale loop back mode */
	__uartWrite(pUart, 0x0, UART_MCR);

	return 0;
}

static unsigned int uartReadLsr(struct S8250_Uart *t)
{
	unsigned int lsr;

	lsr = __uartRead(t, UART_LSR);
	if (lsr & UART_LSR_BI)
		t->break_seen = 1;

	return lsr;
}

static int uartGetc()
{
	struct S8250_Uart *pUart = &g_s8250_uart;
	unsigned int lsr;
	unsigned int temp;
	static unsigned int n;
	static char buf[32];

	/* Clear uart status */
	__uartRead(pUart, UART_USR);
	lsr = uartReadLsr(pUart);

	if (lsr & UART_LSR_DR) {
		temp = __uartRead(pUart, UART_RX);
		buf[n & 0x1f] = temp;
		n++;
		if (temp == 'q' && n > 2) {
			if ((buf[(n - 2) & 0x1f] == 'i') &&
			    (buf[(n - 3) & 0x1f] == 'f'))
				return UART_BREAK;
			else
				return temp;
		} else {
			return temp;
		}
	}

	return UART_NO_CHAR;
}

static void uartPutc(unsigned int c)
{
	struct S8250_Uart *pUart = &g_s8250_uart;
	unsigned int count = 1000;

	while (!(__uartRead(pUart, UART_USR) & UART_USR_TX_FIFO_NOT_FULL) && count--)
		udelay(1);

	/* If uart is always busy, maybe it is abnormal, reinit it */
	if ((count == 0) && (__uartRead(pUart, UART_USR) & UART_USR_BUSY))
		uartInit();

	__uartWrite(pUart, c, UART_TX);
}

static void uartFlush()
{
	struct S8250_Uart *pUart = &g_s8250_uart;
	unsigned int count = 10000;

	while (!(uartReadLsr(pUart) & UART_LSR_TEMT) && count--)
		udelay(10);

	/* If uart is always busy, maybe it is abnormal, reinit it */
	if ((count == 0) && (__uartRead(pUart, UART_USR) & UART_USR_BUSY))
		uartInit();
}

void uartPuts(const char *pStr)
{
    while (*pStr != 0) {
	uartPutc(*pStr);
        pStr++;
    }

    uartPutc('\r');
}
