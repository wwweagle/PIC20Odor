/*
 * File:   main.c
 * Author: Tony Lab
 *
 * Created on August 7, 2017, 9:47 AM
 */

#include <i2c.h>
#include <stdlib.h>
//#include <stdbool.h>

#include "utils.h"
#include "hal.h"
#include "lcdi2c.h"

void callFunc(int n);
void testOneValve(int n, int iti, int repeat);
void testValveFast(int board, int valve, int keep);
void testValveOnRA14();
void readADCData();
void testPorts();
void testNSetThres();
void zxLaserSessions_G2(int trialsPerSession, int missLimit, int totalSession);
void addAllOdor();
void bleedWater();
void testVolume(int repeat, int side);
void setWaterLen();
void testLaser(int type);
//void switchOdorPath(int i);
void testNewPorts();
static void feedWaterLR();




unsigned int taskType_G2 = DNMS_TASK;
const char odorTypes_G2[] = "BYRQHNKLTXZdMAES0123456";
int correctionRepeatCount = 0;
int currentSession;
int isLRLED;
int teachChance = 4;
//int totalSession;
//int alterOdorPath=0;

int main(void) {
    initPorts();
    initADC();
    initTMR1();
    initUART2();
    initI2C();
    LCD_PCF8574_ADDR = getLCDAddr();

    LCD_Init();
    splash_G2(__DATE__, __TIME__);
    //    switchOdorPath(1);
    while (1) {
        callFunc(getFuncNumber(2, "Main Func?"));
    }

    StopI2C();
    CloseI2C();
    return 0;
}

void testWaterDual() {
    int i = getFuncNumber(1, "Pump #?");
    if (i == 1) {
        PORTDbits.RD4 = 1;
    } else if (i == 2) {
        PORTDbits.RD5 = 1;
    }
}

static int isLikeOdorClassL(int odor) {
    return (odor == 6 || odor == 16 || odor == 17 || odor == 3 || odor == 5);
}

void addAllOdor() {
    int i;
    int odor;
    if (taskParam.outTaskPairs > 0) {
        taskParam.outSamples = malloc(taskParam.outTaskPairs * sizeof (int));
        if (!(taskType_G2 == ODR_2AFC_TASK || taskType_G2 == GONOGO_TASK))
            taskParam.outTests = malloc(taskParam.outTaskPairs * sizeof (int));
        for (i = 0; i < taskParam.outTaskPairs; i++) {
            odor = getFuncNumber(2, "Add an sample");
            taskParam.outSamples[i] = odor;
            if (!(taskType_G2 == ODR_2AFC_TASK || taskType_G2 == GONOGO_TASK)) {
                odor = getFuncNumber(2, "Add an test");
                taskParam.outTests[i] = odor;
            }
        }
    }
    if (taskParam.innerTaskPairs > 0) {
        taskParam.innerSamples = malloc(taskParam.innerTaskPairs * sizeof (int));
        taskParam.innerTests = malloc(taskParam.innerTaskPairs * sizeof (int));
        for (i = 0; i < taskParam.innerTaskPairs; i++) {
            odor = getFuncNumber(2, "Add an sample2");
            taskParam.innerSamples[i] = odor;
            odor = getFuncNumber(2, "Add an test2");
            taskParam.innerTests[i] = odor;
        }
    }
    if (taskParam.respCount > 0) {
        taskParam.respCue = malloc(taskParam.respCount * sizeof (int));
        for (i = 0; i < taskParam.respCount; i++) {
            odor = getFuncNumber(2, "Add an rsps cue");
            taskParam.respCue[i] = odor;
        }
    }
}

void bleedWater() { //menu 28
    int s = getFuncNumber(1, "1-L 2-R 3-Both");
    switch (s) {
        case 1:
            setWaterPortOpen(LICKING_LEFT, 1);
            break;
        case 2:
            setWaterPortOpen(LICKING_RIGHT, 1);
            break;
        case 3:
            setWaterPortOpen(LICKING_LEFT, 1);
            Nop();
            Nop();
            Nop();
            Nop();
            setWaterPortOpen(LICKING_RIGHT, 1);
            break;
    }
}

void testVolume(int repeat, int side) {
    if (side == 0) {
        side = getFuncNumber(1, "1-Left 2-Right");
    }

    int i, localSide = 0, waterLen = 0;
    if (side == 1) {
        localSide = LICKING_LEFT;
        waterLen = waterLenL;
    } else if (side == 2) {
        localSide = LICKING_RIGHT;
        waterLen = waterLenR;
    }

    for (i = 0; i < repeat; i++) {
        setWaterPortOpen(localSide, 1);
        wait_ms(waterLen);
        setWaterPortOpen(localSide, 0);
        if (waterLen <= 500)
            wait_ms(500 - waterLen);
        else
            wait_ms(2000 - waterLen);
    }
}

