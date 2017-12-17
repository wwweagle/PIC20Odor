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
    .current = 0u, .stable = 0u, .filter = 0u, .portSide = 0u, .LCount = 0u, .RCount = 0u
};

LASER_T_G2 laser_G2 = {.laserSessionType = LASER_NO_TRIAL, .laserTrialType = LASER_OFF,
    .timer = 0u, .onTime = 65535u, .offTime = 0u, .on = 0u, .side = 1u}; //1L,2R,3LR

TASK_T taskParam = {
    .sample1s = NULL,
    .test1s = NULL,
    .pairs1Count = 0,
    .sample2 = 0,
    .test2 = 0,
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
    .delay1 = 5,
    .delay2 = 1500,
    .delay3 = 0,
    .ITI = 8,
    .waitForTrial = 1,
    .minBlock = 4
};

int waterLen = 50;
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

void setWaterPortOpen(int i) {
    PORTAbits.RA14 = i;
}

void sendLargeValue(int val) {
    char valHigh = (char) (val / 100);
    char valLow = (char) (val % 100);
    serialSend(23, valHigh);
    serialSend(24, valLow);
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
    int currLick = lick_G2.LCount;
    taskTimeCounter += dTime;
    while (millisCounter < taskTimeCounter);

    return lick_G2.LCount>currLick;
}

