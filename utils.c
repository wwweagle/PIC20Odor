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

//void assertLaser_G2(int type, int step) {
//    switch (type) {
//        case LASER_OFF:
//            break;
//        case laserDuringDelay:
//            if (step == atDelayBegin) {
//                turnOnLaser_G2(3);
//            } else if (step == atDelayLast500mSBegin) {
//                turnOffLaser_G2();
//            }
//            break;
//        case laserDuringDelayChR2:
//            if (step == atDelay1SecIn || step == atPostDualTask) {
//                turnOnLaser_G2(3);
//            } else if (step == atDelayLastSecBegin) {
//                turnOffLaser_G2();
//            }
//            break;
//        case laserDuringDelay_Odor2:
//            if (step == atDelay1SecIn) {
//                turnOnLaser_G2(3);
//            } else if (step == atSecondOdorEnd) {
//                turnOffLaser_G2();
//            }
//            break;
//        case laserDuringBaselineNDelay:
//            if (step == atDelay1SecIn || step == at3SecBeforeS1) {
//                turnOnLaser_G2(3);
//            } else if (step == atDelayLastSecBegin || step == at500msBeforeS1) {
//                turnOffLaser_G2();
//            }
//            break;
//        case laserDuringOdor:
//            if (step == atS1Beginning || step == atSecondOdorBeginning) {
//                turnOnLaser_G2(3);
//            } else if (step == atS1End || step == atSecondOdorEnd) {
//                turnOffLaser_G2();
//            }
//            break;
//        case laserDuring1stOdor:
//            if (step == atS1Beginning) {
//                turnOnLaser_G2(laser_G2.side);
//            } else if (step == atS1End) {
//                turnOffLaser_G2();
//            }
//            break;
//        case laserDuring2ndOdor:
//            if (step == atSecondOdorBeginning) {
//                turnOnLaser_G2(3);
//            } else if (step == atSecondOdorEnd) {
//                turnOffLaser_G2();
//            }
//            break;
//        case laserDuringEarlyHalf:
//            if (step == atDelay1SecIn) {
//                turnOnLaser_G2(3);
//            } else if (step == atDelayMiddle) {
//                turnOffLaser_G2();
//            }
//            break;
//
//        case laserDuringLateHalf:
//            if (step == atDelayMiddle) {
//                turnOnLaser_G2(3);
//            } else if (step == atDelayLastSecBegin) {
//                turnOffLaser_G2();
//            }
//            break;
//
//        case laserDuring1Quarter:
//            if (step == atDelayBegin) {
//                turnOnLaser_G2(laser_G2.side);
//            } else if (step == atDelay1_5SecIn) {
//                turnOffLaser_G2();
//            }
//            break;
//        case laserDuring2Quarter:
//            if (step == atDelay2SecIn) {
//                turnOnLaser_G2(laser_G2.side);
//            } else if (step == atDelay500msToMiddle) {
//                turnOffLaser_G2();
//            }
//            break;
//        case laserDuring3Quarter:
//            if (step == atDelayMiddle) {
//                turnOnLaser_G2(laser_G2.side);
//            } else if (step == atDelayLast2_5SecBegin) {
//                turnOffLaser_G2();
//            }
//            break;
//        case laserDuring4Quarter:
//            if (step == atDelayLast2SecBegin) {
//                turnOnLaser_G2(laser_G2.side);
//            } else if (step == atDelayLast500mSBegin) {
//                turnOffLaser_G2();
//            }
//            break;
//        case laserDuring12s1Quarter:
//            if (step == atDelayBegin) {
//                turnOnLaser_G2(laser_G2.side);
//            } else if (step == atDelay2_5SecIn) {
//                turnOffLaser_G2();
//            }
//            break;
//        case laserDuring12s2Quarter:
//            if (step == atDelay3SecIn) {
//                turnOnLaser_G2(laser_G2.side);
//            } else if (step == atDelay500msToMiddle) {
//                turnOffLaser_G2();
//            }
//            break;
//        case laserDuring12s3Quarter:
//            if (step == atDelayMiddle) {
//                turnOnLaser_G2(laser_G2.side);
//            } else if (step == atDelayMid2_5Sec) {
//                turnOffLaser_G2();
//            }
//            break;
//        case laserDuring12s4Quarter:
//            if (step == atDelayMid3Sec) {
//                turnOnLaser_G2(laser_G2.side);
//            } else if (step == atDelayLast500mSBegin) {
//                turnOffLaser_G2();
//            }
//            break;
//        case laserDuringResponseDelay:
//            if (step == atSecondOdorEnd) {
//                turnOnLaser_G2(3);
//            } else if (step == atRewardBeginning) {
//                turnOffLaser_G2();
//            }
//            break;
//        case laserNoDelayControl:
//            if (step == at1SecBeforeS1) {
//                turnOnLaser_G2(3);
//            } else if (step == atRewardBeginning) {
//                turnOffLaser_G2();
//            }
//            break;
//            //        case laserNoDelayControlShort:
//            //            if (step == atS1Beginning) {
//            //                turnOnLaser_G2(3);
//            //            } else if (step == atSecondOdorEnd) {
//            //                turnOffLaser_G2();
//            //            }
//            //            break;
//        case laserDuringBaseline:
//            if (step == at3SecBeforeS1) {
//                turnOnLaser_G2(3);
//            } else if (step == atS1Beginning) {
//                turnOffLaser_G2();
//            }
//            break;
//        case laserDuring3Baseline:
//            if (step == at4SecBeforeS1) {
//                turnOnLaser_G2(3);
//            } else if (step == at1SecBeforeS1) {
//                turnOffLaser_G2();
//            }
//            break;
//        case laserDuring4Baseline:
//            if (step == at4SecBeforeS1) {
//                turnOnLaser_G2(3);
//            } else if (step == at500msBeforeS1) {
//                turnOffLaser_G2();
//            }
//            break;
//        case laserDuringBaseAndResponse:
//            if (step == at1SecBeforeS1 || step == atSecondOdorEnd) {
//                turnOnLaser_G2(3);
//            } else if (step == at500msBeforeS1 || step == atRewardBeginning) {
//
//                turnOffLaser_G2();
//            }
//            break;
//        case laser4sRamp:
//            if (step == atDelay500MsIn) {
//                turnOnLaser_G2(3);
//            } else if (step == atDelayLast500mSBegin) {
//                turnOffLaser_G2();
//            }
//            break;
//        case laser2sRamp:
//            if (step == atDelayLast2_5SecBegin) {
//                turnOnLaser_G2(3);
//            } else if (step == atDelayLast500mSBegin) {
//                turnOffLaser_G2();
//            }
//            break;
//        case laser1sRamp:
//            if (step == atDelayLast1_5SecBegin) {
//                turnOnLaser_G2(3);
//            } else if (step == atDelayLast500mSBegin) {
//                turnOffLaser_G2();
//            }
//            break;
//        case laser_5sRamp:
//            if (step == atDelayLastSecBegin) {
//                turnOnLaser_G2(3);
//            } else if (step == atDelayLast500mSBegin) {
//                turnOffLaser_G2();
//            }
//            break;
//            //        case laserDelayDistractorEarly:
//            //            if (step == atDelay2SecIn) {
//            //                turnOnLaser(3);
//            //            } else if (step == atDelay2SecIn) {
//            //                turnOffLaser();
//            //            }
//            //            break;
//            //        case laserDelayDistractorLate:
//            //            if (step == atDelay1SecIn) {
//            //                turnOnLaser(3);
//            //            } else if (step == atDelay2SecIn) {
//            //                turnOffLaser();
//            //            }
//            //            break;
//        case laserRampDuringDelay:
//            if (step == atDelay1SecIn) {
//                turnOnLaser_G2(laser_G2.side);
//            } else if (step == atDelayLast500mSBegin) {
//                turnOffLaser_G2();
//            }
//            break;
//            //        case laserAfterDistractor:
//            //            if (step == atDelayMid2_5Sec) {
//            //                turnOnLaser(laser.side);
//            //            } else if (step == atDelayLast2_5SecBegin) {
//            //                turnOffLaser();
//            //            }
//            //            break;
//            //        case laserAfterDistractorLong:
//            //            if (step == atDelayMid2_5Sec) {
//            //                turnOnLaser(laser.side);
//            //            } else if (step == atDelayLast500mSBegin) {
//            //                turnOffLaser();
//            //            }
//            //            break;
//
//        case laserCoverDistractor:
//            if (step == atPreDualTask) {
//                turnOnLaser_G2(laser_G2.side);
//            } else if (step == atPostDualTask) {
//                turnOffLaser_G2();
//            }
//            break;
//
//        case laserAfterDistractorMax:
//            if (step == atPostDualTask) {
//                turnOnLaser_G2(laser_G2.side);
//            } else if (step == atDelayLast500mSBegin) {
//                turnOffLaser_G2();
//            }
//            break;
//
//        case laserAfterMultiDistractor:
//            if (step == atPostDualTask) {
//                turnOnLaser_G2(laser_G2.side);
//            } else if (step == atPreDualTask) {
//                turnOffLaser_G2();
//            }
//            break;
//
//
//    }
//}

