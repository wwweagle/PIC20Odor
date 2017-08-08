/*
 * File:   main.c
 * Author: Tony Lab
 *
 * Created on August 7, 2017, 9:47 AM
 */

#include <i2c.h>

#include "hwConfig.h"
#include "hal.h"
#include "lcdi2c.h"


void loopPorts4076(void);
void loopPortsIIC(void);
void loopLCD();

int main(void) {
    initPorts();
    initTMR1();
    initUART2();
    initI2C();
    LCD_Init();
    loopPorts4076();

    StopI2C();
    CloseI2C();
    return 0;
}

//void loopLCD() {
//    LCD_Init();
//    LCDclear();
//    LCDcursorOff();
//    LCDblinkOff();
//    int i;
//    while (1)
//        for (i = 0; i < 10; i++) {
//            LCDsetCursor(0, 0);
//            LCD_Write_Char('0' + i);
//            wait_ms(1000);
//        }
//
//}
//
//void loopPorts() {
//    int port, offPort;
//    while (1) {
//        for (port = 8; port > 0; port--) {
//            u2send(1, port);
//            setP10(port, 1);
//            for (offPort = 8; offPort > 0; offPort--) {
//                if (offPort != port) {
//                    setP10(offPort, 0);
//                }
//            }
//            wait_ms(1000);
//        }
//    }
//}

void loopPorts4076() {
    int i, j;
    for (j = 0; j < 16; j++) {
        setP10_dis(j);
        LCD_Write_Num(j, 0, 1);
        for (i = 0; i < 16; i++) {
            setP10_4bit(i);
            LCD_Write_Num(i, 0, 0);
            wait_ms(500);
        }
    }
}

void loopPortsIIC() {
    while (1) {
        PORTGbits.RG2 = 1;
        Nop();
        Nop();
        PORTGbits.RG3 = 1;
        Nop();
        Nop();
        wait_ms(500);
        PORTGbits.RG2 = 0;
        Nop();
        Nop();
        PORTGbits.RG3 = 0;
        Nop();
        Nop();
        wait_ms(500);

    }


}