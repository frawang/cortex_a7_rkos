/*********************************************************************
(C) Copyright 2005-2016 Fuzhou Rockchip Electronics Co.Ltd
This program is the unpublished property and trade secret of Rockchip.
It is to be utilized solely under license from Rockchip and it is   to
be maintained on a confidential basis for internal company use only.
The security and protection of the program is paramount to maintenance
of the trade secret status.  It is to  be  protected  from  disclosure
to unauthorized parties, both within the Licensee company and outside,
in a manner  not less stringent than  that utilized for Licensee's own
proprietary internal information.  No  copies of  the source or Object
Code are to leave the premises of Licensee's business except in strict
accordance with the license agreement signed by Licensee with Rockchip.
*********************************************************************/

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
