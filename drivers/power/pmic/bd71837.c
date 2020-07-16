/*
 * Copyright 2018 NXP  *
 * SPDX-License-Identifier:      GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/bd71837.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct pmic_child_info pmic_children_info[] = {
#ifdef CONFIG_TARGET_IMX8MN_IWG37M
	/* IWG37M: PMIC: BD718XX */
	/* buck */
	{ .prefix = "b", .driver = BD718XX_REGULATOR_DRIVER},
	/* ldo */
	{ .prefix = "l", .driver = BD718XX_REGULATOR_DRIVER},
#else
	/* buck */
	{ .prefix = "b", .driver = BD71837_REGULATOR_DRIVER},
	/* ldo */
	{ .prefix = "l", .driver = BD71837_REGULATOR_DRIVER},
	{ },
#endif
};

static int bd71837_reg_count(struct udevice *dev)
{
#ifdef CONFIG_TARGET_IMX8MN_IWG37M
	/* IWG37M: PMIC: BD718X7 return value of number of registers */
	return BD718XX_MAX_REGISTER - 1;
#else
	return BD71837_REG_NUM;
#endif
}

static int bd71837_write(struct udevice *dev, uint reg, const uint8_t *buff,
			  int len)
{
	if (dm_i2c_write(dev, reg, buff, len)) {
		pr_err("write error to device: %p register: %#x!", dev, reg);
		return -EIO;
	}

	return 0;
}

static int bd71837_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	if (dm_i2c_read(dev, reg, buff, len)) {
		pr_err("read error from device: %p register: %#x!", dev, reg);
		return -EIO;
	}

	return 0;
}

static int bd71837_bind(struct udevice *dev)
{
	int children;
	ofnode regulators_node;

	regulators_node = dev_read_subnode(dev, "regulators");
	if (!ofnode_valid(regulators_node)) {
		debug("%s: %s regulators subnode not found!", __func__,
		      dev->name);
		return -ENXIO;
	}

	debug("%s: '%s' - found regulators subnode\n", __func__, dev->name);

	children = pmic_bind_children(dev, regulators_node, pmic_children_info);
	if (!children)
		debug("%s: %s - no child found\n", __func__, dev->name);

	/* Always return success for this device */
	return 0;
}
#ifdef CONFIG_TARGET_IMX8MN_IWG37M
/* IWG37M: PMIC: BD718X7 probe function */
static int bd718x7_probe(struct udevice *dev)
{
	int ret;
	u8 unlock;

	/* Unlock the PMIC regulator control before probing the children */
	ret = pmic_reg_read(dev, BD718XX_REGLOCK);
	if (ret < 0) {
		debug("%s: %s Failed to read lock register, error %d\n",
		      __func__, dev->name, ret);
		return ret;
	}

	unlock = ret;
	unlock &= ~(BD718XX_REGLOCK_PWRSEQ | BD718XX_REGLOCK_VREG);

	ret = pmic_reg_write(dev, BD718XX_REGLOCK, unlock);
	if (ret) {
		debug("%s: %s Failed to unlock regulator control\n", __func__,
		      dev->name);
		return ret;
	}
	debug("%s: '%s' - BD718x7 PMIC register unlocked\n", __func__,
	      dev->name);

	return 0;
}
#endif

static struct dm_pmic_ops bd71837_ops = {
	.reg_count = bd71837_reg_count,
	.read = bd71837_read,
	.write = bd71837_write,
};

static const struct udevice_id bd71837_ids[] = {
#ifdef CONFIG_TARGET_IMX8MN_IWG37M
/* IWG37M: PMIC:Support for PMIC BD71837 and BD71847 */
	{ .compatible = "rohm,bd71837", .data = ROHM_CHIP_TYPE_BD71837, },
	{ .compatible = "rohm,bd71847", .data = ROHM_CHIP_TYPE_BD71847, },
#else
	{ .compatible = "rohm,bd71837", .data = 0x4b, },
	{ }
#endif
};

U_BOOT_DRIVER(pmic_bd71837) = {
	.name = "bd71837 pmic",
	.id = UCLASS_PMIC,
	.of_match = bd71837_ids,
	.bind = bd71837_bind,
#ifdef CONFIG_TARGET_IMX8MN_IWG37M
	/* IWG37M: PMIC:Probe function for PMIC BD718X7 */
	.probe = bd718x7_probe,
#endif
	.ops = &bd71837_ops,
};
