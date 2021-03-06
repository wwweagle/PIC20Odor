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



#define _EE_WORD         2
#define _memcpy_p2d16(dest, src, len)  _memcpy_helper(src, dest, len, 0) 
#define _wait_eedata() { while (NVMCONbits.WR); }
#define _erase_eedata  _eedata_helper1
#define _write_eedata_word  _eedata_helper3


#define EEP_LICK_THRESHOLD_L 0
#define EEP_WATER_LEN_MS_L 2
#define EEP_LICK_THRESHOLD_R 10
#define EEP_WATER_LEN_MS_R 12




typedef unsigned long _prog_addressT;

extern void _eedata_helper1(_prog_addressT dst, int len);
extern void _eedata_helper3(_prog_addressT dst, int dat);
extern _prog_addressT _memcpy_helper(_prog_addressT src, void *dst,
        unsigned int len, int flags);
extern unsigned char LCD_PCF8574_ADDR;  // Modify this if the default address is altered 


extern int lickThreshL;
extern int lickThreshR;

extern int u2Received;
extern volatile int adcdataL;
extern volatile int adcdataR;
extern volatile uint32_t timerCounterI, millisCounter,taskTimeCounter,delayOnsetTS;


void initPorts(void);
void initTMR1(void);
void wait_ms(int time);
void wait_Sec(int time);
void serialSend(int u2Type, int u2Value);
void initUART2(void);
void set4076_4bit(int val);
void muxOff(int val);
void initADC(void);


void write_eeprom_G2(int offset, int value) ;

int read_eeprom_G2(int offset) ;
int checkKeyboard();
unsigned char getLCDAddr();

#endif	//HAL_H

