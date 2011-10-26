/*
 * cmd_pinmux.c
 *
 * Expose pin_mux() API as a u-boot command.
 *
 * Original Author:
 *     Todd Fischer <todd.fischer@ridgerun.com>
 *
 * Copyright (C) 2011 RidgeRun - http://www.ridgerun.com/
 *
 */

#include <common.h>
#include <command.h>

extern int pinmux_info(void);

/*********************************************************
 Routine: do_pin_mux
 Description:  U-boot command for pin multiplexing
*********************************************************/
int do_pin_mux(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if ((strcmp(argv[1], "info") == 0) && (argc == 2))
		return pinmux_info();

	return cmd_usage(cmdtp);
}

/***************************************************/

U_BOOT_CMD(
	pinmux, 2, 1, do_pin_mux,
	"SoC pin multiplexing",
	"pinmux info"
		" - Display pin multiplexing information\n"
);

