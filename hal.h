#ifndef HAL_H
#define	HAL_H
#include <xc.h> // include processor files - each processor file is guarded.  
#include <timer.h>
#include <uart.h>
#include <stdint.h>

#define FCY  8000000   //8M XT * PLL_4
#define BAUDRATE 19200



#define BNC_1 PORTGbits.RG8
#define BNC_2 PORTGbits.RG9
#define BNC_3 PORTEbits.RE8
#define BNC_4 PORTEbits.RE9
#define BNC_5 PORTAbits.RA9
#define BNC_6 PORTAbits.RA10


extern int u2Received;
extern volatile int adcdata;

void initPorts(void);
void initTMR1(void);
void wait_ms(int time);
void wait_Sec(int time);
void u2send(int u2Type, int u2Value);
void initUART2(void);
void set4076_4bit(int val);
void muxDis(int val);
void ADCinit(void);

#endif	//HAL_H

