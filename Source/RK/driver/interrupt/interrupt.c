/*
 * API for the SoC interrupts
 * Copyright (C) 2018 Rockchip, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* Standard includes. */
#include <stdlib.h>
#include <string.h>

#include "gic.h"
#include "hw_memmap.h"
#include "hw_types.h"
#include "interrupt.h"

static struct irq_desc irqs_desc[INTERRUPT_ID_END];

static u8 g_initialized;
static u8 g_gic_cpumask = 0x01;

#define readl(addr) (*(volatile u32 *) (addr))
#define writel(b,addr) ((*(volatile u32 *) (addr)) = (b))
#define gicc_readl(offset)	readl(CORE_GICC_BASE + (offset))
#define gicc_writel(v, offset)	writel(v, CORE_GICC_BASE + (offset))
#define gicd_readl(offset)	readl(CORE_GICD_BASE + (offset))
#define gicd_writel(v, offset)	writel(v, CORE_GICD_BASE + (offset))

#define IRQ_REG_X4(irq)		(4 * ((irq) / 4))
#define IRQ_REG_X16(irq)	(4 * ((irq) / 16))
#define IRQ_REG_X32(irq)	(4 * ((irq) / 32))
#define IRQ_REG_X4_OFFSET(irq)	((irq) % 4)
#define IRQ_REG_X16_OFFSET(irq)	((irq) % 16)
#define IRQ_REG_X32_OFFSET(irq)	((irq) % 32)

#define IRQ_TYPE_NONE		0
#define IRQ_TYPE_EDGE_RISING	1
#define IRQ_TYPE_EDGE_FALLING	2
#define IRQ_TYPE_EDGE_BOTH	(IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING)
#define IRQ_TYPE_LEVEL_HIGH	4
#define IRQ_TYPE_LEVEL_LOW	8

typedef enum INT_TRIG {
	INT_LEVEL_TRIGGER,
	INT_EDGE_TRIGGER
} eINT_TRIG;

struct irq_desc {
	interrupt_handler_t *handle_irq;
	void *data;
};

static void cpu_local_irq_enable(void)
{
	__asm("MRS r0, CPSR");
	__asm("BIC r0, r0, #0x80");
	__asm("MSR CPSR_cxsf, r0");
}

static void cpu_local_irq_disable(void)
{
	__asm("MRS r0, CPSR");
	__asm("ORR r0, r0, #0x80");
	__asm("MSR CPSR_cxsf, r0");
}

static int irq_bad(int irq)
{
	if (irq >= INTERRUPT_ID_END) {
		return -1;
	}

	if (!g_initialized) {
		return -1;
	}

	return 0;
}

static int gic_irq_get(void)
{
	return gicc_readl(GICC_IAR) & 0x3fff;
}

static void int_set_prio_filter(u32 priority)
{
	gicc_writel(priority & 0xff, GICC_PMR);
}

static void int_enable_distributor(void)
{
	u32 val;

	val = gicd_readl(GICD_CTLR);
	val |= 0x01;
	gicd_writel(val, GICD_CTLR);
}

static void int_disable_distributor(void)
{
	u32 val;

	val = gicd_readl(GICD_CTLR);
	val &= ~0x01;
	gicd_writel(val, GICD_CTLR);
}

static void int_enable_secure_signal(void)
{
	u32 val;

	val = gicc_readl(GICC_CTLR);
	val |= 0x01;
	gicc_writel(val, GICC_CTLR);
}

static void int_disable_secure_signal(void)
{
	u32 val;

	val = gicc_readl(GICC_CTLR);
	val &= ~0x01;
	gicc_writel(val, GICC_CTLR);
}

static void int_enable_nosecure_signal(void)
{
	u32 val;

	val = gicc_readl(GICC_CTLR);
	val |= 0x02;
	gicc_writel(val, GICC_CTLR);
}

static void int_disable_nosecure_signal(void)
{
	u32 val;

	val = gicc_readl(GICC_CTLR);
	val &= ~0x02;
	gicc_writel(val, GICC_CTLR);
}

static u32 gic_get_cpumask(void)
{
	u32 mask = 0, i;

	for (i = mask = 0; i < 32; i += 4) {
		mask = gicd_readl(GICD_ITARGETSRn + 4 * i);
		mask |= mask >> 16;
		mask |= mask >> 8;
		if (mask)
			break;
	}

	return mask;
}

static int gic_irq_init(void)
{
	u32 val;

	/* end of interrupt */
	gicc_writel(INTERRUPT_ID_END - 1, GICC_EOIR);

	/* disable gicc and gicd */
	gicc_writel(0, GICC_CTLR);
	gicd_writel(0, GICD_CTLR);

	/* disable interrupt */
	gicd_writel(0xffffffff, GICD_ICENABLERn + 0);
	gicd_writel(0xffffffff, GICD_ICENABLERn + 4);
	gicd_writel(0xffffffff, GICD_ICENABLERn + 8);
	gicd_writel(0xffffffff, GICD_ICENABLERn + 12);

	val = gicd_readl(GICD_ICFGR + 12);
	val &= ~(1 << 1);
	gicd_writel(val, GICD_ICFGR + 12);

	/* set interrupt priority threhold min: 256 */
	int_set_prio_filter(0xff);
	int_enable_secure_signal();
	int_enable_nosecure_signal();
	int_enable_distributor();

	g_gic_cpumask = gic_get_cpumask();

	return 0;
}

