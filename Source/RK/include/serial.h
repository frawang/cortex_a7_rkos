/*
 * Copyright 2018 Rockchip Electronics Co., Ltd
 *
 */

#ifndef __DEBUGSERIAL_H
#define __DEBUGSERIAL_H

#include "FreeRTOSConfig.h"

void serial_init (void);
void serial_printf(const char* Format, ...);

#undef debug
#if configRK_JTAG_IOMUX_ENABLE == 0
#define debug(x...)	serial_printf(x)
#else
#define debug(x...)	do { } while (0)
#endif

#endif  /* __DEBUGSERIAL_H */
