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
#include <stdlib.h>



LICK_T_G2 lick_G2 = {
    .current = 0u, .stable = 0u, .filter = 0u, .lickSide = 0u,
    .LCount = 0u, .RCount = 0u, .refreshLickReading = 0
};

LASER_T_G2 laser_G2 = {.laserSessionType = LASER_NO_TRIAL, .laserTrialType = LASER_OFF,
    .timer = 0u, .onTime = 65535u, .offTime = 0u, .on = 0, .side = 1}; //1L,2R,3LR

TASK_T taskParam = {
    .outSamples = NULL,
    .outTests = NULL,
    .outTaskPairs = 0,
    .innerSamples = NULL,
    .innerTests = NULL,
    .innerTaskPairs = 0,
    .respCue = NULL,
    .respCount = 0,
    .sample1Length = 1000,
    .sample2Length = 1000,
    .test1Length = 1000,
    .test2Length = 1000,
    .respCueLength = 1000,
    .falsePunish = 0,
    .correctionCue = 0,
    .correctionCueLength = 1000,
    .outDelay = 5,
    //    .delay2 = 1500,
    //    .delay3 = 0,
    .ITI = 8,
    .waitForTrial = 1,
    .minBlock = 4,
    .teaching = 0
};

int waterLenL = 50;
int waterLenR = 50;
int hit, miss, falseAlarm, correctRejection, abortTrial;

int currentMiss, correctRatio;

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
            int kb = checkKeyboard();
            if (kb > 0) {
                serialSend(1, 100 + kb);
                return 100 + kb;
            }

            if (u2Received > 0) {
                bitValue[iter] = u2Received - 0x30;
                u2Received = -1;
            }
        }
        serialSend(1, bitValue[iter]);

    }

    int lcdPos;
    for (lcdPos = 0; lcdPos < targetDigits; lcdPos++) {
        LCDsetCursor(lcdPos, 1);
        LCD_Write_Char(bitValue[lcdPos] + 0x30);
    }


    for (iter1 = 0; iter1 < targetDigits; iter1++) {
        n = n * 10 + bitValue[iter1];
    }
    return n;
}

void setWaterPortOpen(int side, int i) {
    if (side == LICKING_LEFT) {
        PORTAbits.RA14 = i;
        PORTDbits.RD4 = i;
    } else if (side == LICKING_RIGHT) {
        PORTAbits.RA15 = i;
        PORTDbits.RD5 = i;
    }
}

void sendLargeValue(int val) {
    char valHigh = (char) (val / 100);
    char valLow = (char) (val % 100);
    serialSend(23, valHigh);
    serialSend(24, valLow);
}

void sendChart(int val, int idx) {
    int high = ((val & 0x0fc0) >> 6)+(idx == 1 ? 0x40 : 0);
    serialSend(SpChartHigh, high);
    int low = (val & 0x3f)+(idx == 1 ? 0x40 : 0);
    serialSend(SpChartLow, low);
}

void shuffleArray_G2(unsigned int * orgArray, unsigned int arraySize) {
    if (arraySize == 0 || arraySize == 1)
        return;

    int iter;
    for (iter = 0; iter < arraySize; iter++) {
        orgArray[iter] = iter;
    }
    int index, temp;
    for (iter = arraySize - 1; iter > 0; iter--) {

        index = rand() % (iter + 1);
        temp = orgArray[index];
        orgArray[index] = orgArray[iter];
        orgArray[iter] = temp;
    }
}

int waitTaskTimer(unsigned int dTime) {
    int currLickL = lick_G2.LCount;
    int currLickR = lick_G2.RCount;
    taskTimeCounter += dTime;
    while (millisCounter < taskTimeCounter);

    return (lick_G2.LCount > currLickL || lick_G2.RCount > currLickR);
}

