/*
 * Copyright (c) 2020 iWave Systems Technologies Pvt. Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm-generic/gpio.h>
#include <fsl_esdhc.h>
#include <mmc.h>
#include <asm/arch/imx8mn_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/arch/clock.h>
#include <spl.h>
#include <asm/mach-imx/dma.h>
#include <power/pmic.h>
#include <power/bd71837.h>
#include "../common/tcpc.h"

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_FSEL1)

static iomux_v3_cfg_t const uart_pads[] = {
	IMX8MN_PAD_UART4_RXD__UART4_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MN_PAD_UART4_TXD__UART4_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const wdog_pads[] = {
	IMX8MN_PAD_GPIO1_IO02__WDOG1_WDOG_B  | MUX_PAD_CTRL(WDOG_PAD_CTRL),
};

#ifdef CONFIG_FSL_FSPI
#define QSPI_PAD_CTRL	(PAD_CTL_DSE2 | PAD_CTL_HYS)
static iomux_v3_cfg_t const qspi_pads[] = {
	IMX8MN_PAD_NAND_ALE__QSPI_A_SCLK | MUX_PAD_CTRL(QSPI_PAD_CTRL | PAD_CTL_PE | PAD_CTL_PUE),
	IMX8MN_PAD_NAND_CE0_B__QSPI_A_SS0_B | MUX_PAD_CTRL(QSPI_PAD_CTRL),

	IMX8MN_PAD_NAND_DQS__QSPI_A_DQS | MUX_PAD_CTRL(QSPI_PAD_CTRL) | MUX_MODE_SION,
	IMX8MN_PAD_NAND_DATA00__QSPI_A_DATA0 | MUX_PAD_CTRL(QSPI_PAD_CTRL),
	IMX8MN_PAD_NAND_DATA01__QSPI_A_DATA1 | MUX_PAD_CTRL(QSPI_PAD_CTRL),
	IMX8MN_PAD_NAND_DATA02__QSPI_A_DATA2 | MUX_PAD_CTRL(QSPI_PAD_CTRL),
	IMX8MN_PAD_NAND_DATA03__QSPI_A_DATA3 | MUX_PAD_CTRL(QSPI_PAD_CTRL),
};

int board_qspi_init(void)
{
	imx_iomux_v3_setup_multiple_pads(qspi_pads, ARRAY_SIZE(qspi_pads));

	set_clk_qspi();

	return 0;
}
#endif

#ifdef CONFIG_MXC_SPI
#define SPI_PAD_CTRL	(PAD_CTL_DSE2 | PAD_CTL_HYS)
static iomux_v3_cfg_t const ecspi1_pads[] = {
	IMX8MN_PAD_ECSPI1_SCLK__ECSPI1_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL),
	IMX8MN_PAD_ECSPI1_MOSI__ECSPI1_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL),
	IMX8MN_PAD_ECSPI1_MISO__ECSPI1_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL),
	IMX8MN_PAD_ECSPI1_SS0__GPIO5_IO9 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const ecspi2_pads[] = {
	IMX8MN_PAD_ECSPI2_SCLK__ECSPI2_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL),
	IMX8MN_PAD_ECSPI2_MOSI__ECSPI2_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL),
	IMX8MN_PAD_ECSPI2_MISO__ECSPI2_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL),
	IMX8MN_PAD_ECSPI2_SS0__GPIO5_IO13 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void setup_spi(void)
{
	imx_iomux_v3_setup_multiple_pads(ecspi1_pads, ARRAY_SIZE(ecspi1_pads));
	imx_iomux_v3_setup_multiple_pads(ecspi2_pads, ARRAY_SIZE(ecspi2_pads));
	gpio_request(IMX_GPIO_NR(5, 9), "ECSPI1 CS");
	gpio_request(IMX_GPIO_NR(5, 13), "ECSPI2 CS");
}

int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	if (bus == 0)
		return IMX_GPIO_NR(5, 9);
	else
		return IMX_GPIO_NR(5, 13);
}
#endif

#ifdef CONFIG_NAND_MXS
#define NAND_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_FSEL2 | PAD_CTL_HYS)
#define NAND_PAD_READY0_CTRL (PAD_CTL_DSE6 | PAD_CTL_FSEL2 | PAD_CTL_PUE)
static iomux_v3_cfg_t const gpmi_pads[] = {
	IMX8MN_PAD_NAND_ALE__RAWNAND_ALE | MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MN_PAD_NAND_CE0_B__RAWNAND_CE0_B | MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MN_PAD_NAND_CLE__RAWNAND_CLE | MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MN_PAD_NAND_DATA00__RAWNAND_DATA00 | MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MN_PAD_NAND_DATA01__RAWNAND_DATA01 | MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MN_PAD_NAND_DATA02__RAWNAND_DATA02 | MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MN_PAD_NAND_DATA03__RAWNAND_DATA03 | MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MN_PAD_NAND_DATA04__RAWNAND_DATA04 | MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MN_PAD_NAND_DATA05__RAWNAND_DATA05	| MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MN_PAD_NAND_DATA06__RAWNAND_DATA06	| MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MN_PAD_NAND_DATA07__RAWNAND_DATA07	| MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MN_PAD_NAND_RE_B__RAWNAND_RE_B | MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MN_PAD_NAND_READY_B__RAWNAND_READY_B | MUX_PAD_CTRL(NAND_PAD_READY0_CTRL),
	IMX8MN_PAD_NAND_WE_B__RAWNAND_WE_B | MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MN_PAD_NAND_WP_B__RAWNAND_WP_B | MUX_PAD_CTRL(NAND_PAD_CTRL),
};

static void setup_gpmi_nand(void)
{
	imx_iomux_v3_setup_multiple_pads(gpmi_pads, ARRAY_SIZE(gpmi_pads));
}
#endif

int board_early_init_f(void)
{
	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));

#ifdef CONFIG_NAND_MXS
	setup_gpmi_nand(); /* SPL will call the board_early_init_f */
