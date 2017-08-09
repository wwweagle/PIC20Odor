/*
 * File:   utils.c
 * Author: Tony Lab
 *
 * Created on August 9, 2017, 2:59 PM
 */


#include "xc.h"
#include "utils.h"
#include "hal.h"
#include "lcdi2c.h"

int getFuncNumber(int targetDigits, const char* message) {
    int bitValue[targetDigits];
    int n = 0;
    int iter;
    int iter1;

    LCDclear();
    LCDhome();
    LCD_Write_Str(message);


    for (iter = 0; iter < targetDigits; iter++) {
        bitValue[iter] = -6;
    }

    for (iter = 0; iter < targetDigits; iter++) {
        int lcdPos;
        for (lcdPos = 0; lcdPos < targetDigits; lcdPos++) {
            LCDsetCursor(lcdPos, 1);
            LCD_Write_Char(bitValue[lcdPos] + 0x30);
        }

        while (bitValue[iter] < 0) {
            if (u2Received > 0) {
                bitValue[iter] = u2Received - 0x30;
                u2Received=-1;
            }
        }
        u2send(1, bitValue[iter]);

    }
    for (iter1 = 0; iter1 < targetDigits; iter1++) {
        n = n * 10 + bitValue[iter1];
    }
    return n;
}