/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef __MIPI_DSI_PANEL_H
#define __MIPI_DSI_PANEL_H

void hx8363_init(void);
void rm67191_init(void);
#ifdef CONFIG_TARGET_IMX8MN_IWG37M
/* IWG37M: MIPI-DSI: Supporting RM67198 dispaly driver */
void rm67198_init(void);
#endif
void rm68200_init(void);

#endif
