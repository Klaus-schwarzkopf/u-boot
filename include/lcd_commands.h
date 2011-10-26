/*
 * lcd_commands.h
 *
 * This file defines the basic functions, used by the command, that must
 * be included in every lcd_driver to enable it's functionallity.
 *
 * Original Author:
 *     Natanael Castro <natanael.castro@ridgerun.com>
 *
 * Copyright (C) 2011 RidgeRun - http://www.ridgerun.com/
 *
 */

/*******************************************
 Routine: lcd_drv_reg_get
 Description:  routine for reading the values of a LCD controller's register
*******************************************/
int lcd_drv_reg_get(char *argv[]);

/*******************************************
 Routine: lcd_drv_reg_set
 Description:  routine for setting the values of a LCD controller's register
*******************************************/
int lcd_drv_reg_set(int argc, char *argv[]);

/*******************************************
 Routine: lcd_drv_init
 Description:  routine for initializing the LCD, requires its GPIO reset line
*******************************************/
int lcd_drv_init (int lcd_reset_gpio);

/*******************************************
 Routine: lcd_drv_awake
 Description:  routine for setting the LCD to its "awake" mode
*******************************************/
int lcd_drv_awake(void);

/*******************************************
 Routine: lcd_drv_sleep
 Description:  routine for setting the LCD to its "sleep" mode
*******************************************/
int lcd_drv_sleep(void);