void callFunc(int n) {
    lickThreshL = (read_eeprom_G2(EEP_LICK_THRESHOLD_L)) << 2;
    lickThreshR = (read_eeprom_G2(EEP_LICK_THRESHOLD_R)) << 2;
    waterLenL = read_eeprom_G2(EEP_WATER_LEN_MS_L);
    waterLenR = read_eeprom_G2(EEP_WATER_LEN_MS_R);
    srand((unsigned int) millisCounter);
    switch (n) {
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
            teachChance = getFuncNumber(1, "TeachChance(df4)");
            break;
        case 24:
            testNSetThres();
            break;
        case 25:
        {
            lick_G2.refreshLickReading = 1;
            int interval = getFuncNumber(2, "Water Intvl?");
            //            testVolume(5, LICKING_LEFT);
            feedWaterFast_G2(interval * 100);
            break;
        }
        case 26:
        {
            splash_G2("ODPA R_D", "");
            int noLaser = getFuncNumber(1, "No Laser?");
            laser_G2.laserSessionType = noLaser ? LASER_NO_TRIAL : LASER_CATCH_TRIAL;
            laser_G2.laserTrialType = laserDuringDelayChR2;
            taskType_G2 = ODPA_RD_TASK;
            taskParam.falsePunish = getFuncNumber(1, "False Punish 2/0");
            taskParam.outTaskPairs = 2;
            addAllOdor();
            taskParam.outDelay = getFuncNumber(2, "Delay duration");
            taskParam.ITI = getFuncNumber(2, "ITI duration");
            int sessNum = getFuncNumber(2, "Session number?");
            zxLaserSessions_G2(20, 20, sessNum);
            break;
        }
        case 27:
        {
            //int dpadrOdors[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 13};
            int dpadrOdors[] = {2, 3, 6, 7, 9, 14, 15, 16, 17, 18, 19};
            int i;
            for (i = 0; i < (sizeof (dpadrOdors) / sizeof (int)); i++) {
                testOneValve(dpadrOdors[i], 10, 1);
            }
            for (i = 0; i < 20; i++) {
                setWaterPortOpen(LICKING_LEFT, 1);
                wait_ms(waterLenL);
                setWaterPortOpen(LICKING_LEFT, 0);
                if (waterLenL <= 500)
                    wait_ms(500 - waterLenL);
            }
            laser_G2.on = 1;
            break;
        }
        case 28:
            bleedWater();
            break;
        case 29:
            testVolume(100, 0);
            break;
        case 30:
            setWaterLen();
            break;
        case 31:
            testLaser(getFuncNumber(1, "0Step 1Puls 2cfs"));
            break;


        case 33:
        {
            splash_G2("ODPA", "SHAPING");
            laser_G2.laserSessionType = LASER_NO_TRIAL;
            taskType_G2 = ODPA_SHAPING_TASK;
            taskParam.teaching = 1;
            taskParam.falsePunish = 0;
            taskParam.outTaskPairs = 2;
            taskParam.respCount = 0;
            taskParam.outSamples = malloc(taskParam.outTaskPairs * sizeof (int));
            taskParam.outTests = malloc(taskParam.outTaskPairs * sizeof (int));
            taskParam.outSamples[0] = 6;
            taskParam.outSamples[1] = 7;
            taskParam.outTests[0] = 5;
            taskParam.outTests[1] = 12;
            taskParam.outDelay = 5;
            taskParam.ITI = 10;
            taskParam.minBlock = 4;
            int sessNum = getFuncNumber(2, "Session number?");
            zxLaserSessions_G2(20, 20, sessNum);
            break;
        }
        case 34:
        {
            splash_G2("ODPA", "LEARNING");
            laser_G2.laserSessionType = LASER_NO_TRIAL;
            taskType_G2 = ODPA_TASK;
            taskParam.teaching = 0;
            taskParam.falsePunish = 0;
            taskParam.outTaskPairs = 2;
            taskParam.respCount = 0;
            taskParam.outSamples = malloc(taskParam.outTaskPairs * sizeof (int));
            taskParam.outTests = malloc(taskParam.outTaskPairs * sizeof (int));
            taskParam.outSamples[0] = 6;
            taskParam.outSamples[1] = 7;
            taskParam.outTests[0] = 5;
            taskParam.outTests[1] = 12;
            taskParam.outDelay = 5;
            taskParam.ITI = 10;
            taskParam.minBlock = 4;
            int sessNum = getFuncNumber(2, "Session number?");
            zxLaserSessions_G2(20, 20, sessNum);
            break;
        }

        case 35:
        {
            splash_G2("ODPA Multi Samp", "4 Sample");
            int noLaser = getFuncNumber(1, "No Laser?");
            laser_G2.laserSessionType = noLaser ? LASER_NO_TRIAL : LASER_CATCH_TRIAL;
            laser_G2.laserTrialType = laserDuringDelayChR2;
            taskType_G2 = ODPA_RD_TASK;
            taskParam.falsePunish = getFuncNumber(1, "False Punish 2/0");
            taskParam.outTaskPairs = 4;
            taskParam.respCount = 0;
            addAllOdor();
            taskParam.outDelay = getFuncNumber(2, "Delay duration");
            taskParam.ITI = getFuncNumber(2, "ITI duration");
            int sessNum = getFuncNumber(2, "Session number?");
            zxLaserSessions_G2(40, 20, sessNum);
            break;
        }

        case 36:
        {
            splash_G2("GO-Nogo", "(RD)");
            int noLaser = getFuncNumber(1, "No Laser?");
            laser_G2.laserSessionType = noLaser ? LASER_NO_TRIAL : LASER_OTHER_TRIAL;
            laser_G2.laserTrialType = laserDuringDelayChR2;

            taskType_G2 = GONOGO_TASK;
            taskParam.respCount = getFuncNumber(1, "Resp cue?");
            taskParam.teaching = getFuncNumber(1, "Teaching?");
            taskParam.falsePunish = 0;
            taskParam.outTaskPairs = 2;
            addAllOdor();
            taskParam.outDelay = getFuncNumber(2, "Delay duration");
            taskParam.ITI = getFuncNumber(2, "ITI duration");
            //            waterLen = getFuncNumber(1, "Water fold?") * waterLen;
            int sessNum = getFuncNumber(2, "Session number?");
            zxLaserSessions_G2(20, 20, sessNum);
            break;
        }
        case 37:
        {
            splash_G2("Seq 2AFC", "6 Samp Var Rwd");
            int noLaser = getFuncNumber(1, "No Laser?");
            laser_G2.laserSessionType = noLaser ? LASER_NO_TRIAL : LASER_CATCH_TRIAL;
            laser_G2.laserTrialType = laserDuringDelayChR2;
            taskType_G2 = Seq2AFC_TASK;
            taskParam.teaching = 1;
            taskParam.respCount = 0;
            taskParam.falsePunish = 0;
            taskParam.outTaskPairs = 6;
            taskParam.minBlock = 6;
            addAllOdor();
            taskParam.outDelay = getFuncNumber(2, "Delay duration");
            taskParam.ITI = getFuncNumber(2, "ITI duration");
            int sessNum = getFuncNumber(2, "Session number?");
            zxLaserSessions_G2(60, 20, sessNum);
            break;
        }

        case 38:
        {
            splash_G2("ODPA Multi Samp", "6 Sample");
            int noLaser = getFuncNumber(1, "No Laser?");
            laser_G2.laserSessionType = noLaser ? LASER_NO_TRIAL : LASER_CATCH_TRIAL;
            laser_G2.laserTrialType = laserDuringDelayChR2;
            taskType_G2 = ODPA_RD_TASK;
            taskParam.falsePunish = getFuncNumber(1, "False Punish 2/0");
            taskParam.outTaskPairs = 6;
            taskParam.minBlock = 6;
            taskParam.respCount = 0;
            addAllOdor();
            taskParam.outDelay = getFuncNumber(2, "Delay duration");
            taskParam.ITI = getFuncNumber(2, "ITI duration");
            int sessNum = getFuncNumber(2, "Session number?");
            zxLaserSessions_G2(60, 20, sessNum);
            break;
        }


        case 39:
        {
            splash_G2("Seq 2AFC", "6 Samp Var Rwd");
            int noLaser = getFuncNumber(1, "No Laser?");
            laser_G2.laserSessionType = noLaser ? LASER_NO_TRIAL : LASER_OTHER_TRIAL;
            laser_G2.laserTrialType = laserDuringDelayChR2;
            taskType_G2 = Seq2AFC_TASK;
            taskParam.respCount = 0;
            taskParam.falsePunish = 0;
            taskParam.outTaskPairs = 6;
            taskParam.minBlock = 6;
            addAllOdor();
            taskParam.outDelay = getFuncNumber(2, "Delay duration");
            taskParam.ITI = getFuncNumber(2, "ITI duration");
            int sessNum = getFuncNumber(2, "Session number?");
            zxLaserSessions_G2(60, 20, sessNum);
            break;
        }

        case 40:
        {
            splash_G2("Seq 2AFC w/dstrc", "6 Samp Var Rwd");
            int noLaser = getFuncNumber(1, "No Laser?");
            laser_G2.laserSessionType = noLaser ? LASER_NO_TRIAL : LASER_OTHER_TRIAL;
            laser_G2.laserTrialType = laserDuringDelayChR2;
            taskType_G2 = Seq2AFC_TASK;
            taskParam.respCount = 0;
            taskParam.falsePunish = 0;
            taskParam.outTaskPairs = 6;
            taskParam.innerTaskPairs = 1;
            taskParam.minBlock = 6;
            addAllOdor();
            taskParam.outDelay = 12;
            taskParam.ITI = 5;
            int sessNum = getFuncNumber(2, "Session number?");
            zxLaserSessions_G2(60, 20, sessNum);
            break;
        }

        case 41:
        {
            splash_G2("Seq 2AFC +DR", "6 Samp Var Rwd");
            int noLaser = getFuncNumber(1, "No Laser?");
            laser_G2.laserSessionType = noLaser ? LASER_NO_TRIAL : LASER_OTHER_TRIAL;
            laser_G2.laserTrialType = laserDuringDelayChR2;
            taskType_G2 = Seq2AFC_TASK;
            taskParam.teaching = 1;
            taskParam.respCount = 0;
            taskParam.falsePunish = 0;
            taskParam.outTaskPairs = 6;
            taskParam.innerTaskPairs = 2;
            taskParam.minBlock = 6;
            addAllOdor();
            taskParam.outDelay = 12;
            taskParam.ITI = 5;
            int sessNum = getFuncNumber(2, "Session number?");
            zxLaserSessions_G2(60, 20, sessNum);
            break;
        }

        case 42:
        {
            splash_G2("Seq 2AFC +DR", "6 Samp Var Rwd");
            int noLaser = getFuncNumber(1, "No Laser?");
            laser_G2.laserSessionType = noLaser ? LASER_NO_TRIAL : LASER_OTHER_TRIAL;
            laser_G2.laserTrialType = laserDuringDelayChR2;
            taskType_G2 = Seq2AFC_TASK;
            taskParam.respCount = 0;
            taskParam.falsePunish = 0;
            taskParam.outTaskPairs = 6;
            taskParam.innerTaskPairs = 2;
            taskParam.minBlock = 6;
            addAllOdor();
            taskParam.outDelay = 12;
            taskParam.ITI = 5;
            int sessNum = getFuncNumber(2, "Session number?");
            zxLaserSessions_G2(60, 20, sessNum);
            break;
        }

        case 43:
        {
            int start = getFuncNumber(2, "Start from?");
            int len = getFuncNumber(2, "Length?");
            int repeat = getFuncNumber(2, "Repeat?");
            int iti = getFuncNumber(2, "ITI?");
            int i = start;
            for (; i < start + len; i++)
                testOneValve(i, iti, repeat);
        }
            break;


        case 44:
        {
            testWaterDual();
            break;
        }


        case 45:
        {
            splash_G2("Seq 2AFC", "Mult Samp VarRwd");
            int noLaser = getFuncNumber(1, "No Laser?");
            taskParam.teaching = noLaser;
            laser_G2.laserSessionType = noLaser ? LASER_NO_TRIAL : LASER_OTHER_TRIAL;
            laser_G2.laserTrialType = laserDuringDelayChR2;
            taskType_G2 = Seq2AFC_TASK;
            taskParam.respCount = 0;
            taskParam.falsePunish = 0;
            taskParam.outTaskPairs = getFuncNumber(1, "S/T Pairs?");
            addAllOdor();
            int trialPerSess = 0;
            switch (taskParam.outTaskPairs) {
                case 2:
                    trialPerSess = 20;
                    break;
                case 4:
                    trialPerSess = 40;
                    break;
                case 6:
                    taskParam.minBlock = 6;
                    trialPerSess = 60;
                    break;
            }
            taskParam.outDelay = getFuncNumber(2, "Delay duration");
            taskParam.ITI = getFuncNumber(2, "ITI duration");
            int sessNum = getFuncNumber(2, "Session number?");

            zxLaserSessions_G2(trialPerSess, 20, sessNum);
            break;
        }

        case 46:
        {
            splash_G2("Seq 2AFC", "Early Late");
            laser_G2.laserSessionType = LASER_HALF_HALF;
            taskType_G2 = Seq2AFC_TASK;
            taskParam.respCount = 0;
            taskParam.falsePunish = 0;
            taskParam.outTaskPairs = 6;
            taskParam.minBlock = 6;
            addAllOdor();
            taskParam.outDelay = 8;
            taskParam.ITI = 5;
            int sessNum = 30;
            zxLaserSessions_G2(60, 20, sessNum);
            break;
        }

        case 47:
        {
            splash_G2("Dual Task", "Training");
            int noLaser = getFuncNumber(1, "No Laser?");
            laser_G2.laserSessionType = noLaser ? LASER_NO_TRIAL : LASER_EVERY_TRIAL;
            laser_G2.laserTrialType = laserDuringDelayChR2;
            taskType_G2 = DUAL_TASK;
            taskParam.falsePunish = 0;
            taskParam.outTaskPairs = 2;
            taskParam.innerTaskPairs = 2;
            taskParam.respCount = 0;
            addAllOdor();
            taskParam.outDelay = 10;
            taskParam.ITI = 15;
            int sessNum = getFuncNumber(2, "Session number?");
            zxLaserSessions_G2(20, 40, sessNum);
            break;

        }

        case 48:
        {
            splash_G2("Dual Task", "Training");
            int noLaser = getFuncNumber(1, "No Laser?");
            laser_G2.laserSessionType = noLaser ? LASER_NO_TRIAL : LASER_CATCH_TRIAL;
            laser_G2.laserTrialType = laserDuringDelayChR2;
            taskType_G2 = DUAL_TASK_SHAPING;
            taskParam.falsePunish = 0;
            taskParam.outTaskPairs = 2;
            taskParam.innerTaskPairs = 1;
            taskParam.respCount = 0;
            taskParam.teaching = 1;
            //taskParam.minBlock = 8;
            addAllOdor();
            taskParam.outDelay = 10;
            taskParam.ITI = 15;
            int sessNum = getFuncNumber(2, "Session number?");
            zxLaserSessions_G2(20, 40, sessNum);
            break;
        }
        case 49:
        {

            int odorPort = getFuncNumber(2, "Valve?");
            //            taskTimeCounter += dTime;
            set4076_4bit(odorPort > 15 ? odorPort - 16 : odorPort); // target valve
            muxOff(odorPort < 16 ? (~1) : (~4));
            wait_ms(500); // turn off three-connected valve
            muxOff(odorPort < 16 ? (~3) : (~0x0c)); //turn on two-connected valve
            wait_ms(60000); // turn off three-connected valve
            muxOff(odorPort < 16 ? (~2) : (~8));
            wait_ms(200);
            muxOff(0x0f);
            break;


        }


        case 51:
        case 101:
            testNewPorts();
            break;

        case 52:
            lick_G2.refreshLickReading = 1;
            feedWaterLR();
            break;

        case 53:
        {

            lick_G2.refreshLickReading = 1;
            splash_G2("ODR 2AFC", "");

            int opLaser = getFuncNumber(1, "Laser?");
            laser_G2.laserSessionType = opLaser ? LASER_OTHER_TRIAL : LASER_NO_TRIAL;
            laser_G2.laserTrialType = LASERT3SMID;
            taskType_G2 = ODR_2AFC_TASK;
            taskParam.teaching = getFuncNumber(1, "0Tst 1Tch 23NoPu");
            taskParam.waitForTrial = getFuncNumber(1, "TrialWait 1Y 0N");
            taskParam.falsePunish = getFuncNumber(1, "False Punish 2/0");
            taskParam.outTaskPairs = 2;
            taskParam.respCount = 1;
            addAllOdor();
            taskParam.outDelay = getFuncNumber(2, "Delay duration");
            taskParam.ITI = getFuncNumber(2, "ITI duration");
            int sessNum = getFuncNumber(2, "Session number?");
            zxLaserSessions_G2(20, 100, sessNum);
            lick_G2.refreshLickReading = 0;
            break;
        }
        case 54:
        {
            lick_G2.refreshLickReading = 1;
            splash_G2("ODR 2AFC SWCH", "");
            laser_G2.laserSessionType = LASER_NO_TRIAL;
            taskType_G2 = ODR_2AFC_TASK;
            taskParam.teaching = getFuncNumber(1, "0Tst 1Tch 23NoPu");
            taskParam.waitForTrial = getFuncNumber(1, "TrialWait 1Y 0N");
            taskParam.falsePunish = getFuncNumber(1, "False Punish 2/0");
            taskParam.falsePunish = 0;
            taskParam.outTaskPairs = 2;
            taskParam.respCount = 1;
            taskParam.outDelay = getFuncNumber(2, "Delay duration");
            taskParam.ITI = getFuncNumber(2, "ITI duration");
            int sessNum = getFuncNumber(2, "Session number?");
            int sessRpt = getFuncNumber(2, "Session repeat?");
            taskParam.outSamples = malloc(taskParam.outTaskPairs * sizeof (int));
            taskParam.respCue = malloc(taskParam.respCount * sizeof (int));
            taskParam.respCue[0] = 0;
            for (; sessRpt > 0; sessRpt--) {
                int odor = (sessRpt % 2) == 0 ? 6 : 7;
                taskParam.outSamples[0] = odor;
                taskParam.outSamples[1] = odor;

                zxLaserSessions_G2(4, 100, sessNum);

            }
            lick_G2.refreshLickReading = 0;
            break;
        }
        case 55:
        {
            splash_G2("ODR 2AFC MIX", "");
            laser_G2.laserSessionType = LASER_NO_TRIAL;
            taskType_G2 = ODR_2AFC_TASK;
            taskParam.minBlock = 6;
            taskParam.teaching = 2;
            taskParam.waitForTrial = 1;
            taskParam.falsePunish = 0;
            taskParam.outTaskPairs = 6;
            taskParam.respCount = 1;
            taskParam.outSamples = malloc(taskParam.outTaskPairs * sizeof (int));
            taskParam.respCue = malloc(taskParam.respCount * sizeof (int));

            taskParam.respCue[0] = 0;
            taskParam.outSamples[0] = 6;
            taskParam.outSamples[1] = 7;
            taskParam.outSamples[2] = 16;
            taskParam.outSamples[3] = 17;
            taskParam.outSamples[4] = 18;
            taskParam.outSamples[5] = 19;

            taskParam.outDelay = 2;
            taskParam.ITI = 8;
            int sessNum = getFuncNumber(2, "Session number?");
            zxLaserSessions_G2(24, 24, sessNum);
            lick_G2.refreshLickReading = 0;
            break;
        }
        case 61:
        {
            splash_G2("DNMS", "SHAPING");
            //            int noLaser = getFuncNumber(1, "No Laser?");
            laser_G2.laserSessionType = LASER_NO_TRIAL;
            taskType_G2 = DNMS_SHAPING_TASK;
            taskParam.teaching = 1;
            taskParam.falsePunish = 0;
            taskParam.outTaskPairs = 2;
            taskParam.outSamples = malloc(taskParam.outTaskPairs * sizeof (int));
            taskParam.outTests = malloc(taskParam.outTaskPairs * sizeof (int));
            taskParam.outSamples[0] = 2;
            taskParam.outSamples[1] = 3;
            taskParam.outTests[0] = 2;
            taskParam.outTests[1] = 3;
            taskParam.respCount = 0;
            //            addAllOdor();
            taskParam.outDelay = getFuncNumber(2, "Delay duration");
            taskParam.ITI = getFuncNumber(2, "ITI duration");
            //            waterLenL = getFuncNumber(1, "Water fold?") * waterLenL;
            int sessNum = getFuncNumber(2, "Session number?");
            zxLaserSessions_G2(12, 50, sessNum);
            break;
        }
        case 62:
        {
            splash_G2("DNMS", "TEST");
            //            int noLaser = getFuncNumber(1, "No Laser?");
            laser_G2.laserSessionType = LASER_NO_TRIAL;
            taskType_G2 = DNMS_TASK;
            taskParam.teaching = getFuncNumber(1, "0Test 1Teach");
            ;
            taskParam.falsePunish = 0;
            taskParam.outTaskPairs = 2;
            taskParam.outSamples = malloc(taskParam.outTaskPairs * sizeof (int));
            taskParam.outTests = malloc(taskParam.outTaskPairs * sizeof (int));
            taskParam.outSamples[0] = 2;
            taskParam.outSamples[1] = 3;
            taskParam.outTests[0] = 2;
            taskParam.outTests[1] = 3;
            taskParam.respCount = 0;
            taskParam.outDelay = getFuncNumber(2, "Delay duration");
            taskParam.ITI = getFuncNumber(2, "ITI duration");
            int sessNum = getFuncNumber(2, "Session number?");
            if (taskParam.outDelay >= 20) {
                waterLenL = waterLenL + (waterLenL >> 1);
            }
            zxLaserSessions_G2(12, 50, sessNum);
            break;
        }
        case 63:
        {
            splash_G2("DNMS ELF", "OPTOGENETICS");
            //            int noLaser = getFuncNumber(1, "No Laser?");
            laser_G2.laserSessionType = LASER_SESS_ELF;
            taskType_G2 = ELF_DNMS_TASK;
            taskParam.teaching = 0;
            taskParam.falsePunish = 0;
            taskParam.outTaskPairs = 2;
            taskParam.outSamples = malloc(taskParam.outTaskPairs * sizeof (int));
            taskParam.outTests = malloc(taskParam.outTaskPairs * sizeof (int));
            taskParam.outSamples[0] = 2;
            taskParam.outSamples[1] = 3;
            taskParam.outTests[0] = 2;
            taskParam.outTests[1] = 3;
            taskParam.respCount = 0;
            taskParam.outDelay = 12;
            taskParam.ITI = 24;
            taskParam.minBlock = 40;
            int sessNum = getFuncNumber(2, "Session number?");
            zxLaserSessions_G2(40, 50, sessNum);
            break;
        }

        case 64:
        {
            splash_G2("DNMS ELF 18", "OPTOGENETICS");
            //            int noLaser = getFuncNumber(1, "No Laser?");
            laser_G2.laserSessionType = LASER_SESS_ELF;
            taskType_G2 = ELF_DNMS_TASK;
            taskParam.teaching = 0;
            taskParam.falsePunish = 0;
            taskParam.outTaskPairs = 2;
            taskParam.outSamples = malloc(taskParam.outTaskPairs * sizeof (int));
            taskParam.outTests = malloc(taskParam.outTaskPairs * sizeof (int));
            taskParam.outSamples[0] = 2;
            taskParam.outSamples[1] = 3;
            taskParam.outTests[0] = 2;
            taskParam.outTests[1] = 3;
            taskParam.respCount = 0;
            taskParam.outDelay = 12;
            taskParam.ITI = 24;
            taskParam.minBlock = 40;
            int sessNum = getFuncNumber(2, "Session number?");
            zxLaserSessions_G2(40, 50, sessNum);
            break;
        }

        case 65:
        {
            splash_G2("DNMS ELF", "OPTOGENETICS");
            //            int noLaser = getFuncNumber(1, "No Laser?");
            laser_G2.laserSessionType = LASER_SESS_ELF;
            taskType_G2 = ELF_DNMS_TASK;
            taskParam.teaching = 0;
            taskParam.falsePunish = 0;
            taskParam.outTaskPairs = 2;
            taskParam.outSamples = malloc(taskParam.outTaskPairs * sizeof (int));
            taskParam.outTests = malloc(taskParam.outTaskPairs * sizeof (int));
            taskParam.outSamples[0] = 2;
            taskParam.outSamples[1] = 3;
            taskParam.outTests[0] = 2;
            taskParam.outTests[1] = 3;
            taskParam.respCount = 0;
            taskParam.outDelay = 12;
            taskParam.ITI = 24;
            taskParam.minBlock = 40;
            int sessNum = getFuncNumber(2, "Session number?");
            zxLaserSessions_G2(40, 50, sessNum);
            break;
        }
        case 66:
        {
            splash_G2("DNMS ELF 18", "OPTOGENETICS");
            //            int noLaser = getFuncNumber(1, "No Laser?");
            laser_G2.laserSessionType = LASER_SESS_ELF_20;
            taskType_G2 = ELF_DNMS_TASK;
            taskParam.teaching = 0;
            taskParam.falsePunish = 0;
            taskParam.outTaskPairs = 2;
            taskParam.outSamples = malloc(taskParam.outTaskPairs * sizeof (int));
            taskParam.outTests = malloc(taskParam.outTaskPairs * sizeof (int));
            taskParam.outSamples[0] = 2;
            taskParam.outSamples[1] = 3;
            taskParam.outTests[0] = 2;
            taskParam.outTests[1] = 3;
            taskParam.respCount = 0;
            taskParam.outDelay = 20;
            taskParam.ITI = 40;
            taskParam.minBlock = 48;
            int sessNum = getFuncNumber(2, "Session number?");
            zxLaserSessions_G2(48, 50, sessNum);
            break;
        }
        case 67:
        {
            splash_G2("DNMS ELF VDLY", "OPTOGENETICS");
            //            int noLaser = getFuncNumber(1, "No Laser?");
            laser_G2.laserSessionType = LASER_SESS_ELF_VARDELAY;
            taskType_G2 = ELF_DNMS_TASK_VARDELAY;
            taskParam.teaching = 0;
            taskParam.falsePunish = 0;
            taskParam.outTaskPairs = 2;
            taskParam.outSamples = malloc(taskParam.outTaskPairs * sizeof (int));
            taskParam.outTests = malloc(taskParam.outTaskPairs * sizeof (int));
            taskParam.outSamples[0] = 2;
            taskParam.outSamples[1] = 3;
            taskParam.outTests[0] = 2;
            taskParam.outTests[1] = 3;
            taskParam.respCount = 0;
            taskParam.outDelay = 20;
            taskParam.ITI = getFuncNumber(2, "ITI?");
            ;
            taskParam.minBlock = 32;
            int sessNum = getFuncNumber(2, "Session number?");
            zxLaserSessions_G2(taskParam.minBlock, 50, sessNum);
            break;
        }

        default:
        {
            int i;
            for (i = n; i < n + 4; i++)
                testOneValve(i, 8, 10);
        }
            break;

    }
    free(taskParam.outSamples);
    free(taskParam.outTests);
    free(taskParam.respCue);

}