char assertLaser19() {
    char laserTarget;
    uint32_t trialTS = millisCounter - trialOnsetTS - 25000u;

    switch (laser_G2.laserTrialType) {
        case LASER_OFF:
            laserTarget = 0;
            break;
        case LASERT3SEARLY:
            laserTarget = trialTS >= 1000u && trialTS < 4000u;
            break;
        case LASERT3SMID:
            laserTarget = trialTS >= 4500u && trialTS < 7500u;
            break;
        case LASERT3SLATE:
            laserTarget = trialTS >= 8000u && trialTS < 11000u;
            break;
        case LASERT6SEARLY:
            laserTarget = trialTS >= 1000u && trialTS < 7000u;
            break;
        case LASERT6SMID:
            laserTarget = trialTS >= 3000u && trialTS < 9000u;
            break;
        case LASERT6SLATE:
            laserTarget = trialTS >= 5000u && trialTS < 11000u;
            break;
        case LASERT1A1IN12:
            laserTarget = trialTS >= 1000u && trialTS < 11000u;
            break;
        case LASERTBASEAIN12:
            laserTarget = millisCounter - trialOnsetTS >= 13000u && millisCounter - trialOnsetTS < 23000u;
            break;
        case LASERTBASE6IN12:
            laserTarget = millisCounter - trialOnsetTS >= 17000u && millisCounter - trialOnsetTS < 23000u;
            break;
        case LASER_TEST:
            return -1;
            break;
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

    //    while (adcdataL > lickThreshL || adcdataR > lickThreshR) {
    //    volatile int sel = (int) ((((double) (adcdataL - adcdataR)) / (adcdataL + adcdataR) + 1)*512);
    volatile int sel = adcdataL;
    while (sel > lickThreshL || sel < lickThreshR) {
        if (!waitingLickRelease) {
            serialSend(SpTrialWait, 100);
            waitingLickRelease = 1;
            wait_ms(200);
        }
        //        sel = (int) ((((double) (adcdataL - adcdataR)) / (adcdataL + adcdataR) + 1)*512);
        sel = adcdataL;
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
