/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <imx_mipi_dsi_bridge.h>
#include <mipi_display.h>
#include <linux/compat.h>

#define CMD_TABLE_LEN 2
typedef u8 cmd_set_table[CMD_TABLE_LEN];

/* Write Manufacture Command Set Control */
#define WRMAUCCTR 0xFE

/* Manufacturer Command Set pages (CMD2) */
static const cmd_set_table manufacturer_cmd_set[] = {
	{0xFE, 0xD0},
	{0x40, 0x02},
	{0x4B, 0x4C},
	{0x49, 0x01},
	{0xFE, 0x70},
	{0x48, 0x05},
	{0x52, 0x00},
	{0x5A, 0xFF},
	{0x5C, 0xF6},
	{0x5D, 0x07},
	{0x7D, 0x35},
	{0x86, 0x07},
	{0xA7, 0x02},
	{0xA9, 0x2C},
	{0xFE, 0xA0},
	{0x2B, 0x18},
	{0xFE, 0x90},
	{0x26, 0x10},
	{0x28, 0x20},
	{0x2A, 0x40},
	{0x2D, 0x60},
	{0x30, 0x70},
	{0x32, 0x80},
	{0x34, 0x90},
	{0x36, 0x98},
	{0x38, 0xA0},
	{0x3A, 0xC0},
	{0x3D, 0xE0},
	{0x40, 0xF0},
	{0x42, 0x00},
	{0x43, 0x01},
	{0x44, 0x40},
	{0x45, 0x01},
	{0x46, 0x80},
	{0x47, 0x01},
	{0x48, 0xC0},
	{0x49, 0x01},
	{0x4A, 0x00},
	{0x4B, 0x02},
	{0x4C, 0x40},
	{0x4D, 0x02},
	{0x4E, 0x80},
	{0x4F, 0x02},
	{0x50, 0x00},
	{0x51, 0x03},
	{0x52, 0x80},
	{0x53, 0x03},
	{0x54, 0x00},
	{0x55, 0x04},
	{0x56, 0x8D},
	{0x58, 0x04},
	{0x59, 0x20},
	{0x5A, 0x05},
	{0x5B, 0xBD},
	{0x5C, 0x05},
	{0x5D, 0x63},
	{0x5E, 0x06},
	{0x5F, 0x13},
	{0x60, 0x07},
	{0x61, 0xCD},
	{0x62, 0x07},
	{0x63, 0x91},
	{0x64, 0x08},
	{0x65, 0x60},
	{0x66, 0x09},
	{0x67, 0x38},
	{0x68, 0x0A},
	{0x69, 0x1A},
	{0x6A, 0x0B},
	{0x6B, 0x07},
	{0x6C, 0x0C},
	{0x6D, 0xFE},
	{0x6E, 0x0C},
	{0x6F, 0x00},
	{0x70, 0x0E},
	{0x71, 0x0C},
	{0x72, 0x0F},
	{0x73, 0x96},
	{0x74, 0x0F},
	{0x75, 0xDC},
	{0x76, 0x0F},
	{0x77, 0xFF},
	{0x78, 0x0F},
	{0x79, 0x00},
	{0x7A, 0x00},
	{0x7B, 0x00},
	{0x7C, 0x01},
	{0x7D, 0x02},
	{0x7E, 0x04},
	{0x7F, 0x08},
	{0x80, 0x10},
	{0x81, 0x20},
	{0x82, 0x30},
	{0x83, 0x40},
	{0x84, 0x50},
	{0x85, 0x60},
	{0x86, 0x70},
	{0x87, 0x78},
	{0x88, 0x88},
	{0x89, 0x96},
	{0x8A, 0xA3},
	{0x8B, 0xAF},
	{0x8C, 0xBA},
	{0x8D, 0xC4},
	{0x8E, 0xCE},
	{0x8F, 0xD7},
	{0x90, 0xE0},
	{0x91, 0xE8},
	{0x92, 0xF0},
	{0x93, 0xF8},
	{0x94, 0xFF},
	{0x99, 0x20},
	{0xFE, 0x00},
	{0xC2, 0x08},
	{0x35, 0x00},
	{0x36, 0x02},
	{0x11, 0x00},
	{0x29, 0x00},
};

static u8 color_format_from_dsi_format(enum mipi_dsi_pixel_format format)
{
	switch (format) {
	case MIPI_DSI_FMT_RGB565:
		return 0x55;
	case MIPI_DSI_FMT_RGB666:
	case MIPI_DSI_FMT_RGB666_PACKED:
		return 0x66;
	case MIPI_DSI_FMT_RGB888:
		return 0x77;
	default:
		return 0x77; /* for backward compatibility */
	}
};

static int mipi_dsi_generic_write(const void *payload, size_t size)
{
	int ret;
	u16 tx_buf;
	u8 *tx;

	tx_buf = 0;
	tx = (u8 *)&tx_buf;

	switch (size) {
	case 0:
		ret = imx_mipi_dsi_bridge_pkt_write(MIPI_DSI_GENERIC_SHORT_WRITE_0_PARAM, tx, 0);
		break;

	case 1:
		tx[0] = *(u8 *)payload;
		ret = imx_mipi_dsi_bridge_pkt_write(MIPI_DSI_GENERIC_SHORT_WRITE_1_PARAM, tx, 0);
		break;

	case 2:
		ret = imx_mipi_dsi_bridge_pkt_write(MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM, (const u8 *)payload, 0);
		break;

	default:
		ret = imx_mipi_dsi_bridge_pkt_write(MIPI_DSI_GENERIC_LONG_WRITE, (const u8 *)payload, size);
		break;
	}

	return ret;
}