void testValveFast(int board, int valve, int keep) {
    LCDclear();
    LCDhome();
    LCD_Write_Str("BOARD   VALVE");

    lcdWriteNumber_G2(board, 6, 0);
    lcdWriteNumber_G2(valve, 6, 1);

    set4076_4bit(valve - 1);

    while (1) {
        muxOff(~board);
        if (!keep) {
            wait_ms(500);
            muxOff(0x0f);
            wait_ms(500);
        }

    }

}

void testOneValve(int valve, int iti, int repeat) {
    const int preCharge = 500;

    //    int repeat = 10;
    //    int iti = 8;
    const int onTime = 1000;
    int closingAdvance = 195;
    //    int valve;
    int rpt;
    for (rpt = 0; rpt < repeat; rpt++) {
        LCDclear();
        LCDhome();
        LCD_Write_Str("Valve");
        LCDsetCursor(0, 1);
        LCD_Write_Str("Repeat");
        lcdWriteNumber_G2(rpt + 1, 7, 1);
        //        serialSend(3, rpt + 1);
        //        for (valve = 0; valve < 20; valve++) {
        lcdWriteNumber_G2(valve, 6, 0);
        int P10Val = valve < 16 ? valve : valve - 16;
        int boardA = valve < 16 ? 1 : 4;
        int boardB = valve < 16 ? 2 : 8;
        serialSend(SpIO, valve);
        set4076_4bit(P10Val);
        muxOff(~boardA);
        wait_ms(preCharge);
        BNC_1 = 1;
        muxOff(~(boardA | boardB));
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
        muxOff(~boardB);
        wait_ms(closingAdvance);
        BNC_2 = 0;
        muxOff(0x0F);
        serialSend(SpIO, 0);
        BNC_1 = 0;
        wait_ms(iti * 1000 - preCharge);

    }
}

