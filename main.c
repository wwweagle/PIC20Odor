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
void testOneValve(int n);
void oneValve(int n);
void testValveFast(int board, int valve, int keep);
void testValveOnRA14();
void readADCData();
void testPorts();

int main(void) {
    initPorts();
    initTMR1();
    initUART2();
    initI2C();
    LCD_Init();
    ADCinit();
    callFunc(getFuncNumber(2, "Main Func?"));

    StopI2C();
    CloseI2C();
    return 0;
}

void temp(){

    while(1){
        PORTBbits.RB8=1;
    wait_ms(250);
        PORTBbits.RB9=1;
    wait_ms(250);
        PORTBbits.RB8=0;
    wait_ms(250);
        PORTBbits.RB9=0;
    wait_ms(250);
    }
}

void callFunc(int n) {
    switch (n) {
        case 20:
            testValveOnRA14();
            break;
        case 21:
        {
            int b = getFuncNumber(1, "Board?");
            int v = getFuncNumber(2, "Valve?");
            int k = getFuncNumber(1, "Keep?");
            testValveFast(b, v, k);
        }
        case 22:
            readADCData();
            break;
        case 23:
            testPorts();
            break;
        case 24:
            temp();
            break;
        default:
        {
            int i;
            for (i = n; i < n + 4; i++)
                testOneValve(i);
        }
            break;

    }
}

void testValveOnRA14() {
    uint8_t isOn = 0;
    LCDclear();
    LCDhome();
    while (1) {
        PORTAbits.RA14 = isOn;
        isOn = isOn ^ 0x01;
        LCD_Write_Num(isOn, 0, 0);
        wait_ms(500);
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

void testValveFast(int board, int valve, int keep) {
    LCDclear();
    LCDhome();
    LCD_Write_Str("BOARD   VALVE");

    LCD_Write_Num(board, 6, 0);
    LCD_Write_Num(valve, 6, 14);

    set4076_4bit(valve - 1);

    while (1) {
        muxDis(~board);
        if (!keep) {
            wait_ms(500);
            muxDis(0x0f);
            wait_ms(500);
        }

    }

}

void testOneValve(int valve) {
    const int preCharge = 500;

    int repeat = 10;
    int iti = 11;
    const int onTime = 1000;
    int closingAdvance=195;
    //    int valve;
    int rpt;
    for (rpt = 0; rpt < repeat; rpt++) {
        LCDclear();
        LCDhome();
        LCD_Write_Str("Valve");
        LCDsetCursor(0, 1);
        LCD_Write_Str("Repeat");
        LCD_Write_Num(rpt + 1, 7, 1);
        u2send(3, rpt + 1);
        //        for (valve = 0; valve < 20; valve++) {
        LCD_Write_Num(valve + 1, 6, 0);
        u2send(2, valve + 1);
        int P10Val = valve < 16 ? valve : valve - 16;
        int boardA = valve < 16 ? 1 : 4;
        int boardB = valve < 16 ? 2 : 8;
        set4076_4bit(P10Val);
        muxDis(~boardA);
        wait_ms(preCharge);
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
        wait_ms(onTime / 5 - closingAdvance);
        muxDis(~boardB);
        wait_ms(closingAdvance);
        BNC_2 = 0;
        muxDis(0x0F);
        BNC_1 = 0;
        wait_ms(iti * 1000 - preCharge);

        //        }
    }
}

void readADCData(void){
    while(1){
        volatile int temp=adcdata;
        int highByte=temp/100;
        int lowByte=temp%100;
        
        u2send(23, highByte);
        u2send(24, lowByte);
        wait_ms(50);
        
    }
}

void testPorts(){
    TRISF = 0xFF3F;
    int i=0;
    while(1){
        i=i^1;
        PORTFbits.RF6=i;
        Nop();
        Nop();
        Nop();
        PORTBbits.RB8=i;
        Nop();
        Nop();
        Nop();
        PORTBbits.RB9=i;
        Nop();
        Nop();
        Nop();

        PORTFbits.RF7=i;
        Nop();
        Nop();
        Nop();
        PORTAbits.RA14=i;
        Nop();
        Nop();
        Nop();
        PORTDbits.RD12=i;
        Nop();
        Nop();
        Nop();
        PORTDbits.RD13=i;
//        Nop();
//        Nop();
//        Nop();
//        PORTAbits.RA15=i;
        wait_ms(250);
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


