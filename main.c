/*
 * File:   main.c
 * Author: Tony Lab
 *
 * Created on August 7, 2017, 9:47 AM
 */

#include <i2c.h>
#include <stdlib.h>

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
void testVolume();
void setWaterLen();
void testLaser();



unsigned int taskType_G2 = DNMS_TASK;
const char odorTypes_G2[] = "BYRQHNKLTXZdMAES0123456";
int correctionRepeatCount = 0;
//int alterOdorPath=0;

int main(void) {
    initPorts();
    initTMR1();
    initUART2();
    initI2C();
    LCD_Init();
    initADC();
    splash_G2(__DATE__, __TIME__);
    while (1) {
        callFunc(getFuncNumber(2, "Main Func?"));
    }

    StopI2C();
    CloseI2C();
    return 0;
}

void switchOdorPath() {
    int i = getFuncNumber(1, "Alt Path");
    PORTDbits.RD6 = i;
    Nop();
    Nop();
    PORTDbits.RD7 = ~i;
}

void testWaterDual() {
    int i = getFuncNumber(1, "Pump #?");
    if (i == 1) {
        PORTDbits.RD4 = 1;
    } else if (i == 2) {
        PORTDbits.RD5 = 1;
    }
}

static int isLikeOdorClassA(int odor) {
    if (odor == 3 || odor == 4 || odor == 7 || odor == 6 || odor == 10) return 1;
    return 0;
}

void sendChart(int val, int idx) {
    int high = ((val & 0x0fc0) >> 6)+(idx == 1 ? 0x40 : 0);
    serialSend(SpChartHigh, high);
    int low = (val & 0x3f)+(idx == 1 ? 0x40 : 0);
    serialSend(SpChartLow, low);
}