void readADCData(void) {
    int i = getFuncNumber(1, "ADC channel");
    while (1) {
        volatile int temp = i == 0 ? adcdataL : adcdataR;
        int highByte = temp / 100;
        int lowByte = temp % 100;

        serialSend(23, highByte);
        serialSend(24, lowByte);
        wait_ms(50);

    }
}

int readADCDataNorm(void) {
    while (1) {
        volatile int temp = (((double) (adcdataL - adcdataR)) / (adcdataL + adcdataR) + 1)*512;
        int highByte = temp / 100;
        int lowByte = temp % 100;

        serialSend(23, highByte);
        serialSend(24, lowByte);
        wait_ms(25);
        sendChart(temp, 0);
        wait_ms(25);
    }
    return 0;
}

void testPorts() {
    TRISF = 0xFF3F;
    int i = 0;
    while (1) {
        i = i^1;
        PORTFbits.RF6 = i;
        Nop();
        Nop();
        Nop();
        PORTBbits.RB8 = i;
        Nop();
        Nop();
        Nop();
        PORTBbits.RB9 = i;
        Nop();
        Nop();
        Nop();

        PORTFbits.RF7 = i;
        Nop();
        Nop();
        Nop();
        PORTFbits.RF6 = i;
        Nop();
        Nop();
        Nop();
        PORTAbits.RA14 = i;
        Nop();
        Nop();
        Nop();
        PORTDbits.RD12 = i;
        Nop();
        Nop();
        Nop();
        PORTDbits.RD13 = i;
        //        Nop();
        //        Nop();
        //        Nop();
        //        PORTAbits.RA15=i;
        wait_ms(250);
    }
}

