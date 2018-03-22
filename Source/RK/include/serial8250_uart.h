/*
 * Copyright 2017 Rockchip Electronics Co., Ltd
 *
 */

#ifndef SERIAL8250_UART_H
#define SERIAL8250_UART_H

#define UART_RX			0 /* In:  Receive buffer */
#define UART_TX			0 /* Out: Transmit buffer */

#define UART_LSR		5 /* In:  Line Status Register */
#define UART_LCR		3 /* Out: Line Control Register */
#define UART_LSR_DR		1 /* Receiver data ready */
#define UART_LSR_BI		0x10 /* Break interrupt indicator */
#define UART_LSR_TEMT	0x40 /* Transmitter empty */


#define UART_MCR		4 /* Out: Modem Control Register */

/*
 * DLAB=1
 */
#define UART_DLL		0 /* Out: Divisor Latch Low */
#define UART_DLM		1 /* Out: Divisor Latch High */


#define UART_USR		0x1f /* In: UART Status Register */
#define UART_SRR		0x22 /* Software reset register */
#define UART_USR_RX_FIFO_FULL		0x10 /* Receive FIFO full */
#define UART_USR_RX_FIFO_NOT_EMPTY	0x08 /* Receive FIFO not empty */
#define UART_USR_TX_FIFO_EMPTY		0x04 /* Transmit FIFO empty */
#define UART_USR_TX_FIFO_NOT_FULL	0x02 /* Transmit FIFO not full */
#define UART_USR_BUSY				0x01 /* UART busy indicator */

#define UART_NO_CHAR	0x00ff0000
#define UART_BREAK		0x00ff0100

int uartInit(void);
void uartPuts(const char *pStr);

#endif /* SERIAL8250_UART_H */