#endif

	return 0;
}

#ifdef CONFIG_BOARD_POSTCLK_INIT
int board_postclk_init(void)
{
	/* TODO */
	return 0;
}
#endif

int dram_init(void)
{
	/* rom_pointer[1] contains the size of TEE occupies */
	if (rom_pointer[1])
		gd->ram_size = PHYS_SDRAM_SIZE - rom_pointer[1];
	else
		gd->ram_size = PHYS_SDRAM_SIZE;

	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
	return 0;
}
#endif
int board_init(void)
{

#ifdef CONFIG_MXC_SPI
	setup_spi();
#endif

#ifdef CONFIG_FSL_FSPI
	board_qspi_init();
#endif

	return 0;
}


#define GPIO_PAD_CFG_CTRL ( PAD_CTL_DSE0 | PAD_CTL_ODE | PAD_CTL_PUE )

#define BCONFIG_0 IMX_GPIO_NR(1, 9)
#define BCONFIG_1 IMX_GPIO_NR(5, 1)
#define BCONFIG_2 IMX_GPIO_NR(4, 25)
#define BCONFIG_3 IMX_GPIO_NR(4, 26)
#define BCONFIG_4 IMX_GPIO_NR(4, 27)
#define BCONFIG_5 IMX_GPIO_NR(2, 19)
#define BCONFIG_6 IMX_GPIO_NR(2, 20)

int const board_config_pads[] = {
       BCONFIG_0,
       BCONFIG_1,
       BCONFIG_2,
       BCONFIG_3,
       BCONFIG_4,
       BCONFIG_5,
       BCONFIG_6,
};

static iomux_v3_cfg_t const board_cfg[] = {
      IMX8MN_PAD_GPIO1_IO09__GPIO1_IO9 | MUX_PAD_CTRL(GPIO_PAD_CFG_CTRL),
      IMX8MN_PAD_SAI3_TXD__GPIO5_IO1 | MUX_PAD_CTRL(GPIO_PAD_CFG_CTRL),
      IMX8MN_PAD_SAI2_TXC__GPIO4_IO25 | MUX_PAD_CTRL(GPIO_PAD_CFG_CTRL),
      IMX8MN_PAD_SAI2_TXD0__GPIO4_IO26 | MUX_PAD_CTRL(GPIO_PAD_CFG_CTRL),
      IMX8MN_PAD_SAI2_MCLK__GPIO4_IO27 | MUX_PAD_CTRL(GPIO_PAD_CFG_CTRL),
      IMX8MN_PAD_SD2_RESET_B__GPIO2_IO19 | MUX_PAD_CTRL(GPIO_PAD_CFG_CTRL),
      IMX8MN_PAD_SD2_WP__GPIO2_IO20 | MUX_PAD_CTRL(GPIO_PAD_CFG_CTRL),
};

/* IWG37M: Print Board Information */
static void printboard_info(void)
{
     int i, bom_rev = 0, pcb_rev = 0, revision = 0;

     imx_iomux_v3_setup_multiple_pads(board_cfg, ARRAY_SIZE(board_cfg));

     for (i=0;i<ARRAY_SIZE(board_config_pads);i++) 
     {
	   gpio_request(board_config_pads[i], "SOM-Revision-GPIO");
	   gpio_direction_input(board_config_pads[i]);
	   revision |= gpio_get_value(board_config_pads[i]) << i;
     }

     pcb_rev = ((revision) & 0x03) +1;
     bom_rev = (((revision) & 0x78)>>3);

     printf ("\n");
     printf ("Board Info:\n");
     printf ("\tBSP Version     : %s\n", BSP_VERSION);
     printf ("\tSOM Version     : iW-PRGJZ-AP-01-R%x.%x\n",pcb_rev,bom_rev);
     printf ("\n");
}


static void peripheral_reset(void)
{
        gpio_request(IMX_GPIO_NR(5, 2), "peripheral reset");
        gpio_direction_output(IMX_GPIO_NR(5, 2), 0);
        mdelay(100);
        gpio_direction_output(IMX_GPIO_NR(5, 2), 1);
	
}

int board_late_init(void)
{
	printboard_info();
	peripheral_reset();

	return 0;
}

#ifdef CONFIG_FSL_FASTBOOT
#ifdef CONFIG_ANDROID_RECOVERY
int is_recovery_key_pressing(void)
{
	return 0; /*TODO*/
}
#endif /*CONFIG_ANDROID_RECOVERY*/
#endif /*CONFIG_FSL_FASTBOOT*/