void feedWaterFast_G2(int interval) {

    lick_G2.LCount = 0;
    sendLargeValue(lickThreshL);
    //    lick_G2.RCount = 0;
    unsigned int waterCount = 0;
    unsigned int totalLickCount = 0;

    LCDclear();
    LCDhome();
    LCD_Write_Str("Total Lick");

    timerCounterI = 1000;
    while (1) {
        if (lick_G2.LCount > totalLickCount) {
            if (timerCounterI >= interval) {
                setWaterPortOpen(LICKING_LEFT, 1);
                timerCounterI = 0;
                lcdWriteNumber_G2(++waterCount, 12, 1);
            }
            totalLickCount = lick_G2.LCount;
            lcdWriteNumber_G2(totalLickCount, 11, 0);
        }
        if (timerCounterI >= waterLenL) {
            setWaterPortOpen(LICKING_LEFT, 0);
        }
        if (u2Received == '1') {
            setWaterPortOpen(LICKING_LEFT, 1);
            wait_ms(waterLenL);
            setWaterPortOpen(LICKING_LEFT, 0);
            u2Received = -1;
        }
    }
}

static void feedWaterLR() { //Menu #52
    taskType_G2 = GONOGO_2AFC_TASK;
    int lastLocation = 0;
    lick_G2.LCount = 0u;
    lick_G2.RCount = 0u;
    unsigned int waterCount = 0;
    unsigned int lastL = 0;
    unsigned int lastR = 0;
    int interval = getFuncNumber(2, "Interval 100mSec");

    splash_G2("L       R", "W");

    timerCounterI = interval * 100;
    while (1) {
        if (lick_G2.LCount > lastL) {
            if (timerCounterI > interval * 100 && lastLocation != LICKING_LEFT) {
                serialSend(22, 1);
                setWaterPortOpen(LICKING_LEFT, 1);
                lastLocation = LICKING_LEFT;
                timerCounterI = 0;
                LCDsetCursor(15, 0);
                LCD_Write_Char('L');
                lcdWriteNumber_G2(++waterCount, 2, 1);
            }
            lcdWriteNumber_G2(lick_G2.LCount, 2, 0);
            lastL = lick_G2.LCount;
        } else if (lick_G2.RCount > lastR) {
            if (timerCounterI > interval * 100 && lastLocation != LICKING_RIGHT) {
                serialSend(22, 2);
                setWaterPortOpen(LICKING_RIGHT, 1);
                lastLocation = LICKING_RIGHT;
                timerCounterI = 0;
                LCDsetCursor(15, 0);
                LCD_Write_Char('R');
                lcdWriteNumber_G2(++waterCount, 2, 1);
            }
            lcdWriteNumber_G2(lick_G2.RCount, 10, 0);
            lastR = lick_G2.RCount;
        }

        if (timerCounterI > waterLenL) {
            setWaterPortOpen(LICKING_LEFT, 0);
        }
        if (timerCounterI > waterLenR) {
            setWaterPortOpen(LICKING_RIGHT, 0);
        }

        if (u2Received == '1') {
            setWaterPortOpen(LICKING_LEFT, 1);
            wait_ms(waterLenL);
            setWaterPortOpen(LICKING_LEFT, 0);
            u2Received = -1;
        }
        if (u2Received == '2') {
            setWaterPortOpen(LICKING_RIGHT, 1);
            wait_ms(waterLenR);
            setWaterPortOpen(LICKING_RIGHT, 0);
            u2Received = -1;
        }

    }
}

void testNSetThres() { // #24
    sendLargeValue(lickThreshL);
    sendLargeValue(lickThreshR);
    int side = getFuncNumber(1, "1-LEFT 2-RIGHT");
    int newThres = getFuncNumber(3, "New Lick Thres?");
    sendLargeValue(newThres);
    if (side == 1)
        write_eeprom_G2(EEP_LICK_THRESHOLD_L, newThres >> 2);
    else if (side == 2)
        write_eeprom_G2(EEP_LICK_THRESHOLD_R, newThres >> 2);
    Nop();
    Nop();
    Nop();
    lickThreshL = (read_eeprom_G2(EEP_LICK_THRESHOLD_L)) << 2;
    lickThreshR = (read_eeprom_G2(EEP_LICK_THRESHOLD_R)) << 2;
}

void stim_G2(int place, int odorPort, int laserType) {
    set4076_4bit(odorPort > 15 ? odorPort - 16 : odorPort);
    if (place == 1 || place == 2 || place == 6) {
        muxOff(odorPort < 16 ? (~1) : (~4));
        waitTaskTimer(500);
    }

    if (place == 3) {
        serialSend(SpIO, odorPort > 0 ? odorPort : odorPort + 100);
        muxOff(odorPort < 16 ? (~1) : (~4));
    } else {
        if (place == 4) {
            LATE = LATE | 0x0300;
        } else if (isLikeOdorClassL(odorPort)) {
            BNC_3 = 1;
        } else {
            BNC_4 = 1;
        }
        muxOff(odorPort < 16 ? (~3) : (~0x0c));
        int stimSend = 0;
        switch (place) {
            case 1:
            case 2:
                if (isLikeOdorClassL(odorPort)) {
                    stimSend = 9;
                } else {
                    stimSend = 10;
                }
                break;
                //            case 3:
                //                serialSend(SpIO,odorPort);
                //                break;
            case 4:
                stimSend = SpResponseCue;
                break;
            case 5:
                stimSend = SpCorrectionCue;
                break;
            case 6:
                stimSend = Sp_DistractorSample;
                break;
            case 7:
                stimSend = Sp_DistractorTest;
                break;
        }
        serialSend(stimSend, odorPort == 0 ? 100 + odorPort : odorPort);
        LCDsetCursor(3, 0);
        switch (place) {
            case 1:
                LCD_Write_Char('1');
                waitTaskTimer(taskParam.sample1Length - 200);
                break;
            case 2:
                LCD_Write_Char('2');
                waitTaskTimer(taskParam.test1Length - 200);
                break;
            case 4:
                LCD_Write_Char('3');
                waitTaskTimer(taskParam.respCueLength - 200);
                break;
            case 5:
                waitTaskTimer(taskParam.correctionCueLength - 200);
                break;
            case 6:
                LCD_Write_Char('u');
                waitTaskTimer(taskParam.sample2Length - 200);
                break;
            case 7:
                LCD_Write_Char('U');
                waitTaskTimer(taskParam.test2Length - 200u);
                break;
        }
        muxOff(odorPort < 16 ? (~2) : (~8));
        waitTaskTimer(200);
        muxOff(0x0f);

        serialSend(stimSend, 0);
        LCDsetCursor(3, 0);
        switch (place) {
            case 1:
                LCD_Write_Char('d');
                if (taskParam.sample1Length < 1000u)
                    waitTaskTimer(1000u - taskParam.sample1Length);
                break;
            case 2:
                LCD_Write_Char('D');
                if (taskParam.test1Length < 1000u)
                    waitTaskTimer(1000u - taskParam.test1Length);
                break;
            case 4:
                LCD_Write_Char('R');
                break;
            case 6:
                LCD_Write_Char('d');
                break;
            case 7:
                LCD_Write_Char('d');
                break;
        }
    }
    Nop();
    Nop();
    //    BNC_3 = 0;
    //    BNC_4 = 0;
    LATE = LATE & 0xfcff;

}

static void processHit_G2(int id, int ratio) {
    serialSend(22, 1);
    if (lick_G2.lickSide == 'L') {
        setWaterPortOpen(LICKING_LEFT, 1);
        waitTaskTimer(waterLenL);
        setWaterPortOpen(LICKING_LEFT, 0);
    } else if (lick_G2.lickSide == 'R') {
        setWaterPortOpen(LICKING_RIGHT, 1);
        waitTaskTimer(waterLenR);
        setWaterPortOpen(LICKING_RIGHT, 0);
    }
    currentMiss = 0;
    serialSend(SpHit, id);
    lcdWriteNumber_G2(++hit, 5, 0);
}

static void processFalse_G2(int id) {
    currentMiss = 0;
    serialSend(SpFalseAlarm, id);
    lcdWriteNumber_G2(++falseAlarm, 5, 1);
}

static void processMiss_G2(int id) {
    currentMiss++;
    serialSend(SpMiss, id);
    lcdWriteNumber_G2(++miss, 9, 0);
    if (taskParam.falsePunish > 0)
        waitTaskTimer(5000u);
}

