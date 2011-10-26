/*
 * Copyright (C) 2010 RidgeRun
 *
 * Author: Pablo Barrantes <pablo.barrantes@ridgerun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <nand.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/emif_defs.h>
#include <asm/arch/nand_defs.h>
#include <asm/arch/gpio_defs.h>
#include <netdev.h>
#include "../common/misc.h"
#ifdef CONFIG_DAVINCI_MMC
#include <mmc.h>
#include <asm/arch/sdmmc_defs.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	int i=0;

	struct davinci_gpio *gpio1_base =
			(struct davinci_gpio *)DAVINCI_GPIO_BANK01;
	struct davinci_gpio *gpio23_base =
			(struct davinci_gpio *)DAVINCI_GPIO_BANK23;

	gd->bd->bi_arch_number = MACH_TYPE_DM365_LEOPARD;
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;


	/* PINMUX2: Enable AEMIF pins and GPIO[64:57] */
	writel((readl(PINMUX2) & 0x00000015), PINMUX2);

	/* Disable NAND write protection */
	for(i=0; i < 20; i++) {
		/* Configure GPIO59 as output */
		writel((readl(&gpio23_base->dir) & ~(1 << 27)), &gpio23_base->dir);

		/* GPIO 59 high */
		writel((readl(&gpio23_base->out_data) | (1 << 27)), &gpio23_base->out_data);
	}

	/* Enable I2C bus */
	REG(PINMUX3) |= 0x01400000;

	/* Enable SPI0 ports */
/*	writel(((readl(PINMUX3) & 0x81FFFFFF) | 0x36000000), PINMUX3);	*/
/*	REG(PINMUX3) |= 0x36000000;	*/

	/* Enable SPI1 ports */
	writel(((readl(PINMUX3) & 0x7FFFFFFF) | 0x80000000), PINMUX3);
	writel(((readl(PINMUX4) & 0xFFFFFFC0) | 0x00000015), PINMUX4);
//	REG(PINMUX3) |= 0x80000000;
//	REG(PINMUX4) |= 0x00000015;

	/* Enable CLKOUT0 (CMOS_OE) */
    /* Disable output CLKOUT0  */
    REG(0x01C40048) |= 1;
    /* Enable output CLKOUT0 */
    REG(0x01C40048) &= ~1;
    /* PLL1 enable output OBSCLK for CLKOUT0 */
    REG(0x01C40800 + 0x148) |= 0x2;
	/* PINMUX4 to select CLKOUT0 */
    REG(PINMUX4) |= 0x00300000;

	/* PINMUX4: Select GIO31 (CMOS_RST) & GIO36 (AIC31_RST), image sensor and audio codec reset */
	REG(PINMUX4) &= 0xfff3fcff;

	/* Reset image sensor */
	for (i=0; i < 20; i++)
	{
		/* Configure GPIO31 as output */
		writel((readl(&gpio1_base->dir) & ~(1 << 31)), &gpio1_base->dir);

		/* GPIO 31 low */
		writel((readl(&gpio1_base->out_data) | (0 << 31)),
						&gpio1_base->out_data);

		/* GPIO 31 high */
		writel((readl(&gpio1_base->out_data) | (1 << 31)),
						&gpio1_base->out_data);
	}

	/* Reset AIC3104 audio codec */
	for (i=0; i < 20; i++)
	{
		/* Configure GPIO36 as output  */
		writel((readl(&gpio23_base->dir) & ~(1 << 4)), &gpio23_base->dir);

		/*GPIO 36 low */
		writel((readl(&gpio23_base->out_data) | (0 << 4)),
						&gpio23_base->out_data);

		/*GPIO 36 high */
		writel((readl(&gpio23_base->out_data) | (1 << 4)),
						&gpio23_base->out_data);

	}

	return 0;
}

#ifdef CONFIG_DRIVER_TI_EMAC
int board_eth_init(bd_t *bis)
{
	int i;
	struct davinci_gpio *gpio1_base =
			(struct davinci_gpio *)DAVINCI_GPIO_BANK01;

	/* Configure PINMUX 3 to enable EMAC pins */
	writel((readl(PINMUX3) | 0x1affff), PINMUX3);

	/* Configure GPIO20 as output */
	writel((readl(&gpio1_base->dir) & ~(1 << 20)), &gpio1_base->dir);

	/* Toggle GPIO 20 */
	for (i = 0; i < 20; i++) {
		/* GPIO 20 low */
		writel((readl(&gpio1_base->out_data) & ~(1 << 20)),
						&gpio1_base->out_data);

		udelay(1000);

		/* GPIO 20 high */
		writel((readl(&gpio1_base->out_data) | (1 << 20)),
						&gpio1_base->out_data);
	}

	davinci_emac_initialize();

	return 0;
}
#endif

#ifdef CONFIG_NAND_DAVINCI
int board_nand_init(struct nand_chip *nand)
{
	davinci_nand_init(nand);

	return 0;
}
#endif
#ifdef CONFIG_DAVINCI_MMC

static struct davinci_mmc mmc_sd0 = {
	.reg_base = (struct davinci_mmc_regs *)DAVINCI_MMC_SD0_BASE,
	.input_clk = 121500000,
	.host_caps = MMC_MODE_4BIT,
	.voltages = MMC_VDD_32_33 | MMC_VDD_33_34,
	.version = MMC_CTLR_VERSION_2,
};

#ifdef CONFIG_DAVINCI_MMC_SD1
static struct davinci_mmc mmc_sd1 = {
	.reg_base = (struct davinci_mmc_regs *)DAVINCI_MMC_SD1_BASE,
	.input_clk = 121500000,
	.host_caps = MMC_MODE_4BIT,
	.voltages = MMC_VDD_32_33 | MMC_VDD_33_34,
	.version = MMC_CTLR_VERSION_2,
};
#endif

int board_mmc_init(bd_t *bis)
{
	int err;

	/* Add slot-0 to mmc subsystem */
	err = davinci_mmc_init(bis, &mmc_sd0);
	if (err)
		return err;

#ifdef CONFIG_DAVINCI_MMC_SD1
#define PUPDCTL1		0x01c4007c
	/* PINMUX(4)-DAT0-3/CMD;  PINMUX(0)-CLK */
	writel((readl(PINMUX4) | 0x55400000), PINMUX4);
	writel((readl(PINMUX0) | 0x00010000), PINMUX0);

	/* Configure MMC/SD pins as pullup */
	writel((readl(PUPDCTL1) & ~0x07c0), PUPDCTL1);

	/* Add slot-1 to mmc subsystem */
	err = davinci_mmc_init(bis, &mmc_sd1);
#endif

	return err;
}

#endif
