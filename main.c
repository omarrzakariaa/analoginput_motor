/*
File: main.c
Author: ozakaria
Main file to control analog input and encoder on Explorer 16/32 board
Due on: October 13th, 2023
*/
#include <xc.h>
#include "newxc32_header.h" // include header file
#include <stdio.h> // allows strings
#include <sys/attribs.h> // allows interrups
#include <string.h>
#pragma config FPLLMUL = MUL_20, FPLLIDIV = DIV_2, FPLLODIV = DIV_1, FWDTEN = OFF 
#pragma config POSCMOD = HS, FNOSC = PRIPLL, FPBDIV = DIV_1 // set peripheral clock to 80MHz

char msgpot[20] = {}; // array of strings for potentiometer message on lcd
char msgcount[20] = {}; // array of strings for count of wheel message on lcd
char msgtemp[20] = {}; // array of strings for temperature message on lcd
char msgangle[20] = {}; // array of strings for angle message on lcd
static volatile int count; // keep count while adapting to unexpected changes
int oldbit1; // past bit 1
int oldbit2; // past bit 2
int newbit1; // present bit 1
int newbit2; // present bit 2
int motorticks = 99; // number of ticks on wheel
int encoderticks = 48; // 48 counts per revolution
int buttonPressed = 0; // check is push button S4 got pressed

void __ISR(_CHANGE_NOTICE_VECTOR, IPL5SOFT) CNISR(void) { // CN interrupt

    
    if(PORTDbits.RD13 == 0) { // loop reads if S4 got pressed
        buttonPressed = 1; // if pressed, buttonPressed is set to 1
        IFS1bits.CNIF = 0; // clear flag in case
    }
    
    newbit1 = PORTGbits.RG6; // read and save the change of encoder for bit1
    newbit2 = PORTGbits.RG7; // read and save the change of encoder for bit1
    
    // STATE MACHINE that compares old state to present state!!
    // checks wheather to increment or decrement depending on old state and new state
    if (oldbit1 == 0 && oldbit2 == 0) { 
        if (newbit1 == 1 && newbit2 == 0) {
            count--; // decrement count
        } else if (newbit2 == 1 && newbit1 == 0) {
            count++; // increment count
        }
    } 
    else if (oldbit1 == 0 && oldbit2 == 1){    
        if (newbit1 == 1 && newbit2 == 1) {
            count++; // increment count
        } else if (newbit2 == 0 && newbit1 == 0) {
            count--; // decrement count
        }
    } 
    else if (oldbit1 == 1 && oldbit2 == 0) {
        if (newbit1 == 0 && newbit2 == 0) {
            count++; // increment count
        } else if (newbit2 == 1 && newbit1 == 1) {
            count--; // decrement count
        }
    } 
    else if(oldbit1 == 1 && oldbit2 == 1){
        if (newbit1 == 0 && newbit2 == 1) {
            count--; // decrement count
        } else if (newbit2 == 0 && newbit1 == 1) {
            count++; // increment count
        }
    }

    oldbit1 = newbit1; // old state changes to new state
    oldbit2 = newbit2; // old state changes to new state
    
    IFS1bits.CNIF = 0; // Clear flag
}


main(void){
    INTCONbits.MVEC = 1; // set multivector mode
    
    // set tristate special register for different pins
    TRISDbits.TRISD4 = 0; 
    TRISDbits.TRISD5 = 0; 
    TRISBbits.TRISB15 = 0; 
    TRISGbits.TRISG6 = 1; 
    TRISGbits.TRISG7 = 1; 
    TRISE = 0xff00; 
    TRISD = 0xFFCF;
    
    AD1PCFGbits.PCFG2 = 0; // AN2 is an adc pin
    AD1PCFGbits.PCFG4 = 0; // AN4 is an adc pin
    AD1CON1bits.ADON = 1; // turn on A/D converter
    
    
    __builtin_disable_interrupts();
    
    CNCONbits.ON = 1;  // enable change notification
    IPC6bits.CNIP = 5; // set priority
    IFS1bits.CNIF = 0; // clear flag
    IEC1bits.CNIE = 1; // enable interrupt
    CNENbits.CNEN19 = 1; // enable change notif for push button s4
    CNENbits.CNEN9 = 1; // enable change notif for motor spins
    CNENbits.CNEN8 = 1; // enable change notif for motor spins
    
    __builtin_enable_interrupts();
    
    
    lcd_display_driver_initialize(); 
    lcd_display_driver_clear(); 
    
    
    count = 0; // initialize count
    float productticks = motorticks * encoderticks; // 99 times 48
    oldbit1 = PORTGbits.RG6; // set initial oldbits to current PORT value
    oldbit2 = PORTGbits.RG7; // set initial oldbits to current PORT value
   while(1){
    if (buttonPressed == 0){ // before the button is pressed this loop runs
        unsigned int potRead = read_potentiometer(); // call function and save value
        unsigned int tempRead = read_temp(); // call function and save value
        sprintf(msgpot, "Pot: %4u", potRead); // turn to string
        sprintf(msgtemp, "Temp: %4u", tempRead); // turn to string
   
        display_driver_use_first_line(); 
        lcd_display_driver_write(msgpot, 9); // write potentiometer value to first line
        display_driver_use_second_line(); 
        lcd_display_driver_write(msgtemp, 10); // write temperature value to second line
        }
    else if (buttonPressed == 1){ // after the button is pressed this loop runs
        float angle = count * 360/productticks; // calculate the angle at the end of each tick
        sprintf(msgangle, "Angle : %.2f\0xDF", angle); // \0xDF to display angle sign
        sprintf(msgcount, "Count : %d", count); // turn to string
        
        display_driver_use_first_line();
        lcd_display_driver_write(msgangle, strlen(msgangle)); // write angle value to first line
        display_driver_use_second_line();
        lcd_display_driver_write(msgcount, strlen(msgcount)); // write count value to second line
        }
    }
    
    return 0;
}