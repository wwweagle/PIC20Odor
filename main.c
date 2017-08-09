/*
 * File:   main.c
 * Author: Tony Lab
 *
 * Created on August 7, 2017, 9:47 AM
 */

#include <i2c.h>

#include "hwConfig.h"
#include "utils.h"
#include "hal.h"
#include "lcdi2c.h"


void callFunc(int n);
void testAllValves(void);
void oneValve(int n);

int main(void) {
    initPorts();
    initTMR1();
    initUART2();
    initI2C();
    LCD_Init();
    callFunc(getFuncNumber(2, "Main Func?"));

    StopI2C();
    CloseI2C();
    return 0;
}

void callFunc(int n) {
    switch (n) {
        case 99:
            testAllValves();
            break;
        default:
            oneValve(n);
            break;

    }
}

void oneValve(int n) {
    LCDclear();
    LCDhome();
    LCD_Write_Num(n, 0, 0);
    set4076_4bit(n > 15 ? n - 16 : n);
    while (1) {
        muxDis(n < 16 ? (~1) : (~4));
        wait_Sec(1);
        muxDis(n < 16 ? (~2) : (~8));
        wait_Sec(1);
        muxDis(0x0f);
        wait_Sec(1);
    }
}

void testAllValves() {
    int preCharge = 2;

    int repeat = 10;
    int iti = 5;
    const int onTime = 1000;
    int valve;
    int rpt;
    for (rpt = 0; rpt < repeat; rpt++) {
        LCDclear();
        LCDhome();
        LCD_Write_Str("Valve");
        LCDsetCursor(0, 1);
        LCD_Write_Str("Repeat");
        LCD_Write_Num(rpt + 1, 7, 1);
        u2send(3, rpt + 1);
        for (valve = 0; valve < 20; valve++) {
            LCD_Write_Num(valve + 1, 6, 0);
            u2send(2, valve + 1);
            int P10Val = valve < 16 ? valve : valve - 16;
            int boardA = valve < 16 ? 1 : 4;
            int boardB = valve < 16 ? 2 : 8;
            set4076_4bit(P10Val);
            muxDis(~boardA);
            wait_Sec(preCharge);
            BNC_1 = 1;
            muxDis(~(boardA | boardB));
            BNC_2 = (valve & 0x10) >> 4;
            wait_ms(onTime / 5);
            BNC_2 = (valve & 0x08) >> 3;
            wait_ms(onTime / 5);
            BNC_2 = (valve & 0x04) >> 2;
            wait_ms(onTime / 5);
            BNC_2 = (valve & 0x02) >> 1;
            wait_ms(onTime / 5);
            BNC_2 = (valve & 0x01);
            wait_ms(onTime / 5);
            BNC_2 = 0;
            muxDis(0x0F);
            BNC_1 = 0;
            wait_Sec(iti - preCharge);

        }
    }
}



//
////void loopLCD() {
////    LCD_Init();
////    LCDclear();
////    LCDcursorOff();
////    LCDblinkOff();
////    int i;
////    while (1)
////        for (i = 0; i < 10; i++) {
////            LCDsetCursor(0, 0);
////            LCD_Write_Char('0' + i);
////            wait_ms(1000);
////        }
////
////}
////
////void loopPorts() {
////    int port, offPort;
////    while (1) {
////        for (port = 8; port > 0; port--) {
////            u2send(1, port);
////            setP10(port, 1);
////            for (offPort = 8; offPort > 0; offPort--) {
////                if (offPort != port) {
////                    setP10(offPort, 0);
////                }
////            }
////            wait_ms(1000);
////        }
////    }
////}
//
//void loopPorts4076() {
//    int i, j;
//    for (j = 0; j < 16; j++) {
//        setP10_dis(j);
//        LCD_Write_Num(j, 0, 1);
//        for (i = 0; i < 16; i++) {
//            setP10_4bit(i);
//            LCD_Write_Num(i, 0, 0);
//            wait_ms(500);
//        }
//    }
//}
//
//void loopPortsIIC() {
//    while (1) {
//        PORTGbits.RG2 = 1;
//        Nop();
//        Nop();
//        PORTGbits.RG3 = 1;
//        Nop();
//        Nop();
//        wait_ms(500);
//        PORTGbits.RG2 = 0;
//        Nop();
//        Nop();
//        PORTGbits.RG3 = 0;
//        Nop();
//        Nop();
//        wait_ms(500);
//
//    }


