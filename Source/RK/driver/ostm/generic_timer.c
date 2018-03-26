/*
 * Copyright 2018 Rockchip Electronics Co., Ltd
 *
 */

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"

#include "generic_timer.h"
#include "interrupt.h"
#include "serial.h"

static unsigned int timer_reload_val;

static unsigned int read_cntfrq(void)
{
	unsigned int cntfrq;

	__asm volatile {
		mrc p15, 0, cntfrq, c14, c0, 0
	}

	return cntfrq;
}

static void write_cntp_tval(unsigned int cntp_tval)
{
	__asm volatile {
		mcr p15, 0, cntp_tval, c14, c2, 0
		isb
	}
}

static void write_cntp_ctl(unsigned int cntp_ctl)
{
	__asm volatile {
		mcr p15, 0, cntp_ctl, c14, c2, 1
		isb
	}
}

static void platformSetOneshotTimer(int interval)
{
	write_cntp_tval(interval);
	write_cntp_ctl(1); /* enable timer */
}

static void platformTick(int irq, void *data)
{
	write_cntp_ctl(0); /* disable timer */

	FreeRTOS_Tick_Handler();

	platformSetOneshotTimer(timer_reload_val);
}

void armGenericTimerInit(int irq, unsigned int freq_override)
{
	unsigned int cntfrq;
	int ret;

	if (freq_override == 0) {
		cntfrq = read_cntfrq(); /* default: 24MHz */
		if (!cntfrq) {
			debug("Timer frequency is zero.\n");
			return;
		}
	} else {
		cntfrq = freq_override;
	}

	timer_reload_val = (cntfrq / configTICK_RATE_HZ) * 10; /* 10ms */

	IrqRegister(ARCH_GENERIC_TIMER_PPI_IRQ, &platformTick, NULL);

	platformSetOneshotTimer(timer_reload_val);

	ret = IrqEnable(ARCH_GENERIC_TIMER_PPI_IRQ);
	if (ret)
		debug("arch-timer irq enable failed.\n");
}