static int waterNResult_G2(int sample, int test, int id, int rewardWindow) {
    int rtn = 0;
    int tried = 0;
    lick_G2.lickSide = 0;

    switch (taskType_G2) {
        case GONOGO_TASK:
            for (timerCounterI = 0; timerCounterI < rewardWindow && lick_G2.lickSide != 'L'; lick_G2.lickSide = lick_G2.stable);
            taskTimeCounter = millisCounter;
            /////Reward
            if (lick_G2.lickSide != 'L') {
                if (!isLikeOdorClassL(sample)) {
                    serialSend(SpCorrectRejection, OUTCOME_1PORT_OR_2AFC_L);
                    lcdWriteNumber_G2(++correctRejection, 9, 1);
                } else {
                    processMiss_G2(OUTCOME_1PORT_OR_2AFC_L);
                    if (((rand() % teachChance) == 0 && (taskParam.teaching & 1)) || u2Received == '1') {
                        serialSend(SpLickFreq, 1);
                        setWaterPortOpen(LICKING_LEFT, 1);
                        serialSend(SpWater, OUTCOME_1PORT_OR_2AFC_L);
                        waitTaskTimer(waterLenL);
                        setWaterPortOpen(LICKING_LEFT, 0);
                    }
                }
            } else if (!isLikeOdorClassL(sample)) {
                processFalse_G2(OUTCOME_1PORT_OR_2AFC_L);
            } else {
                processHit_G2(OUTCOME_1PORT_OR_2AFC_L, 1);
            }
            break;
        case ODR_2AFC_TASK:

            for (timerCounterI = 0; timerCounterI < rewardWindow && !lick_G2.lickSide; lick_G2.lickSide = lick_G2.stable) {
                if (timerCounterI == rewardWindow / 2 && (!tried)) {
                    tried = 1;
                    if (((rand() % teachChance) == 0 && (taskParam.teaching & 1)) || u2Received == '1') {
                        serialSend(SpLickFreq, 1);
                        serialSend(SpWater, id);
                        if (isLikeOdorClassL(sample)) {
                            setWaterPortOpen(LICKING_LEFT, 1); //(side, on/off))
                            wait_ms(waterLenL);
                            setWaterPortOpen(LICKING_LEFT, 0); //(side, on/off))
                        } else {
                            setWaterPortOpen(LICKING_RIGHT, 1);
                            wait_ms(waterLenR);
                            setWaterPortOpen(LICKING_RIGHT, 0);
                        }
                        u2Received = -1;
                        break;
                    }
                }
            }
            taskTimeCounter = millisCounter;
            /////Reward
            id = isLikeOdorClassL(sample) ? OUTCOME_1PORT_OR_2AFC_L : OUTCOME_2AFCR;
            if (!lick_G2.lickSide) {
                processMiss_G2(id);
                taskParam.falsePunish |= 1;
            } else if ((isLikeOdorClassL(sample) && lick_G2.lickSide == 'R')
                    || (lick_G2.lickSide == 'L' && !isLikeOdorClassL(sample))) {
                processFalse_G2(id);
                taskParam.falsePunish |= 1;
            } else {
                processHit_G2(id, 1);
                taskParam.falsePunish &= 0xFE;
            }
            break;

        default:
            /*///////////////
             *DNMS
             *//////////////
            //        case DNMS_TASK:
            //        case SHAPING_TASK:

            //        case NO_ODOR_CATCH_TRIAL_TASK:
            //        case VARY_ODOR_LENGTH_TASK:
            //        case DELAY_DISTRACTOR:

            //        case _ASSOCIATE_TASK:
            //        case _ASSOCIATE_SHAPING_TASK:
            //        

            ///////////Detect/////////////////
            for (timerCounterI = 0; timerCounterI < rewardWindow && !lick_G2.lickSide;
                    lick_G2.lickSide = lick_G2.stable) {
                if (timerCounterI == rewardWindow / 2 && (!tried)) {
                    tried = 1;
                    if (((rand() % teachChance) == 0 && (taskParam.teaching & 1)) || u2Received == '1') {
                        serialSend(SpLickFreq, 1);
                        serialSend(SpWater, id);

                        setWaterPortOpen(LICKING_LEFT, 1); //(side, on/off))
                        wait_ms(waterLenL);
                        setWaterPortOpen(LICKING_LEFT, 0); //(side, on/off))

                        u2Received = -1;
                        break;
                    }
                }
            }
            taskTimeCounter = millisCounter;
            /////Reward
            if (!lick_G2.lickSide) {
                if (isLikeOdorClassL(sample) == isLikeOdorClassL(test)) {
                    serialSend(SpCorrectRejection, id);
                    lcdWriteNumber_G2(++correctRejection, 9, 1);
                    rtn = SpCorrectRejection;
                } else {
                    processMiss_G2(id);
                    rtn = SpMiss;
                }
            } else if (isLikeOdorClassL(sample) == isLikeOdorClassL(test)) {
                processFalse_G2(id);
                rtn = SpFalseAlarm;
            } else {
                //Seq-2AFC was here, removed Jan 23 2019
                processHit_G2(id, 1);
                rtn = SpHit;
            }
            break;

    }

    //    waitTaskTimer(rewardWindow);
    return rtn;

}


//static void seq2AFCResult(int firstOdor, int laserType) {
//    switch (taskType_G2) {
//        case Seq2AFC_TEACH:
//            //            if (taskParam.respCueLength >= 200) {
//        {
//            int cueSeq[] = {0, 1};
//            if (rand() % 2) {
//                cueSeq[0] = 1;
//                cueSeq[1] = 0;
//            }
//            waitTaskTimer(500u);
//            stim_G2(3, taskParam.respCue[cueSeq[0]], laserType);
//            waitTaskTimer(500u);
//            stim_G2(4, taskParam.respCue[cueSeq[0]], laserType);
//            LCDsetCursor(3, 0);
//            int rtn = waterNResult_G2(firstOdor, taskParam.respCue[cueSeq[0]], 4);
//            waitTaskTimer(500u);
//            if (rtn == SpCorrectRejection || rtn == SpMiss) {
//                stim_G2(3, taskParam.respCue[cueSeq[1]], laserType);
//                waitTaskTimer(500u);
//                stim_G2(4, taskParam.respCue[cueSeq[1]], laserType);
//                waterNResult_G2(firstOdor, taskParam.respCue[cueSeq[1]], 5);
//            }
//        }
//    }
//}

void dual_task_D_R(int laserType, int sample2, int test2) {
    // delay+0.5s
    stim_G2(6, sample2, laserType); //delay+2s w/1000ms sample
    LCDsetCursor(3, 0);
    LCD_Write_Char('D');
    if (test2 == 0) {
        waitTaskTimer(3000u); //delay+5000ms, func return
    } else {
        int licked = waitTaskTimer(1000u); //delay+3s;
        set4076_4bit(test2 > 15 ? test2 - 16 : test2);
        muxOff(test2 < 16 ? (~1) : (~4)); // 1* 2 4* 8
        licked |= waitTaskTimer(500); //delay+3.5s
        if (!licked) { //test and reward
            stim_G2(7, test2, laserType);
            waterNResult_G2(sample2, test2, OUTCOME_DUAL, 500); //delay+5s
        } else {//abortTrial
            muxOff(0x0f);
            serialSend(SpAbortTrial, OUTCOME_DUAL);
            LCDsetCursor(3, 0);
            LCD_Write_Char('A');
            waitTaskTimer(1500u);
        }

    }




}

int delayedRspsDelay(int laserType, int waterPortSide) {
    if (taskParam.respCueLength >= 200) {
        int delayLick = 0;
        if (taskParam.outDelay >= 2) {
            delayLick |= waitTaskTimer(((unsigned int) (taskParam.outDelay - 1)) * 1000u);
            delayLick |= waitTaskTimer(500u);
        }
        stim_G2(3, taskParam.respCue[0], laserType);
        delayLick |= waitTaskTimer(500u);
        LCDsetCursor(3, 0);
        if (taskParam.teaching < 2 && delayLick) {
            muxOff(0x0f);
            serialSend(SpAbortTrial, waterPortSide);
            LCD_Write_Char('A');
            abortTrial++;
            return 0;
        } else {
            stim_G2(4, taskParam.respCue[0], laserType);
            return 1;
        }
    } else {
        return 0;
    }
}

