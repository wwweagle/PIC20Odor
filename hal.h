#ifndef HAL_H
#define	HAL_H
#include <xc.h> // include processor files - each processor file is guarded.  
#include <timer.h>
#include <uart.h>
#include <stdint.h>

#define FCY  16000000   //8M XT * PLL_8, Used to be 8000000 with XT_PLL4
#define BAUDRATE 9600



//#define PWM1LE1 PORTGbits.RG0
//#define PWM2LE2 PORTGbits.RG1
//#define PWM3LE1 PORTGbits.RG6
//#define PWM4LE2 PORTGbits.RG7
//#define PWM2LE1 PORTFbits.RF0
//#define PWM1LE2 PORTFbits.RF1
//#define PWM4LE1 PORTCbits.RC1
//#define PWM3LE2 PORTCbits.RC3


//#define P10_2 PORTGbits.RG0
//#define P10_4 PORTGbits.RG1
//#define P10_6 PORTGbits.RG6
//#define P10_8 PORTGbits.RG7
#define P10_1 PORTEbits.RE0  //1L
#define P10_3 PORTEbits.RE2  //2L
#define P10_5 PORTEbits.RE4  //3L
#define P10_7 PORTEbits.RE6  //4L




void initPorts(void);
void initTMR1(void);
void wait_ms(unsigned int time);
void u2send(int u2Type, int u2Value);
void initUART2(void);
void setP10_4bit(int val);
void setP10_dis(int val);

#endif	//HAL_H

