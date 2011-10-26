/*
 * Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Driver for SPI controller on DaVinci. Based on atmel_spi.c
 * by Atmel Corporation
 *
 * Copyright (C) 2007 Atmel Corporation
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <spi.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/byteorder.h>
#include <asm/arch/hardware.h>
#include "davinci_spi.h"

struct davinci_spi_capabilities {
	void *base_address;
	unsigned int max_cs;
};

static const struct davinci_spi_capabilities dm365_spi_capabilities[] = {
	{
		.base_address = (void *)0x01C66000,
		.max_cs = 2,
	},
	{
		.base_address = (void *)0x01C66800,
		.max_cs = 2,
	},
	{
		.base_address = (void *)0x01C67800,
		.max_cs = 2,
	},
	{
		.base_address = (void *)0x01C68000,
		.max_cs = 2,
	},
	{
		.base_address = (void *)0x01C23000,
		.max_cs = 2,
	},
};

void spi_init()
{
	/* do nothing */
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
			unsigned int max_hz, unsigned int mode)
{
	struct davinci_spi_slave	*ds;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	ds = malloc(sizeof(*ds));
	if (!ds)
		return NULL;

	ds->slave.bus = bus;
	ds->slave.cs = cs;
	ds->regs = (struct davinci_spi_regs *)CONFIG_SYS_SPI_BASE;
	ds->freq = max_hz;

	return &ds->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct davinci_spi_slave *ds = to_davinci_spi(slave);

	free(ds);
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct davinci_spi_slave *ds = to_davinci_spi(slave);
	unsigned int scalar;

	/* Enable the SPI hardware */
	writel(SPIGCR0_SPIRST_MASK, &ds->regs->gcr0);
	udelay(1000);
	writel(SPIGCR0_SPIENA_MASK, &ds->regs->gcr0);

	/* Set master mode, powered up and not activated */
	writel(SPIGCR1_MASTER_MASK | SPIGCR1_CLKMOD_MASK, &ds->regs->gcr1);

	/* CS, CLK, SIMO and SOMI are functional pins */
	writel((SPIPC0_EN0FUN_MASK | SPIPC0_CLKFUN_MASK |
		SPIPC0_DOFUN_MASK | SPIPC0_DIFUN_MASK), &ds->regs->pc0);

	/* setup format */
	scalar = ((CONFIG_SYS_SPI_CLK / ds->freq) - 1) & 0xFF;

	/*
	 * Use following format:
	 *   clock signal delayed by half clk cycle,
	 *   clock low in idle state - Mode 0,
	 *   MSB shifted out first
	 */
	writel((scalar << SPIFMT_PRESCALE_SHIFT) |
		(1 << SPIFMT_PHASE_SHIFT), &ds->regs->fmt0);

#if 0
	/*
	 * Including a minor delay. No science here. Should be good even with
	 * no delay
	 */
	writel((50 << SPI_C2TDELAY_SHIFT) |
		(50 << SPI_T2CDELAY_SHIFT), &ds->regs->delay);
#endif
	/* default chip select register */
	writel(SPIDEF_CSDEF0_MASK, &ds->regs->def);

	/* no interrupts */
	writel(0, &ds->regs->int0);
	writel(0, &ds->regs->lvl);

	/* enable SPI */
	writel((readl(&ds->regs->gcr1) | SPIGCR1_SPIENA_MASK), &ds->regs->gcr1);

	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	struct davinci_spi_slave *ds = to_davinci_spi(slave);

	/* Disable the SPI hardware */
	writel(SPIGCR0_SPIRST_MASK, &ds->regs->gcr0);
}

/*
 * This functions needs to act like a macro to avoid pipeline reloads in the
 * loops below. Use always_inline. This gains us about 160KiB/s and the bloat
 * appears to be zero bytes (da830).
 */
__attribute__((always_inline))
static inline u32 davinci_spi_xfer_data(struct davinci_spi_slave *ds, u32 data)
{
	u32	buf_reg_val;

	/* send out data */
	writel(data, &ds->regs->dat1);

	/* wait for the data to clock in/out */
	while ((buf_reg_val = readl(&ds->regs->buf)) & SPIBUF_RXEMPTY_MASK)
		;

	return buf_reg_val;
}