static int mipi_dsi_dcs_write(u8 cmd, const void *data, size_t len)
{
	u32 size;
	u8 *tx;
	u16 tx_buf;
	int ret;

	if (len > 0) {
		size = 1 + len;

		tx = kmalloc(size, GFP_KERNEL);
		if (!tx)
			return -ENOMEM;

		/* concatenate the DCS command byte and the payload */
		tx[0] = cmd;
		memcpy(&tx[1], data, len);
	} else {
		tx = (u8 *)&tx_buf;
		tx[0] = cmd;
		tx[1] = 0;
		size = 1;
	}

	switch (size) {
	case 1:
		ret = imx_mipi_dsi_bridge_pkt_write(MIPI_DSI_DCS_SHORT_WRITE, tx, 0);
		break;

	case 2:
		ret = imx_mipi_dsi_bridge_pkt_write(MIPI_DSI_DCS_SHORT_WRITE_PARAM, tx, 0);
		break;

	default:
		ret = imx_mipi_dsi_bridge_pkt_write(MIPI_DSI_DCS_LONG_WRITE, tx, size);
		break;
	}

	if (len > 0)
		kfree(tx);

	return ret;
}

static int rad_panel_push_cmd_list(void)
{
	size_t i;
	const u8 *cmd;
	size_t count = sizeof(manufacturer_cmd_set) / CMD_TABLE_LEN;
	int ret = 0;

	for (i = 0; i < count ; i++) {
		cmd = manufacturer_cmd_set[i];
		ret = mipi_dsi_generic_write(cmd, CMD_TABLE_LEN);
		if (ret < 0)
			return ret;
	}

	return ret;
};

int m67198_lcd_setup(struct mipi_dsi_client_dev *panel_dev)
{
	u8 color_format = color_format_from_dsi_format(panel_dev->format);
	u16 brightness;
	int ret;

	ret = rad_panel_push_cmd_list();
	if (ret < 0) {
		printf("Failed to send MCS (%d)\n", ret);
		return -EIO;
	}

	/* Select User Command Set table (CMD1) */
	ret = mipi_dsi_generic_write((u8[]){ WRMAUCCTR, 0x00 }, 2);
	if (ret < 0)
		return -EIO;

	/* Software reset */
	ret = mipi_dsi_dcs_write(MIPI_DCS_SOFT_RESET, NULL, 0);
	if (ret < 0) {
		printf("Failed to do Software Reset (%d)\n", ret);
		return -EIO;
	}

	mdelay(10);

	/* Set DSI mode */
	ret = mipi_dsi_generic_write((u8[]){ 0xC2, 0x0B }, 2);
	if (ret < 0) {
		printf("Failed to set DSI mode (%d)\n", ret);
		return -EIO;
	}

	/* Set tear ON */
	ret = mipi_dsi_dcs_write(MIPI_DCS_SET_TEAR_ON, (u8[]){ 0x0 }, 1);
	if (ret < 0) {
		printf("Failed to set tear ON (%d)\n", ret);
		return -EIO;
	}

	/* Set tear scanline */
	ret = mipi_dsi_generic_write((u8[]){ MIPI_DCS_SET_TEAR_SCANLINE, 0x3,0x80},  3);
	if (ret < 0) {
		printf("Failed to set tear scanline (%d)\n", ret);
		return -EIO;
	}

	/* Set pixel format */
	ret = mipi_dsi_dcs_write(MIPI_DCS_SET_PIXEL_FORMAT, &color_format, 1);
	if (ret < 0) {
		printf("Failed to set pixel format (%d)\n", ret);
		return -EIO;
	}

	/* Set display brightness */
	brightness = 255; /* Max brightness */
	ret = mipi_dsi_dcs_write(MIPI_DCS_SET_DISPLAY_BRIGHTNESS, &brightness, 2);
	if (ret < 0) {
		printf("Failed to set display brightness (%d)\n",
				  ret);
		return -EIO;
	}

	/* Exit sleep mode */
	ret = mipi_dsi_dcs_write(MIPI_DCS_EXIT_SLEEP_MODE, NULL, 0);
	if (ret < 0) {
		printf("Failed to exit sleep mode (%d)\n", ret);
		return -EIO;
	}

	mdelay(5);

	ret = mipi_dsi_dcs_write(MIPI_DCS_SET_DISPLAY_ON, NULL, 0);
	if (ret < 0) {
		printf("Failed to set display ON (%d)\n", ret);
		return -EIO;
	}

	return 0;
}

static struct mipi_dsi_client_driver m67198_drv = {
	.name = "RM67198_OLED",
	.dsi_client_setup = m67198_lcd_setup,
};

void rm67198_init(void)
{
	imx_mipi_dsi_bridge_add_client_driver(&m67198_drv);
}