static void zxLaserTrial_G2(int sOutter, int tOutter, int sInner, int tInner, int laserTType) {
    taskTimeCounter = millisCounter;
    delayOnsetTS = millisCounter + 1000u * (uint32_t) taskParam.ITI + 1000u;
    serialSend(SpLaserTType, laserTType);
    serialSend(Splaser, (laserTType != LASER_OFF));

    LCDsetCursor(3, 0);
    LCD_Write_Char('I');

    if (taskParam.ITI >= 4u) {
        unsigned int trialITI = taskParam.ITI - 4u;
        while (trialITI > 60u) {
            waitTaskTimer(60u * 1000u);
            trialITI -= 60u;
        }
        waitTaskTimer(trialITI * 1000u);
    }
    waitTaskTimer(3500u);
    serialSend(SpITI, 0);
    /////////////////////////////////////////////////
    stim_G2(1, sOutter, laserTType);
    ////////////////////////////////////////////////


    switch (taskType_G2) {
            //Do nothing during Go Nogo Tasks
        case GONOGO_TASK:
        case ODR_2AFC_TASK:
            break; //next line just before waterNresult

        default:
            if (taskParam.outDelay > 0) {
                if (taskParam.innerTaskPairs != 0) {
                    /////////DISTRACTOR TIMELINE @Delay+0/////////////
                    waitTaskTimer(2500u);
                    dual_task_D_R(laserTType, sInner, tInner); //Delay+5000ms
                    if (taskParam.outDelay >= 8u) {
                        waitTaskTimer((unsigned int) taskParam.outDelay * 1000 - 8000);
                    }
                } else {
                    ////// DNMS/DPA Pairs 2 count==0, no inner task, delay >4
                    waitTaskTimer((unsigned int) taskParam.outDelay * 1000u - 1000u);
                }
                waitTaskTimer(500u);
            }

            ///////////-Second odor-/////////////////
            stim_G2(2, tOutter, laserTType);
            //////////////////////////////////////////
            break;
    }
    int resultRtn = 0;
    switch (taskParam.respCount) {
        case 0:
            //            waitTaskTimer(1000u);
            LCDsetCursor(3, 0);
            LCD_Write_Char('R');
            resultRtn = waterNResult_G2(sOutter, tOutter, OUTCOME_1PORT_OR_2AFC_L, taskParam.teaching ? 2000 : 1000);
            //DPA SEQ-2AFC WAS HERE, REOVED Jan,23, 2019

            break;
        case 1:
            if (delayedRspsDelay(laserTType, isLikeOdorClassL(sOutter) ? OUTCOME_1PORT_OR_2AFC_L : OUTCOME_2AFCR))
                resultRtn = waterNResult_G2(sOutter, tOutter, OUTCOME_2AFCR, 2000);
            break;
            //case 2:
            //    seq2AFCResult(s1, laserType);
            //   break;

    }
    //    waitTaskTimer(1000u); //water time sync
    // Total Trials
    LCD_Write_Char('I');
    int totalTrials = hit + correctRejection + miss + falseAlarm + abortTrial;
    lcdWriteNumber_G2(totalTrials, 13, 1);
    // Discrimination rate
    if (hit + correctRejection > 0) {
        correctRatio = 100 * (hit + correctRejection) / totalTrials;
        correctRatio = correctRatio > 99 ? 99 : correctRatio;
    } else {
        correctRatio = 0;
    }
    lcdWriteNumber_G2(correctRatio, correctRatio > 9 ? 13 : 14, 0);
    waitTrial_G2();
}

void setWaterLen() {//#30
    sendLargeValue(waterLenL);
    sendLargeValue(waterLenR);
    int side = getFuncNumber(1, "1-Left, 2-Right");
    int newWaterLen = getFuncNumber(3, "New water len?");
    sendLargeValue(newWaterLen);

    if (side == 1)
        write_eeprom_G2(EEP_WATER_LEN_MS_L, newWaterLen);
    else if (side == 2)
        write_eeprom_G2(EEP_WATER_LEN_MS_R, newWaterLen);
    waterLenL = read_eeprom_G2(EEP_WATER_LEN_MS_L);
    waterLenR = read_eeprom_G2(EEP_WATER_LEN_MS_R);
    serialSend(61, 0);
}