static int davinci_spi_read(struct spi_slave *slave, unsigned int len,
			    unsigned int bitsperxfer, void *rxp,
			    unsigned long flags)
{
	struct davinci_spi_slave *ds = to_davinci_spi(slave);
	unsigned int data1_reg_val;
	u8 *rxpb = rxp; /* 8 bits / xfer or less */
	u16 *rxpw = rxp; /* 9 bits / xfer or more */
	u16 mask = (1 << bitsperxfer) - 1;

	/* enable CS hold, CS[n] and clear the data bits */
	data1_reg_val = ((1 << SPIDAT1_CSHOLD_SHIFT) |
			 (slave->cs << SPIDAT1_CSNR_SHIFT));

	/* wait till TXFULL is deasserted */
	while (readl(&ds->regs->buf) & SPIBUF_TXFULL_MASK)
		;

	/* preload the TX buffer to avoid clock starvation */
	writel(data1_reg_val, &ds->regs->dat1);

	/* keep reading data until only 1 transfer left */
	while ((len--) > 1)
		if (bitsperxfer <= 8)
			*rxpb++ = davinci_spi_xfer_data(ds, data1_reg_val) & (u8)mask;
		else
			*rxpw++ = davinci_spi_xfer_data(ds, data1_reg_val) & (u16)mask;

	/* clear CS hold when we reach the end */
	if (flags & SPI_XFER_END)
		data1_reg_val &= ~(1 << SPIDAT1_CSHOLD_SHIFT);

	/* read the last value */
		if (bitsperxfer <= 8)
			*rxpb++ = davinci_spi_xfer_data(ds, data1_reg_val) & (u8)mask;
		else
			*rxpw++ = davinci_spi_xfer_data(ds, data1_reg_val) & (u16)mask;

	return 0;
}

static int davinci_spi_write(struct spi_slave *slave, unsigned int len,
			     unsigned int bitsperxfer, const void *txp,
			     unsigned long flags)
{
	struct davinci_spi_slave *ds = to_davinci_spi(slave);
	unsigned int data1_reg_val;
	const u8 *txpb = txp; /* 8 bits / xfer or less */
	const u16 *txpw = txp; /* 9 bits / xfer or more */
	u16 mask = (1 << bitsperxfer) - 1;

	/* enable CS hold and clear the data bits */
	data1_reg_val = ((1 << SPIDAT1_CSHOLD_SHIFT) |
			 (slave->cs << SPIDAT1_CSNR_SHIFT));

	/* wait till TXFULL is deasserted */
	while (readl(&ds->regs->buf) & SPIBUF_TXFULL_MASK)
		;

	/* preload the TX buffer to avoid clock starvation */
	if (len > 2) {
		if (bitsperxfer <= 8)
			writel(data1_reg_val | (*txpb++ & mask), &ds->regs->dat1);
		else {
			writel(data1_reg_val | (swab16(*txpw++) & mask), &ds->regs->dat1);
		}
		len--;
	}

	/* keep sending data until only 1 transfer left */
	while ((len--) > 1)
		if (bitsperxfer <= 8)
			davinci_spi_xfer_data(ds, data1_reg_val | (*txpb++ & mask));
		else {
			davinci_spi_xfer_data(ds, data1_reg_val | (swab16(*txpw++) & mask));
		}

	/* clear CS hold when we reach the end */
	if (flags & SPI_XFER_END)
		data1_reg_val &= ~(1 << SPIDAT1_CSHOLD_SHIFT);

	/* send the last transfer */
	if (bitsperxfer <= 8)
		davinci_spi_xfer_data(ds, data1_reg_val | (*txpb++ & mask));
	else {
		davinci_spi_xfer_data(ds, data1_reg_val | (swab16(*txpw++) & mask));
	}

	return 0;
}

static void hexdump(unsigned char *buf, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if ((i % 16) == 0)
			printf("%s%08x: ", i ? "\n" : "",
							(unsigned int)&buf[i]);
		printf("%02x ", buf[i]);
	}
	printf("\n");
}


