/*
 * Copyright 2018 Rockchip Electronics Co., Ltd
 *
 */

#include "serial.h"
#include <stdarg.h>

#include "vsprintf.c"

extern void uartInit(void);
extern void uartFlush(void );
extern void uartPutc(char c);

void serial_init (void)
{
	uartInit();
}

void serial_printf(const char* Format, ...)
{
	unsigned int i;
	unsigned int len;
	char printBuf[2048];
	va_list  Args;

	va_start(Args, Format);
	len = vsprintf(printBuf , (const char*)Format , Args);
	va_end(Args);

	for(i = 0; i < len; i++) {
		uartPutc(printBuf[i]);

		if (printBuf[i] == '\n')
			uartPutc('\r');
	}
}