void assertLaser_G2(int type, int step) {
    switch (type) {
        case LASER_OFF:
            break;
        case laserDuringDelay:
            if (step == atDelay1SecIn) {
                turnOnLaser_G2(3);
            } else if (step == atDelayLast500mSBegin) {
                turnOffLaser_G2();
            }
            break;
        case laserDuringDelayChR2:
            if (step == atDelay1SecIn || step==atPostDualTask) {
                turnOnLaser_G2(3);
            } else if (step == atDelayLastSecBegin) {
                turnOffLaser_G2();
            }
            break;
        case laserDuringDelay_Odor2:
            if (step == atDelay1SecIn) {
                turnOnLaser_G2(3);
            } else if (step == atSecondOdorEnd) {
                turnOffLaser_G2();
            }
            break;
        case laserDuringBaselineNDelay:
            if (step == atDelay1SecIn || step == at3SecBeforeS1) {
                turnOnLaser_G2(3);
            } else if (step == atDelayLastSecBegin || step == at500msBeforeS1) {
                turnOffLaser_G2();
            }
            break;
        case laserDuringOdor:
            if (step == atS1Beginning || step == atSecondOdorBeginning) {
                turnOnLaser_G2(3);
            } else if (step == atS1End || step == atSecondOdorEnd) {
                turnOffLaser_G2();
            }
            break;
        case laserDuring1stOdor:
            if (step == atS1Beginning) {
                turnOnLaser_G2(laser_G2.side);
            } else if (step == atS1End) {
                turnOffLaser_G2();
            }
            break;
        case laserDuring2ndOdor:
            if (step == atSecondOdorBeginning) {
                turnOnLaser_G2(3);
            } else if (step == atSecondOdorEnd) {
                turnOffLaser_G2();
            }
            break;
        case laserDuringEarlyHalf:
            if (step == atDelayBegin) {
                turnOnLaser_G2(3);
            } else if (step == atDelayMiddle) {
                turnOffLaser_G2();
            }
            break;

        case laserDuringLateHalf:
            if (step == atDelayMiddle) {
                turnOnLaser_G2(3);
            } else if (step == atDelayLast500mSBegin) {
                turnOffLaser_G2();
            }
            break;

        case laserDuring1Quarter:
            if (step == atDelayBegin) {
                turnOnLaser_G2(laser_G2.side);
            } else if (step == atDelay1_5SecIn) {
                turnOffLaser_G2();
            }
            break;
        case laserDuring2Quarter:
            if (step == atDelay2SecIn) {
                turnOnLaser_G2(laser_G2.side);
            } else if (step == atDelay500msToMiddle) {
                turnOffLaser_G2();
            }
            break;
        case laserDuring3Quarter:
            if (step == atDelayMiddle) {
                turnOnLaser_G2(laser_G2.side);
            } else if (step == atDelayLast2_5SecBegin) {
                turnOffLaser_G2();
            }
            break;
        case laserDuring4Quarter:
            if (step == atDelayLast2SecBegin) {
                turnOnLaser_G2(laser_G2.side);
            } else if (step == atDelayLast500mSBegin) {
                turnOffLaser_G2();
            }
            break;
        case laserDuring12s1Quarter:
            if (step == atDelayBegin) {
                turnOnLaser_G2(laser_G2.side);
            } else if (step == atDelay2_5SecIn) {
                turnOffLaser_G2();
            }
            break;
        case laserDuring12s2Quarter:
            if (step == atDelay3SecIn) {
                turnOnLaser_G2(laser_G2.side);
            } else if (step == atDelay500msToMiddle) {
                turnOffLaser_G2();
            }
            break;
        case laserDuring12s3Quarter:
            if (step == atDelayMiddle) {
                turnOnLaser_G2(laser_G2.side);
            } else if (step == atDelayMid2_5Sec) {
                turnOffLaser_G2();
            }
            break;
        case laserDuring12s4Quarter:
            if (step == atDelayMid3Sec) {
                turnOnLaser_G2(laser_G2.side);
            } else if (step == atDelayLast500mSBegin) {
                turnOffLaser_G2();
            }
            break;
        case laserDuringResponseDelay:
            if (step == atSecondOdorEnd) {
                turnOnLaser_G2(3);
            } else if (step == atRewardBeginning) {
                turnOffLaser_G2();
            }
            break;
        case laserNoDelayControl:
            if (step == at1SecBeforeS1) {
                turnOnLaser_G2(3);
            } else if (step == atRewardBeginning) {
                turnOffLaser_G2();
            }
            break;
        case laserNoDelayControlShort:
            if (step == atS1Beginning) {
                turnOnLaser_G2(3);
            } else if (step == atSecondOdorEnd) {
                turnOffLaser_G2();
            }
            break;
        case laserDuringBaseline:
            if (step == at3SecBeforeS1) {
                turnOnLaser_G2(3);
            } else if (step == atS1Beginning) {
                turnOffLaser_G2();
            }
            break;
        case laserDuring3Baseline:
            if (step == at4SecBeforeS1) {
                turnOnLaser_G2(3);
            } else if (step == at1SecBeforeS1) {
                turnOffLaser_G2();
            }
            break;
        case laserDuring4Baseline:
            if (step == at4SecBeforeS1) {
                turnOnLaser_G2(3);
            } else if (step == at500msBeforeS1) {
                turnOffLaser_G2();
            }
            break;
        case laserDuringBaseAndResponse:
            if (step == at1SecBeforeS1 || step == atSecondOdorEnd) {
                turnOnLaser_G2(3);
            } else if (step == at500msBeforeS1 || step == atRewardBeginning) {

                turnOffLaser_G2();
            }
            break;
        case laser4sRamp:
            if (step == atDelay500MsIn) {
                turnOnLaser_G2(3);
            } else if (step == atDelayLast500mSBegin) {
                turnOffLaser_G2();
            }
            break;
        case laser2sRamp:
            if (step == atDelayLast2_5SecBegin) {
                turnOnLaser_G2(3);
            } else if (step == atDelayLast500mSBegin) {
                turnOffLaser_G2();
            }
            break;
        case laser1sRamp:
            if (step == atDelayLast1_5SecBegin) {
                turnOnLaser_G2(3);
            } else if (step == atDelayLast500mSBegin) {
                turnOffLaser_G2();
            }
            break;
        case laser_5sRamp:
            if (step == atDelayLastSecBegin) {
                turnOnLaser_G2(3);
            } else if (step == atDelayLast500mSBegin) {
                turnOffLaser_G2();
            }
            break;
            //        case laserDelayDistractorEarly:
            //            if (step == atDelay2SecIn) {
            //                turnOnLaser(3);
            //            } else if (step == atDelay2SecIn) {
            //                turnOffLaser();
            //            }
            //            break;
            //        case laserDelayDistractorLate:
            //            if (step == atDelay1SecIn) {
            //                turnOnLaser(3);
            //            } else if (step == atDelay2SecIn) {
            //                turnOffLaser();
            //            }
            //            break;
        case laserRampDuringDelay:
            if (step == atDelay1SecIn) {
                turnOnLaser_G2(laser_G2.side);
            } else if (step == atDelayLast500mSBegin) {
                turnOffLaser_G2();
            }
            break;
            //        case laserAfterDistractor:
            //            if (step == atDelayMid2_5Sec) {
            //                turnOnLaser(laser.side);
            //            } else if (step == atDelayLast2_5SecBegin) {
            //                turnOffLaser();
            //            }
            //            break;
            //        case laserAfterDistractorLong:
            //            if (step == atDelayMid2_5Sec) {
            //                turnOnLaser(laser.side);
            //            } else if (step == atDelayLast500mSBegin) {
            //                turnOffLaser();
            //            }
            //            break;

        case laserCoverDistractor:
            if (step == atPreDualTask) {
                turnOnLaser_G2(laser_G2.side);
            } else if (step == atPostDualTask) {
                turnOffLaser_G2();
            }
            break;

        case laserAfterDistractorMax:
            if (step == atPostDualTask) {
                turnOnLaser_G2(laser_G2.side);
            } else if (step == atDelayLast500mSBegin) {
                turnOffLaser_G2();
            }
            break;

        case laserAfterMultiDistractor:
            if (step == atPostDualTask) {
                turnOnLaser_G2(laser_G2.side);
            } else if (step == atPreDualTask) {
                turnOffLaser_G2();
            }
            break;


    }
}

void waitTrial_G2() {
    static int waitingLickRelease = 0;
    if (!taskParam.waitForTrial) {
        return;
    }
    while (adcdata > lickThresh) {
        if (!waitingLickRelease) {
            serialSend(20, 100);
            waitingLickRelease = 1;
            wait_ms(200);
        }
    }
    waitingLickRelease = 0;

    while (u2Received != 0x31) {
        serialSend(20, 1);
        wait_ms(200);
    }
    u2Received = -1;
}



