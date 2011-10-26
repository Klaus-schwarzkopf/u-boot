/*
 *
 * (c) 20100 Ridgerun, Todd Fischer <todd.fischer@ridgerun.com>
 *
 * Based on MXC specific GPIO work by Sascha Hauer
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
#include <gpio.h>
#include <asm/io.h>
#include <asm/types.h>

#define DAVINCI_GPIO_BASE (void *)(0x01C67000)

#ifdef CONFIG_DAVINCI_GPIO_DEBUG
#define GPIO_DBG(fmt,args...)	printf (fmt ,##args)
#else
#define GPIO_DBG(fmt,args...) do {} while (0)
#endif

struct gpio_controller {
	u32	dir;
	u32	out_data;
	u32	set_data;
	u32	clr_data;
	u32	in_data;
	u32	set_rising;
	u32	clr_rising;
	u32	set_falling;
	u32	clr_falling;
	u32	intstat;
};

static struct gpio_controller *gpio_to_controller(unsigned gpio)
{
	void *ptr;
	void *base = DAVINCI_GPIO_BASE;

	if (gpio < 32 * 1)
		ptr = base + 0x10;
	else if (gpio < 32 * 2)
		ptr = base + 0x38;
	else if (gpio < 32 * 3)
		ptr = base + 0x60;
	else if (gpio < 32 * 4)
		ptr = base + 0x88;
	else if (gpio < 32 * 5)
		ptr = base + 0xb0;
	else
		ptr = NULL;
	return ptr;
}

static inline u32 gpio_mask(unsigned gpio)
{
	return 1 << (gpio % 32);
}

static inline u32 gpio_reg_read(void *reg)
{
	u32 val = __raw_readl(reg);

	GPIO_DBG("%s [0x%p] --> 0x%8.8X\n", __FUNCTION__, reg, val);

	return val;
}

static inline void gpio_reg_write(u32 val, void *reg)
{

	GPIO_DBG("%s [0x%p] <-- 0x%8.8X\n", __FUNCTION__, reg, val);

	__raw_writel(val, reg);
}


int gpio_dir(unsigned int gpio, enum gpio_direction direction)
{
	struct gpio_controller	*g;
	u32			mask;
	u32			temp;

	GPIO_DBG("%s ENTRY: gpio %d direction %d\n", __FUNCTION__, gpio, direction);

	g = gpio_to_controller(gpio);

	if (!g) {
		return -1;
	}

	mask = gpio_mask(gpio);

	temp = gpio_reg_read(&g->dir);

	if (direction == GPIO_DIRECTION_IN)
		temp |= mask;
	else
		temp &= ~mask;

	gpio_reg_write(temp, &g->dir);
	return 0;
}

/* Set the value, then set the direction to out
 * to avoid an unintended glitch on the GPIO pin.
 */

int gpio_set(unsigned int gpio, unsigned int value)
{
	struct gpio_controller	*g;
	u32			mask;

	GPIO_DBG("%s ENTRY: gpio %d value %d\n", __FUNCTION__, gpio, value);

	g = gpio_to_controller(gpio);
	if (!g) {
		return -1;
	}

	mask = gpio_mask(gpio);

	if (value)
		gpio_reg_write(mask, &g->set_data);
	else
		gpio_reg_write(mask, &g->clr_data);

	return 0;
}

int gpio_get(unsigned int gpio, int *value)
{
	struct gpio_controller	*g;
	u32			mask;

	GPIO_DBG("%s ENTRY: gpio %d\n", __FUNCTION__, gpio);

	g = gpio_to_controller(gpio);
	if (!g) {
		return -1;
	}

	mask = gpio_mask(gpio);

	if (gpio_reg_read(&g->in_data) & mask)
		*value = 1;
	else
		*value = 0;

	return 0;
}
