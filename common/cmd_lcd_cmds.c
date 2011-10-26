/*
 * cmd_lcd_cmds.c
 *
 * This file defines the basic functions to set commands to an LCD controller
 * deoending on its driver
  *
 * Original Author:
 *     Pablo Barrantes <pablo.barrantes@ridgerun.com>
 *	Natanel Castro	 <natanael.castro@ridgerun.com>
 *
 * Copyright (C) 2011 RidgeRun - http://www.ridgerun.com/
 *
 */

#include <common.h>
#include <command.h>
#include <lcd_commands.h>

/*********************************************************
 Routine: do_lcd_reg_get
 Description:  U-boot command for reading
					a LCD register
*********************************************************/
int do_lcd_reg_get (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
#ifdef LCD_DRIVER_PRESENT
	if (argc >= 2)
		return lcd_drv_reg_get(argv);

	return 1;
#else
	printf("No driver for handling lcd present\n");
	return 1;
#endif
}

/*********************************************************
 Routine: do_lcd_reg_set
 Description:  U-boot command for writing
					a LCD register
*********************************************************/
int do_lcd_reg_set (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
#ifdef LCD_DRIVER_PRESENT
	if (argc >= 3)
		return lcd_drv_reg_set(argc, argv);

	return 1;
#else
	printf("No driver for handling lcd present\n");
	return 1;
#endif
}

/*********************************************************
 Routine: do_lcd_init
 Description:  U-boot command for initializing LCD
*********************************************************/
int do_lcd_init(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
#ifdef LCD_DRIVER_PRESENT

	int lcd_reset_gpio = -1;

	if (argc >= 2) {
		lcd_reset_gpio = simple_strtoul(argv[1], NULL, 10);
	}

	printf("%s: lcd reset gpio: %d\n", __FUNCTION__, lcd_reset_gpio);

	return lcd_drv_init(lcd_reset_gpio);

#else
	printf("No driver for handling lcd present\n");
	return 1;
#endif
}

/*********************************************************
 Routine: do_lcd_awake
 Description:  U-boot command for switching
				LCD to its awake mode
*********************************************************/
int do_lcd_awake(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
#ifdef LCD_DRIVER_PRESENT
	return lcd_drv_awake();
#else
	printf("No driver for handling lcd present\n");
	return 1;
#endif
}

/*********************************************************
 Routine: do_lcd_sleep
 Description:  U-boot command for switching
				LCD to its sleep mode
*********************************************************/
int do_lcd_sleep(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
#ifdef LCD_DRIVER_PRESENT
	return lcd_drv_sleep();
#else
	printf("No driver for handling lcd present\n");
	return 1;
#endif
}

/***************************************************/

U_BOOT_CMD(
	lcdinit, 2, 1, do_lcd_init,
	"Initializes LCD display",
	"lcdinit [gpio reset pin number]\n"
);

U_BOOT_CMD(
	lcdsleep,	1,	1,	do_lcd_sleep,
	"Switch LCD to its sleep mode",
	"lcdsleep \n"
);

U_BOOT_CMD(
	lcdawake,	1,	1,	do_lcd_awake,
	"Switch LCD to its awake mode",
	"lcdwake \n"
);

U_BOOT_CMD(
	lcdregget, 2,	1,	do_lcd_reg_get,
	"reads LCD controller's register",
	"<reg_number> - Read <reg_number> value from LCD controller\n"
	"<reg_number> - Hexadecimal string with the register number (requires a 2 figures number)\n"
);

U_BOOT_CMD(
	lcdregset,	3,	1,	do_lcd_reg_set,
	"writes LCD controller's register",
	"<reg_number> <data> - Write <data> to <reg_number> on LCD controller\n"
	"<reg_number> - Hexadecimal string with the register number (requires a 2 figures number)\n"
	"<data>     - Hexadecimal string that gets sent"
);