char assertLaser() {
    char laserTarget = laser_G2.on;
    uint32_t delayLen = (uint32_t) taskParam.outDelay * 1000u;

    if (millisCounter >= delayOnsetTS) {
        uint32_t clockTS = millisCounter - delayOnsetTS;
        switch (laser_G2.laserTrialType) {
            case LASER_TEST:
                return -1;
                break;
            case LASER_OFF:
                laserTarget = 0;
                break;
            case LASERT3SEARLY:
                laserTarget = clockTS >= 1000u && clockTS < 4000u;
                break;
            case LASERT3SMID:
                laserTarget = (clockTS >= ((delayLen >> 1) - 1500u))
                        &&(clockTS < ((delayLen >> 1) + 1500u));
                break;
            case LASERT3SLATE:
                laserTarget = (clockTS >= delayLen - 4000u)
                        && (clockTS < delayLen - 1000u);
                break;
            case LASERT6SEARLY:
                laserTarget = clockTS >= 1000u && clockTS < 7000u;
                break;
            case LASERT6SMID:
                laserTarget = (clockTS >= ((delayLen >> 1) - 3000u))
                        &&(clockTS < ((delayLen >> 1) + 3000u));
                break;
            case LASERT6SLATE:
                laserTarget = (clockTS >= (delayLen - 7000u))
                        &&(clockTS < (delayLen - 1000u));
                break;
            case LASERT10SEARLY:
                laserTarget = clockTS >= 1000u && clockTS < 11000u;
                break;
            case LASERT12SEARLY:
                laserTarget = clockTS >= 1000u && clockTS < 13000u;
                break;
            case LASERT12SMID:
                laserTarget = (clockTS >= ((delayLen >> 1) - 6000u))
                        &&(clockTS < ((delayLen >> 1) + 6000u));
                break;
                break;
            case LASERT12SLATE:
                laserTarget = (clockTS >= (delayLen - 13000u))
                        &&(clockTS < (delayLen - 1000u));
                break;
            default:
                laserTarget=0;
                break;
        }
    } else {
        switch (laser_G2.laserTrialType) {
            case LASERTBASE10S:
                laserTarget = millisCounter >= (delayOnsetTS - 12000u) && millisCounter < (delayOnsetTS - 2000u);
                break;
            case LASERTBASE6S:
                laserTarget = millisCounter >= (delayOnsetTS - 8000u) && millisCounter < (delayOnsetTS - 2000u);
                break;
            default:
                laserTarget=0;
                break;
        }
    }

    if (laserTarget != 0) {
        if (laser_G2.on == 0) {
            laser_G2.on = 1;
            return 1;
        }
    } else {//laserTarget==0
        if (laser_G2.on == 1) {
            laser_G2.on = 0;

            return 0;
        }
    }
    return -1;
}

void waitTrial_G2() {
    static int waitingLickRelease = 0;
    if (!taskParam.waitForTrial) {
        return;
    }

    //    while (adcdataL > lickThreshL || adcdataR > lickThreshR) { //TODO: removed
        volatile int sel = (int) ((((double) (adcdataL - adcdataR)) / (adcdataL + adcdataR) + 1)*512);
//    volatile int sel = adcdataL;
    while (sel > lickThreshL || sel < lickThreshR) {
        if (!waitingLickRelease) {
            serialSend(SpTrialWait, 100);
            waitingLickRelease = 1;
            wait_ms(200);
        }
                sel = (int) ((((double) (adcdataL - adcdataR)) / (adcdataL + adcdataR) + 1)*512);
//        sel = adcdataL;
    }
    waitingLickRelease = 0;

    while (u2Received != 0x31) {
        serialSend(SpTrialWait, 1);
        wait_ms(200);
    }
    u2Received = -1;
}
//
//void turnOnLaser_G2(int type) {
//    laser_G2.on = 1;
//    LCDsetCursor(3, 0);
//    LCD_Write_Char('L');
//    serialSend(SpLaserSwitch, 1);
//}
//
//void turnOffLaser_G2() {
//    laser_G2.on = 0;
//    LCDsetCursor(3, 0);
//    LCD_Write_Char('.');
//    serialSend(SpLaserSwitch, 0);
//}