void zxLaserSessions_G2(int trialsPerSession, int missLimit, int totalSession) {


    //    wait_ms(1000);
    int currentTrial = 0;
    currentSession = 0;
    int laserOnType = laser_G2.laserTrialType;
    unsigned int* shuffledList = malloc(taskParam.minBlock * sizeof (unsigned int));
    serialSend(SpTaskType, taskType_G2);
    while ((currentMiss < missLimit) && (currentSession++ < totalSession)) {
        serialSend(SpSess, 1);
        splash_G2("    H___M___ __%", "S__ F___C___t___");
        lcdWriteNumber_G2(currentSession, 1, 1);
        hit = miss = falseAlarm = correctRejection = abortTrial = 0;
        int outSample = 0, outTest = 0;
        int innerSample = 0, innerTest = 0;
        for (currentTrial = 0; currentTrial < trialsPerSession && currentMiss < missLimit;) {
            shuffleArray_G2(shuffledList, taskParam.minBlock);
            int idxInMinBlock;
            for (idxInMinBlock = 0; idxInMinBlock < taskParam.minBlock && currentMiss < missLimit; idxInMinBlock++) {
                int shuffledMinBlock = shuffledList[idxInMinBlock];
                switch (taskType_G2) {

                    case DNMS_TASK:
                        outSample = (shuffledMinBlock == 0 || shuffledMinBlock == 2) ? taskParam.outSamples[0] : taskParam.outSamples[1];
                        outTest = (shuffledMinBlock == 1 || shuffledMinBlock == 2) ? taskParam.outTests[0] : taskParam.outTests[1];
                        break;
                    case ELF_DNMS_TASK:
                    case ELF_DNMS_TASK_VARDELAY:
                        outSample = (shuffledMinBlock & 1) ? taskParam.outSamples[0] : taskParam.outSamples[1];
                        outTest = (shuffledMinBlock & 2) ? taskParam.outSamples[0] : taskParam.outSamples[1];
                        if (taskType_G2 == ELF_DNMS_TASK_VARDELAY) {
                            delayOnsetTS = millisCounter + 1000u * (uint32_t) taskParam.ITI + 1000u;
                            int delayLens[] = {5, 8, 12, 20};
                            taskParam.outDelay = delayLens[(shuffledMinBlock & 0xc) >> 2];
                        }

                        break;
                    case DNMS_SHAPING_TASK:
                        outSample = (shuffledMinBlock == 0 || shuffledMinBlock == 2) ? taskParam.outSamples[0] : taskParam.outSamples[1];
                        outTest = (shuffledMinBlock == 0 || shuffledMinBlock == 2) ? taskParam.outTests[1] : taskParam.outTests[0];
                        break;
                    case GONOGO_TASK:
                    case ODR_2AFC_TASK:
                        if ((taskParam.falsePunish & 0x03) != 0x03 || correctionRepeatCount > 9 || currentTrial == 0) {
                            if (taskParam.outTaskPairs == 2) {
                                outSample = (shuffledMinBlock == 0 || shuffledMinBlock == 2) ? taskParam.outSamples[0] : taskParam.outSamples[1];
                            } else if (taskParam.outTaskPairs > 4) {
                                outSample = taskParam.outSamples[shuffledMinBlock];
                            }
                            correctionRepeatCount = 0;
                        } else {
                            correctionRepeatCount++;
                        }
                        outTest = taskType_G2 == odorTypes_G2 ? taskParam.respCue[0] : 0;
                        break;

                    case ODPA_TASK:
                        outSample = (shuffledMinBlock == 0 || shuffledMinBlock == 2) ? taskParam.outSamples[0] : taskParam.outSamples[1];
                        outTest = (shuffledMinBlock == 1 || shuffledMinBlock == 2) ? taskParam.outTests[0] : taskParam.outTests[1];
                        break;

                    case ODPA_SHAPING_TASK:
                        outSample = (shuffledMinBlock == 0 || shuffledMinBlock == 2) ? taskParam.outSamples[0] : taskParam.outSamples[1];
                        outTest = (outSample == taskParam.outSamples[0]) ? taskParam.outTests[1] : taskParam.outTests[0];
                        break;

                    case DUAL_TASK_SHAPING:
                        outSample = (shuffledMinBlock == 0 || shuffledMinBlock == 2) ? taskParam.outSamples[0] : taskParam.outSamples[1];
                        outTest = (outSample == taskParam.outSamples[0]) ? taskParam.outTests[1] : taskParam.outTests[0];
                        break;

                    case DUAL_TASK:
                        outSample = (shuffledMinBlock == 0 || shuffledMinBlock == 2) ? taskParam.outSamples[0] : taskParam.outSamples[1];
                        outTest = (shuffledMinBlock == 1 || shuffledMinBlock == 2) ? taskParam.outTests[0] : taskParam.outTests[1];
                        break;
                }

                LCDsetCursor(0, 0);
                LCD_Write_Char(odorTypes_G2[outSample]);
                LCD_Write_Char(odorTypes_G2[outTest]);

                switch (taskParam.innerTaskPairs) {
                    case 1:
                        innerSample = taskParam.innerSamples[0];
                        innerTest = taskParam.innerTests[0];
                        break;
                    case 2:
                        innerSample = (rand()&1) ? taskParam.innerSamples[0] : taskParam.innerSamples[1];
                        innerTest = taskParam.innerTests[0];
                        break;
                }




                switch (laser_G2.laserSessionType) {
                    case LASER_NO_TRIAL:
                        laser_G2.laserTrialType = LASER_OFF;
                        break;
                    case LASER_EVERY_TRIAL:
                        break;
                    case LASER_OTHER_TRIAL:
                        laser_G2.laserTrialType = (currentTrial % 2) == 0 ? LASER_OFF : laserOnType;
                        break;
                    case LASER_SESS_ELF:
                    {
                        static const int elfTypes[] = {
                            LASER_OFF,
                            LASERT3SEARLY,
                            LASERT6SEARLY,
                            LASERT10SEARLY,
                            LASERT6SMID,
                            LASERT3SMID,
                            LASERT6SLATE,
                            LASERT3SLATE,
                            LASERTBASE6S,
                            LASERTBASE10S
                        };
                        laser_G2.laserTrialType = elfTypes[shuffledMinBlock >> 2];
                        break;
                    }
                    case LASER_SESS_ELF_20:
                    {
                        static const int elfTypes[] = {
                            LASER_OFF,
                            LASER_OFF,
                            LASERT3SEARLY,
                            LASERT6SEARLY,
                            LASERT12SEARLY,
                            LASERT3SMID,
                            LASERT6SMID,
                            LASERT12SMID,
                            LASERT3SLATE,
                            LASERT6SLATE,
                            LASERT12SLATE,
                            LASERTBASE10S
                        };
                        laser_G2.laserTrialType = elfTypes[shuffledMinBlock >> 2];
                        break;
                    }
                    case LASER_SESS_ELF_VARDELAY:
                    {
                        if (shuffledMinBlock > 15) {
                            laser_G2.laserTrialType = LASERT3SLATE;
                        } else {
                            laser_G2.laserTrialType = LASER_OFF;
                        }

                        break;
                    }
                        //                    case LASER_CATCH_TRIAL:
                        //                    {
                        //                        int toCatch = trialsPerSession * 2 / 10;
                        //                        laser_G2.laserTrialType = ((trialsPerSession - currentTrial) <= toCatch) ? laserOnType : LASER_OFF;
                        //                        break;
                        //                    }
                        //
                        //                    case LASER_LR_EACH_QUARTER:
                        //                        laser_G2.side = isLikeOdorClassL(outSample) ? 1 : 2;
                        //                    case LASER_EACH_QUARTER:
                        //                        switch (currentTrial % 5) {
                        //                            case 0:
                        //                                laser_G2.laserTrialType = LASER_OFF;
                        //                                break;
                        //                            case 1:
                        //                                laser_G2.laserTrialType = laserDuring1Quarter;
                        //                                break;
                        //                            case 2:
                        //                                laser_G2.laserTrialType = laserDuring2Quarter;
                        //                                break;
                        //                            case 3:
                        //                                laser_G2.laserTrialType = laserDuring3Quarter;
                        //                                break;
                        //                            case 4:
                        //                                laser_G2.laserTrialType = laserDuring4Quarter;
                        //                                break;
                        //                        }
                        //                        break;
                        //
                        //
                        //                    case LASER_12s_LR_EACH_QUARTER:
                        //                        laser_G2.side = isLikeOdorClassL(outSample) ? 1 : 2;
                        //                    case LASER_12s_EACH_QUARTER:
                        //                        switch (currentTrial % 5) {
                        //                            case 0:
                        //                                laser_G2.laserTrialType = LASER_OFF;
                        //                                break;
                        //                            case 1:
                        //                                laser_G2.laserTrialType = laserDuring12s1Quarter;
                        //                                break;
                        //                            case 2:
                        //                                laser_G2.laserTrialType = laserDuring12s2Quarter;
                        //                                break;
                        //                            case 3:
                        //                                laser_G2.laserTrialType = laserDuring12s3Quarter;
                        //                                break;
                        //                            case 4:
                        //                                laser_G2.laserTrialType = laserDuring12s4Quarter;
                        //                                break;
                        //                        }
                        //                        break;
                        //
                        //
                        //                    case LASER_VARY_LENGTH:
                        //                        switch (currentTrial % 5) {
                        //                            case 0:
                        //                                laser_G2.laserTrialType = LASER_OFF;
                        //                                break;
                        //                            case 1:
                        //                                laser_G2.laserTrialType = laser4sRamp;
                        //                                break;
                        //                            case 2:
                        //                                laser_G2.laserTrialType = laser2sRamp;
                        //                                break;
                        //                            case 3:
                        //                                laser_G2.laserTrialType = laser1sRamp;
                        //                                break;
                        //                            case 4:
                        //                                laser_G2.laserTrialType = laser_5sRamp;
                        //                                break;
                        //                        }
                        //                        break;
                        //
                        //                    case LASER_HALF_HALF:
                        //                        switch (currentTrial % 6) {
                        //                            case 0:
                        //                            case 3:
                        //                                laser_G2.laserTrialType = LASER_OFF;
                        //                                break;
                        //                            case 1:
                        //                            case 5:
                        //                                laser_G2.laserTrialType = laserDuringEarlyHalf;
                        //                                break;
                        //                            case 2:
                        //                            case 4:
                        //                                laser_G2.laserTrialType = laserDuringLateHalf;
                        //                                break;
                        //                        }
                        //                        break;
                        //
                        //
                        //                    case LASER_LR_EVERYTRIAL:
                        //                        laser_G2.side = isLikeOdorClassL(outSample) ? 1 : 2;
                        //                        break;
                        //
                        //                    case LASER_LR_EVERY_OTHER_TRIAL:
                        //                        laser_G2.side = isLikeOdorClassL(outSample) ? 1 : 2;
                        //
                        //                        laser_G2.laserTrialType = (currentTrial % 2) == 0 ? LASER_OFF : laserOnType;
                        //                        break;
                        //
                        //                    case LASER_INCONGRUENT_CATCH_TRIAL:
                        //                        if ((currentTrial > 3 && currentTrial < 8 && isLikeOdorClassL(outSample) && isLikeOdorClassL(outTest))
                        //                                || (currentTrial > 7 && currentTrial < 12 && isLikeOdorClassL(outSample)&& !isLikeOdorClassL(outTest))
                        //                                || (currentTrial > 11 && currentTrial < 16 && !isLikeOdorClassL(outSample) && isLikeOdorClassL(outTest))
                        //                                || (currentTrial > 15 && currentTrial < 20 && !isLikeOdorClassL(outSample) && !isLikeOdorClassL(outTest))) {
                        //                            laser_G2.side = isLikeOdorClassL(outSample) ? 2 : 1;
                        //                        } else {
                        //                            laser_G2.side = isLikeOdorClassL(outSample) ? 1 : 2;
                        //                        }
                        //                        break;
                        //                    case LASER_DUAL_TASK_ODAP_ON_OFF:
                        //                        laser_G2.laserTrialType = (shuffledMinBlock < 2) ? LASER_OFF : laserCoverDistractor;
                        //                        break;
                    case LASER_OTHER_BLOCK:
                        if (rand()&0x1)
                            laser_G2.laserTrialType = currentTrial < (trialsPerSession / 2) ? LASER_OFF : laserOnType;
                        else
                            laser_G2.laserTrialType = currentTrial < (trialsPerSession / 2) ? laserOnType : LASER_OFF;
                        break;
                }
                zxLaserTrial_G2(outSample, outTest, innerSample, innerTest, laser_G2.laserTrialType);
                currentTrial++;
            }
        }
        //        serialSend()
        serialSend(SpSess, 0);
        if (!lick_G2.refreshLickReading) {
            sendChart(correctRatio, 0);
            sendChart(miss * 100 / (miss + hit), 1);
        }
    }
    serialSend(SpTrain, 0); // send it's the end
    u2Received = -1;
    free(shuffledList);
}

void testLaser(int type) { //31
    laser_G2.laserTrialType = LASER_TEST;
    int i = 0;
    switch (type) {
        case 0:
            while (1) {
                i ^= 1;
                laser_G2.on = i;
                getFuncNumber(1, "Toggle Laser");

            }
            break;
        case 1:
            while (1) {
                laser_G2.on = 1;
                wait_Sec(5);
                laser_G2.on = 0;
                wait_Sec(5);
            }
            break;
        case 2:
            for (i = 0; i < 50; i++) {
                laser_G2.on = 1;
                wait_Sec(3);
                laser_G2.on = 0;
                wait_Sec(17);
                serialSend(SpDebugInfo, i);
            }
            break;
    }
}

void testNewPorts() {
    volatile int temp;
    int highByte;
    int lowByte;
    while (1) {
        PORTAbits.RA15 = 1;
        Nop();
        Nop();
        Nop();
        PORTDbits.RD5 = 1;
        Nop();
        Nop();
        Nop();
        PORTDbits.RD7 = 0;


        wait_ms(50);

        PORTAbits.RA15 = 0;
        Nop();
        Nop();
        Nop();
        PORTDbits.RD5 = 0;
        Nop();
        Nop();
        Nop();
        PORTDbits.RD6 = 1;
        Nop();
        Nop();
        Nop();
        temp = adcdataR;
        highByte = temp / 100;
        lowByte = temp % 100;

        serialSend(23, highByte);
        serialSend(24, lowByte);
        wait_ms(500);

        PORTDbits.RD6 = 0;
        Nop();
        Nop();
        Nop();
        PORTDbits.RD7 = 1;

        temp = adcdataR;
        highByte = temp / 100;
        lowByte = temp % 100;

        serialSend(23, highByte);
        serialSend(24, lowByte);
        wait_ms(500);

    }
}