static void gic_irq_eoi(int irq)
{
	gicc_writel(irq, GICC_EOIR);
}

static int irq_handler_disable(int irq)
{
	if (irq_bad(irq))
		return -1;

	return gic_irq_disable(irq);
}

static int gic_irq_enable(int irq)
{
	u32 val;
	u32 shift = (irq % 4) * 8;

	if (irq >= INTERRUPT_ID_END)
		return -1;

	/* set enable */
	val = gicd_readl(GICD_ISENABLERn + IRQ_REG_X32(irq));
	val |= 1 << IRQ_REG_X32_OFFSET(irq);
	gicd_writel(val, GICD_ISENABLERn + IRQ_REG_X32(irq));

	/* set target */
	val = gicd_readl(GICD_ITARGETSRn + IRQ_REG_X4(irq));
	val &= ~(0xFF << shift);
	val |= (g_gic_cpumask << shift);
	gicd_writel(val, GICD_ITARGETSRn + IRQ_REG_X4(irq));

	return 0;
}

static int gic_irq_disable(int irq)
{
	gicd_writel(1 << IRQ_REG_X32_OFFSET(irq),
		    GICD_ICENABLERn + IRQ_REG_X32(irq));

	return 0;
}

static int gic_irq_set_trigger(int irq, eINT_TRIG trig)
{
	u32 val;

	if (trig == INT_LEVEL_TRIGGER) {
		val = gicd_readl(GICD_ICFGR + IRQ_REG_X16(irq));
		val &= ~(1 << (2 * IRQ_REG_X16_OFFSET(irq) + 1));
		gicd_writel(val, GICD_ICFGR + IRQ_REG_X16(irq));
	} else {
		val = gicd_readl(GICD_ICFGR + IRQ_REG_X16(irq));
		val |= (1 << (2 * IRQ_REG_X16_OFFSET(irq) + 1));
		gicd_writel(val, GICD_ICFGR + IRQ_REG_X16(irq));
	}

	return 0;
}

/*
 * irq_set_type - set the irq trigger type for an irq
 *
 * @irq: irq number
 * @type: IRQ_TYPE_{LEVEL,EDGE}_* value - see asm/arch/irq.h
 */
static int gic_irq_set_type(int irq, unsigned int type)
{
	unsigned int int_type;

	switch (type) {
	case IRQ_TYPE_EDGE_RISING:
	case IRQ_TYPE_EDGE_FALLING:
		int_type = 0x1;
		break;
	case IRQ_TYPE_LEVEL_HIGH:
	case IRQ_TYPE_LEVEL_LOW:
		int_type = 0x0;
		break;
	default:
		return -1;
	}

	gic_irq_set_trigger(irq, int_type);

	return 0;
}

void IrqRegister(int irq, interrupt_handler_t *handler, void *data)
{
	if (irq_bad(irq))
		return;

	irqs_desc[irq].handle_irq = handler;
	irqs_desc[irq].data = data;
}

void IrqUnregister(int irq)
{
	if (irq_handler_disable(irq))
		return;

	irqs_desc[irq].handle_irq = NULL;
	irqs_desc[irq].data = NULL;
}

int IrqEnable(int irq)
{
	if (irq_bad(irq))
		return -1;

	return gic_irq_enable(irq);
}

int IrqDisable(int irq)
{
	if (irq_bad(irq))
		return -1;

	gic_irq_disable(irq);
}

int IrqSetType(int irq, unsigned int type)
{
	if (irq_bad(irq))
		return -1;

	return gic_irq_set_type(irq, type);
}

void EnableInterrupts(void)
{
    cpu_local_irq_enable();
}

void DisableInterrupts(void)
{
    cpu_local_irq_disable();
}

void IrqHandler(void)
{
	u32 irq = gic_irq_get();

	if (irq < INTERRUPT_ID_END) {
		if (irqs_desc[irq].handle_irq)
			irqs_desc[irq].handle_irq(irq, irqs_desc[irq].data);
	}

	gic_irq_eoi(irq);
}

int InitInterrupt(void)
{
	int irq, err = -1;

	g_initialized = true;

	for (irq = 0; irq < INTERRUPT_ID_END; irq++) {
		irqs_desc[irq].handle_irq = NULL;
		irqs_desc[irq].data = NULL;
	}

	err = gic_irq_init();
	if (err) {
		goto out;
	}

	return 0;

out:
	g_initialized = false;

	return err;
}

