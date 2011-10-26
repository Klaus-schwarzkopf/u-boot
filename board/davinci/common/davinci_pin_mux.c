/*
 * TI DaVinci DM365 pin multiplex
 *
 * Copyright (C) 2011 RidgeRun
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <asm/io.h>
#include <config.h>

#define DAVINCI_SYSTEM_MODULE_BASE        0x01C40000

#define PINMUX0		0x00
#define PINMUX1		0x04
#define PINMUX2		0x08
#define PINMUX3		0x0C
#define PINMUX4		0x10

#define MUX_DBG(muxreg, mode_offset, mode_mask, m0, m1, m2, m3) \
	{						\
		.reg_num = muxreg,			\
		.mux_reg = PINMUX##muxreg,		\
		.mask_offset = mode_offset,		\
		.mask = mode_mask,			\
		.muxnames = { m0, m1, m2, m3 },		\
	},

#define DM365_MUX_NR_MODES 4

struct mux_debug {
	const unsigned char reg_num;
	const unsigned char mux_reg;
	const unsigned char mask_offset;
	const unsigned char mask;
	const char *muxnames[DM365_MUX_NR_MODES];
 };

static const struct mux_debug dm365_mux_debug[]	= {
MUX_DBG(4,	30,	3,	"GIO42",	"MMCSD1_CMD",	"EM_A19",	NULL)
MUX_DBG(4,	28,	3,	"GIO41",	"MMSCD1_DATA3",	"EM_A18",	NULL)
MUX_DBG(4,	26,	3,	"GIO40",	"MMCSD1_DATA2",	"EM_A17",	NULL)
MUX_DBG(4,	24,	3,	"GIO39",	"MMCSD1_DATA1",	"EM_A16",	NULL)
MUX_DBG(4,	22,	3,	"GIO38",	"MMCSD1_DATA0",	"EM_A15",	NULL)
MUX_DBG(4,	20,	3,	"GIO37",	"SPI4_SCS[0]",	"McBSP_CLKS",	"CLKOUT0")
MUX_DBG(4,	18,	3,	"GIO36",	"SPI4_SCLK",	"EM_A21",	"EM_A14")
MUX_DBG(4,	16,	3,	"GIO35",	"SPI4_SOMI",	"SPI4_SCS[1]",	"CLKOUT1")
MUX_DBG(4,	14,	3,	"GIO34",	"SPI4_SIMO",	"SPI4_SOMI",	"UART1_RXD")
MUX_DBG(4,	12,	3,	"GIO33",	"SPI2_SCS[0]",	"USBDRVVBUS",	"R1")
MUX_DBG(4,	10,	3,	"GIO32",	"SPI2_SCLK",	NULL,		"R0")
MUX_DBG(4,	8,	3,	"GIO31",	"SPI2_SOMI",	"SPI2_SCS[1]",	"CLKOUT2")
MUX_DBG(4,	6,	3,	"GIO30",	"SPI2_SIMO",	NULL,		"G1")
MUX_DBG(4,	4,	3,	"GIO29",	"SPI1_SCS[0]",	NULL,		"G0")
MUX_DBG(4,	2,	3,	"GIO28",	"SPI1_SCLK",	NULL,		"B1")
MUX_DBG(4,	0,	3,	"GIO27",	"SPI1_SOMI",	"SPI1_SCS[1]",	"B0")

MUX_DBG(3,	31,	1,	"GIO26",	"SPI1_SIMO",	NULL,		NULL)
MUX_DBG(3,	29,	3,	"GIO25",	"SPI0_SCS[0]",	"PWM1",		"UART1_TXD")
MUX_DBG(3,	28,	1,	"GIO24",	"SPI0_SCLK",	NULL,		NULL)
MUX_DBG(3,	26,	3,	"GIO23",	"SPI0_SOMI",	"SPI0_SCS[1]",	"PWM0")
MUX_DBG(3,	25,	1,	"GIO22",	"SPI0_SIMO",	NULL,		NULL)
MUX_DBG(3,	23,	3,	"GIO21",	"UART1_RTS",	"I2C_SDA",	NULL)
MUX_DBG(3,	21,	3,	"GIO20",	"UART1_CTS",	"I2C_SCL",	NULL)
MUX_DBG(3,	20,	1,	"GIO19",	"UART0_RXD",	NULL,		NULL)
MUX_DBG(3,	19,	1,	"GIO18",	"UART0_TXD",	NULL,		NULL)
MUX_DBG(3,	17,	3,	"GIO17",	"EMAC_TX_EN",	"UART1_RXD",	NULL)
MUX_DBG(3,	15,	3,	"GIO16",	"EMAC_TX_CLK",	"UART1_TXD",	NULL)
MUX_DBG(3,	14,	1,	"GIO15",	"EMAC_COL",	NULL,		NULL)
MUX_DBG(3,	13,	1,	"GIO14",	"EMAC_TXD3",	NULL,		NULL)
MUX_DBG(3,	12,	1,	"GIO13",	"EMAC_TXD2",	NULL,		NULL)
MUX_DBG(3,	11,	1,	"GIO12",	"EMAC_TXD1",	NULL,		NULL)
MUX_DBG(3,	10,	1,	"GIO11",	"EMAC_TXD0",	NULL,		NULL)
MUX_DBG(3,	9,	1,	"GIO10",	"EMAC_RXD3",	NULL,		NULL)
MUX_DBG(3,	8,	1,	"GIO9",		"EMAC_RXD2",	NULL,		NULL)
MUX_DBG(3,	7,	1,	"GIO8",		"EMAC_RXD1",	NULL,		NULL)
MUX_DBG(3,	6,	1,	"GIO7",		"EMAC_RXD0",	NULL,		NULL)
MUX_DBG(3,	5,	1,	"GIO6",		"EMAC_RX_CLK",	NULL,		NULL)
MUX_DBG(3,	4,	1,	"GIO5",		"EMAC_RX_DV",	NULL,		NULL)
MUX_DBG(3,	3,	1,	"GIO4",		"EMAC_RX_ER",	NULL,		NULL)
MUX_DBG(3,	2,	1,	"GIO3",		"EMAC_CRS",	NULL,		NULL)
MUX_DBG(3,	1,	1,	"GIO2",		"EMAC_MDIO",	NULL,		NULL)
MUX_DBG(3,	0,	1,	"GIO1",		"EMAC_MDCLK",	NULL,		NULL)

MUX_DBG(2,	7,	1,	"EM_CE0",	"GIO56",	NULL,		NULL)
MUX_DBG(2,	6,	1,	"GIO[64:57]",	"EM_D[15:8]",	NULL,		NULL)
MUX_DBG(2,	4,	3,	"GIO72",	"EM_A7",	"EM_A7",	"KEYA3")
MUX_DBG(2,	2,	3,	"GIO68",	"EM_A3",	"EM_A3",	"KEYB3")
MUX_DBG(2,	0,	3,	"GIO[78:73,71:69,67:65]",
				"EM_A[14:8,6:4,0], EM_BA1",
				"EM_A[13:8,6:4.0] EM_BA1, EM_BA0",
				"GIO[78:73], KEYA[2:0], KEYB[2:0]")

MUX_DBG(1,	22,	1,	"VCLK",		"GIO79",	NULL,		NULL)
MUX_DBG(1,	20,	3,	"GIO80",	"EXTCLK",	"B2",		"PWM3")
MUX_DBG(1,	18,	3,	"GIO81",	"FIELD",	"R2",		"PWM3")
MUX_DBG(1,	17,	1,	"LCD_OE",	"GIO82",	NULL,		NULL)
MUX_DBG(1,	16,	1,	"HSYNC/VSYNC",	"GIO[84:83]",	NULL,		NULL)
MUX_DBG(1,	14,	3,	"GIO85",	"COUT0",	"PWM3",		NULL)
MUX_DBG(1,	12,	3,	"GIO86",	"COUT1",	"PWM3",		"STTRIG")
MUX_DBG(1,	10,	3,	"GIO87",	"COUT2",	"PWM2",		"RTO3")
MUX_DBG(1,	8,	3,	"GIO88",	"COUT3",	"PWM2",		"RTO2")
MUX_DBG(1,	6,	3,	"GIO89",	"COUT4",	"PWM2",		"RTO1")
MUX_DBG(1,	4,	3,	"GIO90",	"COUT5",	"PWM2",		"RTO0")
MUX_DBG(1,	0,	3,	"GIO92",	"COUT7",	"PWM0",		NULL)

MUX_DBG(0,	23,	1,	"GIO49",	"MCBSP0_BDX",	NULL,		NULL)
MUX_DBG(0,	22,	1,	"GIO48",	"MCBSP0_CLKX",	NULL,		NULL)
MUX_DBG(0,	21,	1,	"GIO47",	"MCBSP0_BFSX",	NULL,		NULL)
MUX_DBG(0,	20,	1,	"GIO46",	"MCBSP0_BDR",	NULL,		NULL)
MUX_DBG(0,	19,	1,	"GIO45",	"MCBSP0_CLKR",	NULL,		NULL)
MUX_DBG(0,	18,	1,	"GIO44",	"MCBSP0_BFSR",	NULL,		NULL)
MUX_DBG(0,	16,	3,	"GIO43",	"MMCSD1_CLK",	"EM_A20",	NULL)
MUX_DBG(0,	14,	3,	"C_WE_FIELD",	"GIO93",	"CLKOUT0",	"USBDRVVBUS")
MUX_DBG(0,	13,	1,	"VD",		"GIO94",	NULL,		NULL)
MUX_DBG(0,	12,	1,	"HD",		"GIO95",	NULL,		NULL)
MUX_DBG(0,	11,	1,	"YIN0",		"GPIO96",	NULL,		NULL)
MUX_DBG(0,	10,	1,	"YIN1",		"GPIO97",	NULL,		NULL)
MUX_DBG(0,	9,	1,	"YIN2",		"GPIO98",	NULL,		NULL)
MUX_DBG(0,	8,	1,	"YIN3",		"GPIO99",	NULL,		NULL)
MUX_DBG(0,	6,	3,	"YIN4",		"GIO100",	"SPI3_SOMI",	"SPI3_SCS1")
MUX_DBG(0,	4,	3,	"YIN5",		"GIO101",	"SPI3_SCS0",	NULL)
MUX_DBG(0,	2,	3,	"YIN6",		"GIO102",	"SPI3_SIMO",	NULL)
MUX_DBG(0,	0,	3,	"YIN7",		"GIO103",	"SPI3_SCLK",	NULL)
};

#define DM365_MUX_MAX_NR_FLAGS	10
#define DM365_MUX_TEST_FLAG(val, mask)				\
	if (((val) & (mask)) == (mask)) {			\
		i++;						\
		flags[i] =  #mask;				\
	}

#define DM365_MUX_DEFNAME_LEN	16

int pinmux_info(void)
{
  void  *base = (void *)DAVINCI_SYSTEM_MODULE_BASE;
	int i;

	for (i = 0; i < ARRAY_SIZE(dm365_mux_debug); i++) {
		const struct mux_debug *m = &dm365_mux_debug[i];
		u32 val;
		int j, mode;

		val = __raw_readl(base + m->mux_reg);
		mode = (val >> m->mask_offset) & m->mask;

		printf("[0x%p: 0x%8.8X] -- ", base + m->mux_reg, val);
		if (mode < DM365_MUX_NR_MODES) {
		  printf("PINMUX%d:%d = %s[%d] ( ", m->reg_num, m->mask_offset,
			     m->muxnames[mode], mode);
			for (j = 0; j < DM365_MUX_NR_MODES; j++) {
				if (m->muxnames[j]) {
					printf("%d-%s ", j, m->muxnames[j]);
				}
			}
			printf(")\n");
		} else {
			printf("PINMUX%d:%d = [%d]\n",  m->reg_num, m->mask_offset,
				   mode);
		}
	}

	return 0;
}