void addAllOdor() {
    int i;
    int odor;
    if (taskParam.pairs1Count > 0) {
        taskParam.sample1s = malloc(taskParam.pairs1Count * sizeof (int));
        taskParam.test1s = malloc(taskParam.pairs1Count * sizeof (int));
        for (i = 0; i < taskParam.pairs1Count; i++) {
            odor = getFuncNumber(2, "Add an sample");
            taskParam.sample1s[i] = odor;
            odor = getFuncNumber(2, "Add an test");
            taskParam.test1s[i] = odor;
        }
    }
    if (taskParam.pairs2Count > 0) {
        taskParam.sample2s = malloc(taskParam.pairs2Count * sizeof (int));
        taskParam.test2s = malloc(taskParam.pairs2Count * sizeof (int));
        for (i = 0; i < taskParam.pairs2Count; i++) {
            odor = getFuncNumber(2, "Add an sample2");
            taskParam.sample2s[i] = odor;
            odor = getFuncNumber(2, "Add an test2");
            taskParam.test2s[i] = odor;
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

void bleedWater() {
    setWaterPortOpen(1);
}

void testVolume() {
    int i;
    for (i = 0; i < 100; i++) {
        setWaterPortOpen(1);
        wait_ms(waterLen);
        setWaterPortOpen(0);
        wait_ms(500 - waterLen);
    }
}

void callFunc(int n) {
    lickThresh = (read_eeprom_G2(EEP_LICK_THRESHOLD)) << 2;
    waterLen = read_eeprom_G2(EEP_WATER_LEN_MS);
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
            //            testPorts();
            switchOdorPath();
            break;
        case 24:
            testNSetThres();
            break;
        case 25:
            feedWaterFast_G2(waterLen);
        case 26:
        {
            splash_G2("ODPA R_D", "");
            int noLaser = getFuncNumber(1, "No Laser?");
            laser_G2.laserSessionType = noLaser ? LASER_NO_TRIAL : LASER_CATCH_TRIAL;
            laser_G2.laserTrialType = laserDuringDelayChR2;
            taskType_G2 = ODPA_RD_TASK;
            taskParam.falsePunish = getFuncNumber(1, "False Punish 2/0");
            taskParam.pairs1Count = 2;
            addAllOdor();
            taskParam.delay1 = getFuncNumber(2, "Delay duration");
            taskParam.ITI = getFuncNumber(2, "ITI duration");
            waterLen = getFuncNumber(1, "Water fold?") * waterLen;
            int sessNum = getFuncNumber(2, "Session number?");
            zxLaserSessions_G2(20, 20, sessNum);
            break;
        }
        case 27:
        {
            int dpadrOdors[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 13};
            int i;
            for (i = 0; i < (sizeof (dpadrOdors) / sizeof (int)); i++) {
                testOneValve(dpadrOdors[i], 10, 1);
            }
            for (i = 0; i < 20; i++) {
                setWaterPortOpen(1);
                wait_ms(waterLen);
                setWaterPortOpen(0);
                wait_ms(500 - waterLen);
            }
            laser_G2.on = 1;
            break;
        }
        case 28:
            bleedWater();
            break;
        case 29:
            testVolume();
            break;
        case 30:
            setWaterLen();
            break;
        case 31:
            testLaser();
            break;

            //        case 32:
            //        {
            //            splash_G2("ODPA R_D SHAP", "REPEAT");
            //            laser_G2.laserSessionType = LASER_SESS_UNDEFINED;
            //            taskType_G2 = ODPA_RD_SHAPING_TASK;
            //            taskParam.teaching = 1;
            //            taskParam.falsePunish = 0;
            //            taskParam.pairs1Count = 2;
            //            addAllOdor();
            //            taskParam.delay1 = 5;
            //            taskParam.ITI = 8;
            //            int sessNum = getFuncNumber(2, "Session Number?");
            //            zxLaserSessions_G2(20, 100, sessNum);
            //            break;
            //        }
        case 33:
        {
            splash_G2("ODPA", "");
            int noLaser = getFuncNumber(1, "No Laser?");
            laser_G2.laserSessionType = noLaser ? LASER_NO_TRIAL : LASER_EVERY_TRIAL;
            taskType_G2 = ODPA_SHAPING_TASK;
            taskParam.teaching = 1;
            taskParam.falsePunish = getFuncNumber(1, "False Punish 2/0");
            taskParam.pairs1Count = 2;
            taskParam.respCount = 0;
            addAllOdor();
            taskParam.delay1 = getFuncNumber(2, "Delay duration");
            taskParam.ITI = getFuncNumber(2, "ITI duration");
            waterLen = getFuncNumber(1, "Water fold?") * waterLen;
            int sessNum = getFuncNumber(2, "Session number?");
            zxLaserSessions_G2(20, 20, sessNum);
            break;
        }
        case 34:
        {
            splash_G2("ODPA", "");
            int noLaser = getFuncNumber(1, "No Laser?");
            laser_G2.laserSessionType = noLaser ? LASER_NO_TRIAL : LASER_CATCH_TRIAL;
            laser_G2.laserTrialType = laserDuringDelayChR2;
            taskType_G2 = ODPA_TASK;
            taskParam.falsePunish = getFuncNumber(1, "False Punish 2/0");
            taskParam.pairs1Count = 2;
            taskParam.respCount = 0;
            addAllOdor();
            taskParam.delay1 = getFuncNumber(2, "Delay duration");
            taskParam.ITI = getFuncNumber(2, "ITI duration");
            waterLen = getFuncNumber(1, "Water fold?") * waterLen;
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
            taskParam.pairs1Count = 4;
            taskParam.respCount = 0;
            addAllOdor();
            taskParam.delay1 = getFuncNumber(2, "Delay duration");
            taskParam.ITI = getFuncNumber(2, "ITI duration");
            waterLen = getFuncNumber(1, "Water fold?") * waterLen;
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
            taskParam.pairs1Count = 2;
            addAllOdor();
            taskParam.delay1 = getFuncNumber(2, "Delay duration");
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
            taskParam.pairs1Count = 6;
            taskParam.minBlock = 6;
            addAllOdor();
            taskParam.delay1 = getFuncNumber(2, "Delay duration");
            taskParam.ITI = getFuncNumber(2, "ITI duration");
            waterLen = getFuncNumber(1, "Water fold?") * waterLen;
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
            taskParam.pairs1Count = 6;
            taskParam.minBlock = 6;
            taskParam.respCount = 0;
            addAllOdor();
            taskParam.delay1 = getFuncNumber(2, "Delay duration");
            taskParam.ITI = getFuncNumber(2, "ITI duration");
            waterLen = getFuncNumber(1, "Water fold?") * waterLen;
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
            taskParam.pairs1Count = 6;
            taskParam.minBlock = 6;
            addAllOdor();
            taskParam.delay1 = getFuncNumber(2, "Delay duration");
            taskParam.ITI = getFuncNumber(2, "ITI duration");
            waterLen = getFuncNumber(1, "Water fold?") * waterLen;
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
            taskParam.pairs1Count = 6;
            taskParam.pairs2Count = 1;
            taskParam.minBlock = 6;
            addAllOdor();
            taskParam.delay1 = 12;
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
            taskParam.pairs1Count = 6;
            taskParam.pairs2Count = 2;
            taskParam.minBlock = 6;
            addAllOdor();
            taskParam.delay1 = 12;
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
            taskParam.pairs1Count = 6;
            taskParam.pairs2Count = 2;
            taskParam.minBlock = 6;
            addAllOdor();
            taskParam.delay1 = 12;
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

            //        DELAY_DISTRACTOR
        default:
        {
            int i;
            for (i = n; i < n + 4; i++)
                testOneValve(i, 8, 10);
        }
            break;

    }
    free(taskParam.sample1s);
    free(taskParam.test1s);
}

void testValveFast(int board, int valve, int keep) {
    LCDclear();
    LCDhome();
    LCD_Write_Str("BOARD   VALVE");

    lcdWriteNumber_G2(board, 6, 0);
    lcdWriteNumber_G2(valve, 6, 1);

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
        serialSend(SpIO, 0);
        BNC_1 = 0;
        wait_ms(iti * 1000 - preCharge);

    }
}

void readADCData(void) {
    while (1) {
        volatile int temp = adcdata;
        int highByte = temp / 100;
        int lowByte = temp % 100;

        serialSend(23, highByte);
        serialSend(24, lowByte);
        wait_ms(50);

    }
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

void feedWaterFast_G2() {

    lick_G2.LCount = 0;
    sendLargeValue(lickThresh);
    //    lick_G2.RCount = 0;
    unsigned int waterCount = 0;
    unsigned int totalLickCount = 0;


    LCDclear();
    LCDhome();
    LCD_Write_Str("Total Lick");

    timerCounterI = 1000;
    while (1) {
        if (lick_G2.LCount > totalLickCount) {
            if (timerCounterI >= 500) {
                setWaterPortOpen(1);
                timerCounterI = 0;
                lcdWriteNumber_G2(++waterCount, 12, 1);
            }
            totalLickCount = lick_G2.LCount;
            lcdWriteNumber_G2(totalLickCount, 11, 0);
        }
        if (timerCounterI >= waterLen) {
            setWaterPortOpen(0);
        }
    }

}

void testNSetThres() {
    unsigned long startTime = millisCounter;
    while (millisCounter < startTime + 10000ul) {
        int sum = 0;
        int i = 0;
        for (; i < 15; i++) {
            sum += adcdata;
        }
        int val = sum / 15;
        sendLargeValue(val);
        wait_ms(500);
    }
    sendLargeValue(lickThresh);
    int newThres = getFuncNumber(3, "New Lick Thres?");
    sendLargeValue(newThres);
    write_eeprom_G2(EEP_LICK_THRESHOLD, newThres >> 2);
    serialSend(61, 0);
    asm("Reset");
}

void stim_G2(int place, int odorPort, int laserType) {
    set4076_4bit(odorPort > 15 ? odorPort - 16 : odorPort);
    if (place == 1 || place == 2 || place == 6) {
        muxDis(odorPort < 16 ? (~1) : (~4));
        waitTaskTimer(500);
    }
    if (place == 3) {
        serialSend(SpIO, odorPort);
        muxDis(odorPort < 16 ? (~1) : (~4));
    } else {
        switch (place) {
            case 1:
                assertLaser_G2(laserType, atS1Beginning);
                break;
            case 2:
                assertLaser_G2(laserType, atSecondOdorBeginning);
                break;
            case 4:
                assertLaser_G2(laserType, atResponseCueBeginning);
            case 6:
                assertLaser_G2(laserType, atPreDualTask);
                break;
                //            case 7:
                //                break; //TODO fix laser
        }

        muxDis(odorPort < 16 ? (~3) : (~0x0c));
        int stimSend;
        switch (place) {
            case 1:
            case 2:
                if (isLikeOdorClassA(odorPort)) {
                    stimSend = 9;
                    BNC_5 = 1;
                } else {
                    stimSend = 10;
                    BNC_6 = 1;
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
        muxDis(odorPort < 16 ? (~2) : (~8));
        waitTaskTimer(200);
        muxDis(0x0f);
        BNC_5 = 0;
        Nop();
        Nop();
        BNC_6 = 0;
        Nop();
        Nop();
        serialSend(stimSend, 0);
        LCDsetCursor(3, 0);
        switch (place) {
            case 1:
                assertLaser_G2(laserType, atS1End);
                LCD_Write_Char('d');
                if (taskParam.sample1Length < 1000u)
                    waitTaskTimer(1000u - taskParam.sample1Length);
                break;
            case 2:
                assertLaser_G2(laserType, atSecondOdorEnd);
                LCD_Write_Char('D');
                if (taskParam.test1Length < 1000u)
                    waitTaskTimer(1000u - taskParam.test1Length);
                break;
            case 4:
                assertLaser_G2(laserType, atResponseCueEnd);
                LCD_Write_Char('R');
                break;
                //TODO fix laser
            case 6:
                //                assertLaser_G2(laserType, atResponseCueEnd);
                LCD_Write_Char('d');
                break;
            case 7:
                //                assertLaser_G2(laserType, atResponseCueEnd);
                LCD_Write_Char('d');
                break;
        }
    }
}

static void processHit_G2(int id, int ratio) {
    serialSend(22, 1);
    if (ratio > 0 || (ratio == 0 && rand() % 2)) {
        serialSend(SpDebugInfo, 121);
        setWaterPortOpen(1);
        waitTaskTimer(waterLen);
        setWaterPortOpen(0);
    } else {
        serialSend(SpDebugInfo, 120);
        waitTaskTimer(waterLen);
    }
    if (ratio == 2) {
        serialSend(SpDebugInfo, 122);
        waitTaskTimer(500);
        setWaterPortOpen(1);
        waitTaskTimer(waterLen);
        setWaterPortOpen(0);
    }

    currentMiss = 0;
    serialSend(SpHit, id);
    lcdWriteNumber_G2(++hit, 5, 0);
}

static void processFalse_G2(int id) {
    currentMiss = 0;
    serialSend(SpFalseAlarm, id);
    lcdWriteNumber_G2(++falseAlarm, 5, 1);
    stim_G2(5, taskParam.correctionCue, LASER_OFF);
    if (id != OUTCOM_DUAL) {
        waitTaskTimer(4000);
    }
}

static void processMiss_G2(int id) {
    currentMiss++;
    serialSend(SpMiss, id);
    lcdWriteNumber_G2(++miss, 9, 0);
}

static int waterNResult_G2(int sample, int test, int id, int rewardWindow) {
    int rtn = 0;

    lick_G2.portSide = 0;

    switch (taskType_G2) {
        case GONOGO_TASK:
            for (timerCounterI = 0; timerCounterI < rewardWindow && !lick_G2.portSide; lick_G2.portSide = lick_G2.stable);
            taskTimeCounter = millisCounter;
            /////Reward
            if (!lick_G2.portSide) {
                if (!isLikeOdorClassA(sample)) {
                    serialSend(SpCorrectRejection, OUTCOME_WMDelay);
                    lcdWriteNumber_G2(++correctRejection, 9, 1);
                } else {
                    processMiss_G2(OUTCOME_WMDelay);
                    if (taskParam.teaching == 1 && (rand() % 3) == 0) {
                        serialSend(SpLickFreq, 1);
                        setWaterPortOpen(1);
                        serialSend(SpWater, OUTCOME_WMDelay);
                        waitTaskTimer(waterLen);
                        setWaterPortOpen(0);
                    }
                }
            } else if (!isLikeOdorClassA(sample)) {
                processFalse_G2(OUTCOME_WMDelay);
            } else {
                processHit_G2(OUTCOME_WMDelay, 1);
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
            for (timerCounterI = 0; timerCounterI < rewardWindow && !lick_G2.portSide; lick_G2.portSide = lick_G2.stable);
            taskTimeCounter = millisCounter;
            /////Reward
            if (!lick_G2.portSide) {
                if (isLikeOdorClassA(sample) == isLikeOdorClassA(test)) {
                    serialSend(SpCorrectRejection, id);
                    lcdWriteNumber_G2(++correctRejection, 9, 1);
                    rtn = SpCorrectRejection;
                } else {
                    processMiss_G2(id);
                    rtn = SpMiss;
                    if ((taskParam.teaching
                            //                            taskType_G2 == SHAPING_TASK
                            //                            || taskType_G2 == ODPA_SHAPING_TASK
                            //                            || taskType_G2 == DUAL_TASK_LEARNING
                            //                            || taskType_G2 == DNMS_DUAL_TASK_LEARNING
                            //                            || taskType_G2 == ODPA_RD_SHAPING_TASK
                            //                            || taskType_G2 == Seq2AFC_TEACH
                            //                            ||taskType_G2==
                            ) && ((rand() % 3) == 0)) {
                        serialSend(22, 1);
                        setWaterPortOpen(1);
                        serialSend(SpWater, 1);
                        waitTaskTimer(waterLen);
                        setWaterPortOpen(0);
                    }
                }
            } else if (isLikeOdorClassA(sample) == isLikeOdorClassA(test)) {
                processFalse_G2(id);
                rtn = SpFalseAlarm;
            } else {
                if (taskType_G2 == Seq2AFC_TASK) {
                    if (sample == taskParam.sample1s[0] || sample == taskParam.sample1s[1]) {
                        processHit_G2(id, 1);
                    } else if (sample == taskParam.sample1s[2] || sample == taskParam.sample1s[3])
                        processHit_G2(id, 2);
                    else {
                        processHit_G2(id, 0);
                    }
                } else {
                    processHit_G2(id, 1);
                }
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
        muxDis(test2 < 16 ? (~1) : (~4)); // 1* 2 4* 8
        licked |= waitTaskTimer(500); //delay+3.5s
        if (!licked) { //test and reward
            stim_G2(7, test2, laserType);
            waterNResult_G2(sample2, test2, OUTCOM_DUAL, 500); //delay+5s
        } else {//abortTrial
            muxDis(0x0f);
            serialSend(SpAbortTrial, OUTCOM_DUAL);
            LCDsetCursor(3, 0);
            LCD_Write_Char('A');
            waitTaskTimer(1500u);
        }

    }




}

int delayedRspsDelay(int laserType) {
    if (taskParam.respCueLength >= 200) {
        int delayLick = waitTaskTimer(1000u);
        if (taskParam.delay1 > 2) {
            assertLaser_G2(laserType, atDelay1SecIn);
            delayLick |= waitTaskTimer(taskParam.delay1 * 1000u - 2000u);
            assertLaser_G2(laserType, atDelayLastSecBegin);
            delayLick |= waitTaskTimer(500u);
        }
        stim_G2(3, taskParam.respCue[0], laserType);
        delayLick |= waitTaskTimer(500u);
        LCDsetCursor(3, 0);
        if (delayLick) {
            muxDis(0x0f);
            serialSend(SpAbortTrial, OUTCOME_WMDelay);
            LCD_Write_Char('A');
            stim_G2(5, taskParam.correctionCue, LASER_OFF);
            abortTrial++;
            return 0;
        } else {
            LCD_Write_Char('R');
            assertLaser_G2(laserType, atRewardBeginning);
            stim_G2(4, taskParam.respCue[0], laserType);
            return 1;
        }
    } else {
        return 0;
    }
}

static void zxLaserTrial_G2(int s1, int t1, int s2, int t2, int laserType) {
    taskTimeCounter = millisCounter;
    serialSend(Sptrialtype, laserType);
    serialSend(Splaser, (laserType != LASER_OFF));
    assertLaser_G2(laserType, at4SecBeforeS1);
    waitTaskTimer(1000u);
    assertLaser_G2(laserType, at3SecBeforeS1);
    waitTaskTimer(2000u);
    assertLaser_G2(laserType, at1SecBeforeS1);
    waitTaskTimer(500u);
    assertLaser_G2(laserType, at500msBeforeS1);
    //    waitTimerJ_G2(500u);

    /////////////////////////////////////////////////
    stim_G2(1, s1, laserType);
    ////////////////////////////////////////////////


    switch (taskType_G2) {
            //Do nothing during Go Nogo Tasks
        case GONOGO_TASK:

            //            if (taskParam.delay1 > 2) {
            //                assertLaser_G2(laserType, atDelayBegin);
            //                waitTaskTimer(500u);
            //                assertLaser_G2(laserType, atDelay500MsIn);
            //                waitTaskTimer(500u);
            //                assertLaser_G2(laserType, atDelay1SecIn); ////////////////1Sec////////////
            //                waitTaskTimer(taskParam.delay1 * 1000u - 2000u);
            //                assertLaser_G2(laserType, atDelayLastSecBegin);
            //                waitTaskTimer(1000u);
            //                assertLaser_G2(laserType, atSecondOdorEnd);
            //            }
            break; //next line just before waterNresult


        default:////////////////////////////////////DELAY/////////////////////
            if (taskParam.delay1 == 0) {
                waitTaskTimer(200u); ////////////////NO DELAY////////////////////
            } else {
                if (taskParam.delay1 <= 4) {
                    waitTaskTimer(taskParam.delay1 * 1000u - 1000u);
                } else
                    /*/////////////////////////////////////////////////
                     * ////////DISTRACTOR//////////////////////////////
                     * //////////////////////////////////////////////*/
                    if (taskParam.pairs2Count != 0) {
                    /////////DISTRACTOR TIMELINE @Delay+0/////////////
                    waitTaskTimer(500u); //@D+500ms
                    // assertLaser_G2(type, atPreDualTask); //@2s
                    dual_task_D_R(laserType, s2, t2); //Delay+5000ms
                    // waitTaskTimer_G2(1500u);
                    assertLaser_G2(laserType, atPostDualTask);
                    if (taskParam.delay1 >= 8u) {
                        waitTaskTimer((unsigned int) taskParam.delay1 * 1000 - 6000);
                    }
                    // at test odor -1000 ms
                } else {

                    assertLaser_G2(laserType, atDelayBegin);
                    waitTaskTimer(500u);
                    assertLaser_G2(laserType, atDelay500MsIn);
                    waitTaskTimer(500u);
                    assertLaser_G2(laserType, atDelay1SecIn); ////////////////1Sec////////////


                    waitTaskTimer(1000u);
                    assertLaser_G2(laserType, atDelay2SecIn); /////////////2Sec/////////////


                    waitTaskTimer(taskParam.delay1 * 500u - 2000u);
                    //                    }
                    assertLaser_G2(laserType, atDelayMiddle); //13@6.5
                    if (taskParam.delay1 >= 12) {
                        waitTaskTimer(2000u); //13@7
                        assertLaser_G2(laserType, atDelayMid2Sec);
                        waitTaskTimer(500u); //distractor@9s//13@9
                        assertLaser_G2(laserType, atDelayMid2_5Sec);
                        waitTaskTimer(500u); //distractor@9.5s//13@9.5
                        assertLaser_G2(laserType, atDelayMid3Sec);
                        waitTaskTimer((taskParam.delay1 - 11)*500u); //13@10
                    } else {
                        waitTaskTimer(taskParam.delay1 * 500u - 2500u);
                    }
                    assertLaser_G2(laserType, atDelayLast2_5SecBegin);
                    waitTaskTimer(500u); //13@10.5
                    assertLaser_G2(laserType, atDelayLast2SecBegin); //////////////-2 Sec//////////////////////

                    waitTaskTimer(500u);
                    assertLaser_G2(laserType, atDelayLast1_5SecBegin);
                    waitTaskTimer(500u);

                }
                assertLaser_G2(laserType, atDelayLastSecBegin); /////////////////////////-1 Sec////////////////
                waitTaskTimer(500u);
                assertLaser_G2(laserType, atDelayLast500mSBegin);
                //            waitTimerJ(300u);
                //            assertLaser(type, atDelayLast200mSBegin);
                //            waitTimerJ(200u);
                //                waitTimerJ_G2(500u);
            }

            ///////////-Second odor-/////////////////
            stim_G2(2, t1, laserType);
            //////////////////////////////////////////
            break;
    }
    int resultRtn = 0;
    switch (taskParam.respCount) {
        case 0:
            //            waitTaskTimer(1000u);
            LCDsetCursor(3, 0);
            LCD_Write_Char('R');
            resultRtn = waterNResult_G2(s1, t1, OUTCOME_WMDelay, 1000);
            //DPA 2AFC HERE
            if ((taskType_G2 == Seq2AFC_TASK)
                    && (resultRtn == SpCorrectRejection || resultRtn == SpMiss)) {
                int t2 = (t1 == taskParam.test1s[0]) ? taskParam.test1s[1] : taskParam.test1s[0];
                waitTaskTimer(1000u);
                stim_G2(3, t2, laserType);
                waitTaskTimer(500u);
                stim_G2(4, t2, laserType);
                resultRtn = waterNResult_G2(s1, t2, OUTCOM_2AFC, 1000);
            }
            ///////////////
            break;
        case 1:
            if (delayedRspsDelay(laserType))
                resultRtn = waterNResult_G2(s1, t1, OUTCOME_WMDelay, 500);
            break;
            //case 2:
            //    seq2AFCResult(s1, laserType);
            //   break;

    }
    //    waitTaskTimer(1000u); //water time sync
    // Total Trials
    int totalTrials = hit + correctRejection + miss + falseAlarm + abortTrial;
    lcdWriteNumber_G2(totalTrials, 13, 1);
    // Discrimination rate
    if (hit + correctRejection > 0) {

        correctRatio = 100 * (hit + correctRejection) / totalTrials;
        correctRatio = correctRatio > 99 ? 99 : correctRatio;
        lcdWriteNumber_G2(correctRatio, 13, 0);
    }
    LCDsetCursor(3, 0);
    LCD_Write_Char('I');

    ///--ITI1---///
    if (resultRtn == SpFalseAlarm) {
        taskParam.falsePunish |= 1;
        correctionRepeatCount++;
    } else {
        taskParam.falsePunish &= 0xFFFE;
    }
    if (taskParam.ITI >= 4u) {
        unsigned int trialITI = taskParam.ITI - 4u;
        while (trialITI > 60u) {
            waitTaskTimer(60u * 1000u);
            trialITI -= 60u;
        }
        waitTaskTimer(trialITI * 1000u); //another 4000 is at the beginning of the trials.
    }
    serialSend(SpITI, 0);

    waitTrial_G2();
}

void setWaterLen() {
    sendLargeValue(waterLen);
    int newWaterLen = getFuncNumber(3, "New water len?");
    sendLargeValue(waterLen);
    write_eeprom_G2(EEP_WATER_LEN_MS, newWaterLen);
    serialSend(61, 0);
    asm("Reset");
}

void zxLaserSessions_G2(int trialsPerSession, int missLimit, int totalSession) {


    //    wait_ms(1000);
    int currentTrial = 0;
    int currentSession = 0;
    int laserOnType = laser_G2.laserTrialType;
    unsigned int* shuffledList = malloc(taskParam.minBlock * sizeof (unsigned int));
    serialSend(SpTaskType, taskType_G2);
    while ((currentMiss < missLimit) && (currentSession++ < totalSession)) {
        serialSend(SpSess, 1);

        splash_G2("    H___M___ __%", "S__ F___C___t___");

        lcdWriteNumber_G2(currentSession, 1, 1);
        hit = miss = falseAlarm = correctRejection = abortTrial = 0;
        int sample1, test1;
        int sample2 = 0, test2 = 0;
        for (currentTrial = 0; currentTrial < trialsPerSession && currentMiss < missLimit;) {
            shuffleArray_G2(shuffledList, taskParam.minBlock);
            int idxInMinBlock;
            for (idxInMinBlock = 0; idxInMinBlock < taskParam.minBlock && currentMiss < missLimit; idxInMinBlock++) {
                int shuffledMinBlock = shuffledList[idxInMinBlock];
                switch (taskType_G2) {

                    case DNMS_TASK:
                        sample1 = (shuffledMinBlock == 0 || shuffledMinBlock == 2) ? taskParam.sample1s[0] : taskParam.sample1s[1];
                        test1 = (shuffledMinBlock == 1 || shuffledMinBlock == 2) ? taskParam.test1s[0] : taskParam.test1s[1];
                        break;
                        //                    case SHAPING_TASK:
                        //                        sample1 = (shuffledMinBlock == 0 || shuffledMinBlock == 2) ? taskParam.sample1s[0] : taskParam.sample1s[1];
                        //                        test1 = (sample1 == taskParam.sample1s[0]) ? taskParam.test1s[0] : taskParam.test1s[1];
                        //                        break;
                    case GONOGO_TASK:
                        //                    case Seq2AFC_TEACH:
                        //                    case GONOGO_LR_TASK:
                        sample1 = (shuffledMinBlock == 0 || shuffledMinBlock == 2) ? taskParam.sample1s[0] : taskParam.sample1s[1];
                        test1 = 0;
                        break;

                    case ODPA_TASK:
                        sample1 = (shuffledMinBlock == 0 || shuffledMinBlock == 2) ? taskParam.sample1s[0] : taskParam.sample1s[1];
                        test1 = (shuffledMinBlock == 1 || shuffledMinBlock == 2) ? taskParam.test1s[0] : taskParam.test1s[1];
                        break;

                    case ODPA_SHAPING_TASK:
                        sample1 = (shuffledMinBlock == 0 || shuffledMinBlock == 2) ? taskParam.sample1s[0] : taskParam.sample1s[1];
                        test1 = (sample1 == taskParam.sample1s[0]) ? taskParam.test1s[1] : taskParam.test1s[0];
                        break;
                    case ODPA_RD_SHAPING_TASK:
                        sample1 = (shuffledMinBlock == 0 || shuffledMinBlock == 2) ? taskParam.sample1s[0] : taskParam.sample1s[1];
                        test1 = (sample1 == taskParam.sample1s[0]) ? taskParam.test1s[1] : taskParam.test1s[0];
                        if (currentTrial > 15) {
                            laser_G2.laserTrialType = laserDuringDelayChR2;
                        } else {
                            laser_G2.laserTrialType = LASER_OFF;
                        }
                        break;

                    case ODPA_RD_TASK:
                        //                    case Seq2AFC_TEACH:
                    case Seq2AFC_TASK:
                        if ((taskParam.falsePunish & 0x03) != 0x03 || correctionRepeatCount > 2) {
                            switch (taskParam.pairs1Count) {
                                case 2:
                                    sample1 = (shuffledMinBlock == 0 || shuffledMinBlock == 2) ? taskParam.sample1s[0] : taskParam.sample1s[1];
                                    test1 = (shuffledMinBlock == 1 || shuffledMinBlock == 2) ? taskParam.test1s[0] : taskParam.test1s[1];
                                    break;
                                case 4:
                                    sample1 = taskParam.sample1s[shuffledMinBlock];
                                    if (((shuffledMinBlock == 1 || shuffledMinBlock == 2) && (currentTrial % 8) < 4)
                                            || ((shuffledMinBlock == 0 || shuffledMinBlock == 3) && (currentTrial % 8) > 3)) {
                                        test1 = taskParam.test1s[0];
                                    } else {
                                        test1 = taskParam.test1s[1];
                                    }
                                    break;
                                case 6:
                                    sample1 = taskParam.sample1s[shuffledMinBlock];
                                    if (((shuffledMinBlock < 3) && (currentTrial % 12) < 6)
                                            || ((shuffledMinBlock >= 3) && (currentTrial % 12) >= 6)) {
                                        test1 = taskParam.test1s[0];
                                    } else {
                                        test1 = taskParam.test1s[1];
                                    }
                                    break;

                            }
                            correctionRepeatCount = 0;
                        }

                        break;

                }
                LCDsetCursor(0, 0);
                LCD_Write_Char(odorTypes_G2[sample1]);
                LCD_Write_Char(odorTypes_G2[test1]);

                switch (taskParam.pairs2Count) {
                    case 1:
                        sample2 = taskParam.sample2s[0];
                        break;
                    case 2:
                        sample2 = (rand()&1) ? taskParam.sample2s[0] : taskParam.sample2s[1];
                        test2 = taskParam.test2s[0];
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
                    case LASER_CATCH_TRIAL:
                    {
                        int toCatch = trialsPerSession * 2 / 10;
                        laser_G2.laserTrialType = ((trialsPerSession - currentTrial) <= toCatch) ? laserOnType : LASER_OFF;
                        break;
                    }

                    case LASER_LR_EACH_QUARTER:
                        laser_G2.side = isLikeOdorClassA(sample1) ? 1 : 2;
                    case LASER_EACH_QUARTER:
                        switch (currentTrial % 5) {
                            case 0:
                                laser_G2.laserTrialType = LASER_OFF;
                                break;
                            case 1:
                                laser_G2.laserTrialType = laserDuring1Quarter;
                                break;
                            case 2:
                                laser_G2.laserTrialType = laserDuring2Quarter;
                                break;
                            case 3:
                                laser_G2.laserTrialType = laserDuring3Quarter;
                                break;
                            case 4:
                                laser_G2.laserTrialType = laserDuring4Quarter;
                                break;
                        }
                        break;


                    case LASER_12s_LR_EACH_QUARTER:
                        laser_G2.side = isLikeOdorClassA(sample1) ? 1 : 2;
                    case LASER_12s_EACH_QUARTER:
                        switch (currentTrial % 5) {
                            case 0:
                                laser_G2.laserTrialType = LASER_OFF;
                                break;
                            case 1:
                                laser_G2.laserTrialType = laserDuring12s1Quarter;
                                break;
                            case 2:
                                laser_G2.laserTrialType = laserDuring12s2Quarter;
                                break;
                            case 3:
                                laser_G2.laserTrialType = laserDuring12s3Quarter;
                                break;
                            case 4:
                                laser_G2.laserTrialType = laserDuring12s4Quarter;
                                break;
                        }
                        break;


                    case LASER_VARY_LENGTH:
                        switch (currentTrial % 5) {
                            case 0:
                                laser_G2.laserTrialType = LASER_OFF;
                                break;
                            case 1:
                                laser_G2.laserTrialType = laser4sRamp;
                                break;
                            case 2:
                                laser_G2.laserTrialType = laser2sRamp;
                                break;
                            case 3:
                                laser_G2.laserTrialType = laser1sRamp;
                                break;
                            case 4:
                                laser_G2.laserTrialType = laser_5sRamp;
                                break;
                        }
                        break;


                    case LASER_LR_EVERYTRIAL:
                        laser_G2.side = isLikeOdorClassA(sample1) ? 1 : 2;
                        break;

                    case LASER_LR_EVERY_OTHER_TRIAL:
                        laser_G2.side = isLikeOdorClassA(sample1) ? 1 : 2;

                        laser_G2.laserTrialType = (currentTrial % 2) == 0 ? LASER_OFF : laserOnType;
                        break;

                    case LASER_INCONGRUENT_CATCH_TRIAL:
                        if ((currentTrial > 3 && currentTrial < 8 && isLikeOdorClassA(sample1) && isLikeOdorClassA(test1))
                                || (currentTrial > 7 && currentTrial < 12 && isLikeOdorClassA(sample1)&& !isLikeOdorClassA(test1))
                                || (currentTrial > 11 && currentTrial < 16 && !isLikeOdorClassA(sample1) && isLikeOdorClassA(test1))
                                || (currentTrial > 15 && currentTrial < 20 && !isLikeOdorClassA(sample1) && !isLikeOdorClassA(test1))) {
                            laser_G2.side = isLikeOdorClassA(sample1) ? 2 : 1;
                        } else {
                            laser_G2.side = isLikeOdorClassA(sample1) ? 1 : 2;
                        }
                        break;
                    case LASER_DUAL_TASK_ODAP_ON_OFF:
                        laser_G2.laserTrialType = (shuffledMinBlock < 2) ? LASER_OFF : laserCoverDistractor;
                        break;
                    case LASER_OTHER_BLOCK:
                        if (rand()&0x1)
                            laser_G2.laserTrialType = currentTrial < (trialsPerSession / 2) ? LASER_OFF : laserOnType;
                        else
                            laser_G2.laserTrialType = currentTrial < (trialsPerSession / 2) ? laserOnType : LASER_OFF;
                        break;
                }
                zxLaserTrial_G2(sample1, test1, sample2, test2, laser_G2.laserTrialType);
                currentTrial++;
            }
        }
        //        serialSend()
        serialSend(SpSess, 0);
        sendChart(correctRatio, 0);
        sendChart(miss * 100 / (miss + hit), 1);
    }
    serialSend(SpTrain, 0); // send it's the end
    u2Received = -1;
    free(shuffledList);
    free(taskParam.sample1s);
    free(taskParam.test1s);
    free(taskParam.respCue);
}

void testLaser() {
    int i = 0;
    while (1) {
        i ^= 1;
        laser_G2.on = i;
        getFuncNumber(1, "Toggle Laser");
    }
}

void turnOnLaser_G2(int type) {
    laser_G2.on = 1;
    LCDsetCursor(3, 0);
    LCD_Write_Char('L');
    serialSend(SpLaserSwitch, 1);
}

void turnOffLaser_G2() {
    laser_G2.on = 0;
    LCDsetCursor(3, 0);
    LCD_Write_Char('.');
    serialSend(SpLaserSwitch, 0);
}