#ifndef CONFIG_SPI_HALF_DUPLEX
static int davinci_spi_read_write(struct spi_slave *slave, unsigned int len,
				  unsigned int bitsperxfer, void *rxp, const void *txp,
				  unsigned long flags)
{
	struct davinci_spi_slave *ds = to_davinci_spi(slave);
	int ret = 0;
	unsigned int data1_reg_val;
	u8 *rxpb = rxp; /* 8 bits / xfer or less */
	const u8 *txpb = txp; /* 8 bits / xfer or less */
	u16 *rxpw = rxp; /* 9 bits / xfer or more */
	const u16 *txpw = txp; /* 9 bits / xfer or more */
	u16 mask = (1 << bitsperxfer) - 1;

	writel((readl(&ds->regs->fmt0) & 0xFFFFFFE0) | bitsperxfer, &ds->regs->fmt0);

	/* enable CS hold and clear the data bits */
	data1_reg_val = ((1 << SPIDAT1_CSHOLD_SHIFT) |
			 (slave->cs << SPIDAT1_CSNR_SHIFT));

	/* wait till TXFULL is deasserted */
	while (readl(&ds->regs->buf) & SPIBUF_TXFULL_MASK)
		;

	/* keep reading and writing data until only 1 transfer left */
	while ((len--) > 1)
		if (bitsperxfer <= 8)
			*rxpb++ = davinci_spi_xfer_data(ds, data1_reg_val | (*txpb++ & mask)) & mask;
		else {
			*rxpw++ = davinci_spi_xfer_data(ds, data1_reg_val | (swab16(*txpw++) & mask)) & mask;
		}

	/* clear CS hold when we reach the end */
	if (flags & SPI_XFER_END) {
		data1_reg_val &= ~(1 << SPIDAT1_CSHOLD_SHIFT);
		flags &= ~SPI_XFER_END;
	}

	/* read and write the last byte */
		if (bitsperxfer <= 8)
			*rxpb++ = davinci_spi_xfer_data(ds, data1_reg_val | (*txpb++ & mask)) & mask;
		else {
			*rxpw++ = davinci_spi_xfer_data(ds, data1_reg_val | (swab16(*txpw++) & mask)) & mask;
		}
out:
	if (flags & SPI_XFER_END) {
		u8 dummy = 0;
		davinci_spi_write(slave, 1, 8, &dummy, flags);
	}
	return ret;
}
#endif

int spi_xfer(struct spi_slave *slave, unsigned int bitsperxfer, unsigned int bytelen,
	     const void *dout, void *din, unsigned long flags)
{
	struct davinci_spi_slave *ds = to_davinci_spi(slave);
	unsigned int len;
	int ret = 0;

	if (bytelen == 0)
		/* Finish any previously submitted transfers */
		goto out;

	if ((bitsperxfer > 16) || (bitsperxfer < 2)) {
		printf("Error. bits/transfer=%i out of boundaries (hardware supports 2..16)\n", bitsperxfer);
		/* Errors always terminate an ongoing transfer */
		flags |= SPI_XFER_END;
		ret = -1;
		goto out;
 	}

	if  (bitsperxfer > 8) {
		if (bytelen & 0x1) {
			printf("Error. Mismatch between number of bytes of data provided and bits per transfer\n");
			/* Errors always terminate an ongoing transfer */
			flags |= SPI_XFER_END;
			ret = -1;
			goto out;
		}
		len = bytelen >> 1;
	} else {
		len = bytelen;
	}

	writel((readl(&ds->regs->fmt0) & 0xFFFFFFE0) | bitsperxfer, &ds->regs->fmt0);

	if (!dout)
		return davinci_spi_read(slave, len, bitsperxfer, din, flags);
	else if (!din)
		return davinci_spi_write(slave, len, bitsperxfer, dout, flags);
#ifndef CONFIG_SPI_HALF_DUPLEX
	else
		return davinci_spi_read_write(slave, len, bitsperxfer, din, dout, flags);
#else
	printf("SPI full duplex transaction requested with "
	       "CONFIG_SPI_HALF_DUPLEX defined.\n");
	flags |= SPI_XFER_END;
#endif

out:
	if (flags & SPI_XFER_END) {
		u8 dummy = 0;
		davinci_spi_write(slave, 1, 8, &dummy, flags);
	}
	return ret;
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && cs == 0;
}

void spi_cs_activate(struct spi_slave *slave)
{
	/* do nothing */
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	/* do nothing */
}
