/*
File: newxc32_header.h
Author: ozakaria
Header file for functions used to control analog input and encoder on Explorer 16/32 board
Due on: October 13th, 2023
*/

void lcd_display_driver_enable();
void lcd_display_driver_initialize();
void lcd_display_driver_clear();
void lcd_display_driver_write(char* data, int length);
void display_driver_use_first_line();
void display_driver_use_second_line();
int read_potentiometer();
int read_temp();