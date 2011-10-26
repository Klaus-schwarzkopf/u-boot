/*
 * General Purpose Input Output commands
 *
 *  Copyright (C) 2011 RidgeRun
 *  Todd Fischer <todd.fischer@ridgerun.com>
 *
 * Copyright 2008-2009 Stefan Roese <sr@denx.de>, DENX Software Engineering
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <common.h>
#include <command.h>
#include <exports.h>

#include <gpio.h>

static int do_gpio(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int value = 0;
	int err = 0;
	int gpio_pin = 0;

	if (argc < 3)
		return cmd_usage(cmdtp);

	gpio_pin = simple_strtoul(argv[2], NULL, 10);

	if ((strcmp(argv[1], "get") == 0) && (argc == 3)) {
		err = gpio_get(gpio_pin, &value);
		if (err) {
			printf("GPIO ERROR -- unknown pin: %d\n", gpio_pin);
			return -1;
		}

		printf("%d", value ? 1 : 0);
		return 0;
	}

	if ((strcmp(argv[1], "dir") == 0) && (argc == 4)) {

		if (strcmp(argv[3], "in") == 0) {
			err = gpio_dir(gpio_pin, GPIO_DIRECTION_IN);
			if (err) {
				printf("GPIO ERROR -- unknown pin: %d\n", gpio_pin);
				return -1;
			}

			return 0;
		}

		if (strcmp(argv[3], "out") == 0) {
			err = gpio_dir(gpio_pin, GPIO_DIRECTION_OUT);
			if (err) {
				printf("GPIO ERROR -- unknown pin: %d\n", gpio_pin);
				return -1;
			}

			return 0;
		}

		printf("GPIO ERROR -- unknown direction (use in or out): %s\n", argv[3]);
		return -1;
	}

	if ((strcmp(argv[1], "set") == 0) && (argc == 4)) {

		if (strcmp(argv[3], "0") == 0) {
			err = gpio_set(gpio_pin, 0);
			if (err) {
				printf("GPIO ERROR -- unknown pin: %d\n", gpio_pin);
				return -1;
			}

			return 0;
		}

		if (strcmp(argv[3], "1") == 0) {
			err = gpio_set(gpio_pin, 1);
			if (err) {
				printf("GPIO ERROR -- unknown pin: %d\n", gpio_pin);
				return -1;
			}

			return 0;
		}

		printf("GPIO ERROR -- unknown value (use 0 or 1): %s\n", argv[3]);
		return -1;
	}

	cmd_usage(cmdtp);
	return -1;
}

U_BOOT_CMD(
	gpio, 4, 1, do_gpio,
	"gpio commands",
	"gpio dir gpio_pin in|out"
		" - Configure a GPIO for input or output\n"
	"gpio set gpio_pin 0|1"
		" - Set GPIO output low or high\n"
	"gpio get gpio_pin"
		" - Get GPIO level\n"
);
