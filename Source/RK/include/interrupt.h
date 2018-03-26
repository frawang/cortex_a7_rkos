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

#ifndef __RK_INTERRUPT_H__
#define __RK_INTERRUPT_H__

/* IRQ ID SPI */
typedef enum
{
    ARCH_GENERIC_TIMER_PPI_IRQ = 29,
    BUS_DMAC_IRQ = 32,
    CEVAXM4_EPP_WDOG_VIOL_IRQ,
    DDRUPCTL_INTR,
    DDRMON_INT,
    SARADC_INT,
    CEVAXM4_SEQ_PI_OT_IRQ,
    HDMIPHY_IRQ,
    RKVDEC_M_DEC_IRQ,
    RKVENC_INT,
    VPU_XINTDEC_IRQ,

    VPU_XINTENC_IRQ,
    SDMMC_INT,
    SDIO_INT,
    EMMC_INT,
    NANDC_INT,
    HOST0_EHCI_INT,
    HOST0_OHCI_INT,
    HOST0_ARB_INT,
    OTG_INT,
    MAC_INT,

    MAC_PMT_INT,
    HDMI_IRQ,
    HDMI_WAKEUP_IRQ,
    I2S0_8CH_INTR,
    I2S1_2CH_INTR,
    I2S2_2CH_INTR,
    CRYPTO_INT,
    IEP_IRQ,
    VOP_IRQ,
    RGA_IRQ,

    RKI2C0_INT,
    RKI2C1_INT,
    RKI2C2_INT,
    RKI2C3_INT,
    WDT_INTR,
    TIMER_INTR0,
    TIMER_INTR1,
    SPI0_INTR,
    RKPWM1_INT,
    PWM_PMU_INT,

    GPIO0_INTR,
    GPIO1_INTR,
    GPIO2_INTR,
    GPIO3_INTR,
    UART0_INTR,
    UART1_INTR,
    UART2_INTR,
    TSADC_INT,
    OTG0_BVALID_IRQ,
    CEVAXM4_MCCI_MES_INT,

    CEVAXM4_SNOOP_SN_INT,
    HOST0_LINESTATE_IRQ,
    SD_DETECTN_IRQ,
    SDMMC_DECTN_MASKED_PIN_FLT,
    CIF0_IRQ,
    MIPI_DSI_HOST0_IRQ,
    SFC_INT,
    ISP_IRQ,
    CIF1_IRQ,
    CIF2_IRQ,

    CIF3_IRQ,
    MAILBOX_AP_IRQ,
    CVBS_IRQ,
    CEVAXM4_UOP_IRQ,
    INTERRUPT_ID_END,
} INTERRUPT_ID;

typedef void (interrupt_handler_t)(int, void *);

/* APIs for irqs */
void IrqRegister(int irq, interrupt_handler_t *handler, void *data);
void IrqUnregister(int irq);
int IrqSetType(int irq, unsigned int type);
int IrqEnable(int irq);
int IrqDisable(int irq);
int IrqSetPriority(int irq, int priority);
int IrqGetPriority(int irq);
void IrqHandler(void);

int InitInterrupt(void);
void EnableInterrupts(void);
void DisableInterrupts(void);

#endif

