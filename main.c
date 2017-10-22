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
void testOneValve(int n);
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

int main(void) {
    initPorts();
    initTMR1();
    initUART2();
    initI2C();
    LCD_Init();
    initADC();
    while (1) {
        callFunc(getFuncNumber(2, "Main Func?"));
    }

    StopI2C();
    CloseI2C();
    return 0;
}

static int isLikeOdorA_G2(int odor) {
    if (odor == 3 || odor == 7) return 1; //B,R,H
    return 0;
}

void addAllOdor() {
    int odorPairs = taskParam.pairs1Count;
    taskParam.sample1s = malloc(odorPairs * sizeof (int));
    taskParam.test1s = malloc(odorPairs * sizeof (int));
    int i;
    int odor;
    for (i = 0; i < odorPairs; i++) {
        odor = getFuncNumber(2, "Add an sample");
        taskParam.sample1s[i] = odor;
        odor = getFuncNumber(2, "Add an test");
        taskParam.test1s[i] = odor;
    }
    if (taskParam.respCueLength >= 200) {
        odor = getFuncNumber(2, "Add an rsps cue");
        taskParam.respCue = odor;
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
            testPorts();
            break;
        case 24:
            testNSetThres();
            break;
        case 25:
            feedWaterFast_G2(waterLen);
        case 26:
        {
            splash_G2("ODPA R_D", "");
            highLevelShuffleLength_G2 = 20;
            int noLaser = getFuncNumber(1, "No Laser?");
            laser_G2.laserSessionType = noLaser ? LASER_NO_TRIAL : LASER_EVERY_TRIAL;
            taskType_G2 = ODPA_RD_CATCH_LASER_TASK;
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
            int dpadrOdors[] = {3, 2, 7, 1, 13};
            int i;
            for (i = 0; i < 5; i++) {
                testOneValve(dpadrOdors[i]);
            }
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

        case 32:
        {
            splash_G2("ODPA R_D", "REPEAT");
            highLevelShuffleLength_G2 = 20;
            laser_G2.laserSessionType = LASER_SESS_UNDEFINED;
            taskType_G2 = ODPA_RD_SHAPING_TASK;
            taskParam.falsePunish = 0;
            taskParam.pairs1Count = 2;
            addAllOdor();
            taskParam.delay1 = 5;
            taskParam.ITI = 8;
            int sessNum = getFuncNumber(2, "Session Number?");
            zxLaserSessions_G2(20, 100, sessNum);
            break;
        }
        case 33:
        {
            splash_G2("ODPA", "");
            highLevelShuffleLength_G2 = 20;
            int noLaser = getFuncNumber(1, "No Laser?");
            laser_G2.laserSessionType = noLaser ? LASER_NO_TRIAL : LASER_EVERY_TRIAL;
            taskType_G2 = ODPA_SHAPING_TASK;
            taskParam.falsePunish = getFuncNumber(1, "False Punish 2/0");
            taskParam.pairs1Count = 2;
            taskParam.respCueLength = 0;
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
            highLevelShuffleLength_G2 = 20;
            int noLaser = getFuncNumber(1, "No Laser?");
            laser_G2.laserSessionType = noLaser ? LASER_NO_TRIAL : LASER_EVERY_TRIAL;
            taskType_G2 = ODPA_TASK;
            taskParam.falsePunish = getFuncNumber(1, "False Punish 2/0");
            taskParam.pairs1Count = 2;
            taskParam.respCueLength = 0;
            addAllOdor();
            taskParam.delay1 = getFuncNumber(2, "Delay duration");
            taskParam.ITI = getFuncNumber(2, "ITI duration");
            waterLen = getFuncNumber(1, "Water fold?") * waterLen;
            int sessNum = getFuncNumber(2, "Session number?");
            zxLaserSessions_G2(20, 20, sessNum);
            break;
        }
        default:
        {
            int i;
            for (i = n; i < n + 4; i++)
                testOneValve(i);
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

void testOneValve(int valve) {
    const int preCharge = 500;

    int repeat = 10;
    int iti = 8;
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
        protectedSerialSend_G2(3, rpt + 1);
        //        for (valve = 0; valve < 20; valve++) {
        lcdWriteNumber_G2(valve, 6, 0);
        protectedSerialSend_G2(2, valve);
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

    }
}

void readADCData(void) {
    while (1) {
        volatile int temp = adcdata;
        int highByte = temp / 100;
        int lowByte = temp % 100;

        protectedSerialSend_G2(23, highByte);
        protectedSerialSend_G2(24, lowByte);
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
    protectedSerialSend_G2(61, 0);
    asm("Reset");
}

void stim_G2(int place, int odorPort, int type) {
    if (place == 1 || place == 2) {
        set4076_4bit(odorPort > 15 ? odorPort - 16 : odorPort);
        muxDis(odorPort < 16 ? (~1) : (~4));
        wait_ms(500);
    }
    if (place == 3) {
        set4076_4bit(odorPort > 15 ? odorPort - 16 : odorPort);
        muxDis(odorPort < 16 ? (~1) : (~4));
    } else {
        switch (place) {
            case 1:
                assertLaser_G2(type, atS1Beginning);
                break;
            case 2:
                assertLaser_G2(type, atSecondOdorBeginning);
                break;
            case 4:
                assertLaser_G2(type, atResponseCueBeginning);
                break;
        }

        muxDis(odorPort < 16 ? (~3) : (~0x0c));
        int stimSend;
        switch (place) {
            case 1:
            case 2:
                if (isLikeOdorA_G2(odorPort)) {
                    stimSend = 9;
                    BNC_2 = 1;
                } else {
                    stimSend = 10;
                    BNC_3 = 1;
                }
                protectedSerialSend_G2(stimSend, odorPort);
                break;
            case 4:
                stimSend = SpResponseCue;
                protectedSerialSend_G2(stimSend, odorPort);
                break;
            case 5:
                stimSend = SpCorrectionCue;
                protectedSerialSend_G2(stimSend, 100 + odorPort);
                break;
        }
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
        }

        muxDis(odorPort < 16 ? (~2) : (~8));
        waitTaskTimer(200);
        muxDis(0x0f);
        BNC_2 = 0;
        Nop();
        Nop();
        BNC_3 = 0;
        Nop();
        Nop();
        protectedSerialSend_G2(stimSend, 0);
        LCDsetCursor(3, 0);
        switch (place) {
            case 1:
                assertLaser_G2(type, atS1End);
                LCD_Write_Char('d');
                if (taskParam.sample1Length < 1000u)
                    waitTaskTimer(1000u - taskParam.sample1Length);
                break;
            case 2:
                assertLaser_G2(type, atSecondOdorEnd);
                LCD_Write_Char('D');
                if (taskParam.test1Length < 1000u)
                    waitTaskTimer(1000u - taskParam.test1Length);
                break;
            case 4:
                assertLaser_G2(type, atResponseCueEnd);
                LCD_Write_Char('R');
                break;
        }
    }
}

static void processHit_G2(int id) {
    protectedSerialSend_G2(22, 1);
    setWaterPortOpen(1);
    waitTaskTimer(waterLen);
    setWaterPortOpen(0);
    currentMiss = 0;
    protectedSerialSend_G2(SpHit, id);
    lcdWriteNumber_G2(++hit, 5, 0);
}

static void processFalse_G2(int id) {
    currentMiss = 0;
    protectedSerialSend_G2(SpFalseAlarm, id);
    lcdWriteNumber_G2(++falseAlarm, 5, 1);
    stim_G2(5, taskParam.correctionCue, LASER_OFF);
    waitTaskTimer(4000);
}

static void processMiss_G2(int id) {
    currentMiss++;
    protectedSerialSend_G2(SpMiss, id);
    lcdWriteNumber_G2(++miss, 9, 0);
}

static int waterNResult_G2(int firstOdor, int secondOdor, int id) {
    int rtn = 0;
    int rewardWindow = (taskType_G2 == ODPA_RD_SHAPING_TASK
            || taskType_G2 == ODPA_RD_CATCH_LASER_TASK) ? 1000 : 500;

    lick_G2.portSide = 0;
    switch (taskType_G2) {

        case GONOGO_TASK:

            for (timerCounterI = 0; timerCounterI < 500 && !lick_G2.portSide; lick_G2.portSide = lick_G2.current);

            /////Reward
            if (!lick_G2.portSide) {
                if (!isLikeOdorA_G2(firstOdor)) {
                    protectedSerialSend_G2(SpCorrectRejection, 1);
                    lcdWriteNumber_G2(++correctRejection, 9, 1);
                } else {
                    processMiss_G2(1);
                }
            } else if (!isLikeOdorA_G2(firstOdor)) {
                processFalse_G2(1);
            } else {
                processHit_G2(1);
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
            for (timerCounterI = 0; timerCounterI < rewardWindow && !lick_G2.portSide; lick_G2.portSide = lick_G2.current);

            /////Reward
            if (!lick_G2.portSide) {
                if (isLikeOdorA_G2(firstOdor) == isLikeOdorA_G2(secondOdor)) {
                    protectedSerialSend_G2(SpCorrectRejection, id);
                    lcdWriteNumber_G2(++correctRejection, 9, 1);
                } else {
                    processMiss_G2(id);
                    if ((taskType_G2 == SHAPING_TASK || taskType_G2 == ODPA_SHAPING_TASK
                            || taskType_G2 == DUAL_TASK_LEARNING || taskType_G2 == DNMS_DUAL_TASK_LEARNING
                            || taskType_G2 == ODPA_RD_SHAPING_TASK
                            ) && ((rand() % 3) == 0)) {
                        protectedSerialSend_G2(22, 1);
                        setWaterPortOpen(1);
                        protectedSerialSend_G2(SpWater, 1);
                        waitTaskTimer(waterLen);
                        setWaterPortOpen(0);
                    }
                }
            } else if (isLikeOdorA_G2(firstOdor) == isLikeOdorA_G2(secondOdor)) {
                processFalse_G2(id);
                rtn = SpFalseAlarm;
            } else {
                processHit_G2(id);
            }
            break;

    }
    return rtn;
}

void dual_task_D_R(int type, int currentDistractor,
        int distractorJudgingPair) {
}

static void zxLaserTrial_G2(int s1, int t1, int s2, int t2, int laserType) {

    protectedSerialSend_G2(Sptrialtype, laserType);
    protectedSerialSend_G2(Splaser, (laserType != LASER_OFF));
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
            assertLaser_G2(laserType, atSecondOdorEnd);
            break;


        default:////////////////////////////////////DELAY/////////////////////
            if (taskParam.delay1 == 0) {
                waitTaskTimer(200u); ////////////////NO DELAY////////////////////
            } else {

                assertLaser_G2(laserType, atDelayBegin);
                waitTaskTimer(500u);
                assertLaser_G2(laserType, atDelay500MsIn);
                waitTaskTimer(500u);
                assertLaser_G2(laserType, atDelay1SecIn); ////////////////1Sec////////////

                if (taskParam.delay1 <= 4) {
                    waitTaskTimer(taskParam.delay1 * 1000u - 2000u);
                } else /*/////////////////////////////////////////////////
      * ////////DISTRACTOR//////////////////////////////
      * //////////////////////////////////////////////*/
                    if (taskType_G2 == DUAL_TASK_LEARNING || taskType_G2 == DUAL_TASK ||
                        //                        taskType_G2 == DUAL_TASK_CATCH_LASER ||
                        taskType_G2 == DUAL_TASK_ON_OFF_LASER_TASK ||
                        taskType_G2 == DUAL_TASK_ODAP_ON_OFF_LASER_TASK ||
                        taskType_G2 == DNMS_DUAL_TASK_LEARNING ||
                        taskType_G2 == DNMS_DUAL_TASK ||
                        taskType_G2 == DUAL_TASK_EVERY_TRIAL) {
                    waitTaskTimer(500u); //@500ms
                    // assertLaser_G2(type, atPreDualTask); //@2s
                    dual_task_D_R(laserType, taskParam.test2,
                            taskParam.sample2);
                    // waitTaskTimer_G2(1500u);
                    assertLaser_G2(laserType, atPostDualTask); // distractor@ 4 sec
                    if (taskParam.delay1 >= 8u) {
                        waitTaskTimer((unsigned int) taskParam.delay1 * 1000 - 5000);
                        assertLaser_G2(laserType, atDelayLastSecBegin);
                        waitTaskTimer(500u);
                    }
                    // at test odor -500 ms
                } else {

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
    if (taskParam.respCueLength >= 200) {
        int delayLick = waitTaskTimer(1000u);
        stim_G2(3, taskParam.respCue, laserType);
        delayLick |= waitTaskTimer(500u);
        LCDsetCursor(3, 0);
        if (delayLick) {
            muxDis(0x0f);
            protectedSerialSend_G2(SpAbortTrial, 1);
            LCD_Write_Char('A');
            waitTaskTimer(500u);
        } else {
            LCD_Write_Char('R');
            assertLaser_G2(laserType, atRewardBeginning);
            stim_G2(4, taskParam.respCue, laserType);
            //Assess Performance here
            //            int id = (taskType_G2 == DUAL_TASK || taskType_G2 == DUAL_TASK_LEARNING
            //                    || taskType_G2 == DUAL_TASK_ON_OFF_LASER_TASK || taskType_G2 == DUAL_TASK_ODAP_ON_OFF_LASER_TASK
            //                    || taskType_G2 == DNMS_DUAL_TASK_LEARNING || taskType_G2 == DNMS_DUAL_TASK || taskType_G2 == DUAL_TASK_EVERY_TRIAL) ? 2 : 1;
        }
    } else {
        waitTaskTimer(1000u);
    }
    resultRtn = waterNResult_G2(s1, t1, 1);
    waitTaskTimer(1000u); //water time sync
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
    protectedSerialSend_G2(SpITI, 0);

    waitTrial_G2();
}

void setWaterLen() {
    sendLargeValue(waterLen);
    int newWaterLen = getFuncNumber(3, "New water len?");
    sendLargeValue(waterLen);
    write_eeprom_G2(EEP_WATER_LEN_MS, newWaterLen);
    protectedSerialSend_G2(61, 0);
    asm("Reset");
}

void zxLaserSessions_G2(int trialsPerSession, int missLimit, int totalSession) {


    //    wait_ms(1000);
    int currentTrial = 0;
    int currentSession = 0;
    int laserOnType = laser_G2.laserTrialType;

    protectedSerialSend_G2(SpTaskType, taskType_G2);

    while ((currentMiss < missLimit) && (currentSession++ < totalSession)) {
        //        protectedSerialSend(SpOdorDelay, delay);
        protectedSerialSend_G2(SpSess, 1);

        splash_G2("    H___M___ __%", "S__ F___C___t___");

        lcdWriteNumber_G2(currentSession, 1, 1);
        hit = miss = falseAlarm = correctRejection = 0;
        //        unsigned int lastHit = 0;
        unsigned int shuffledList[4];
        unsigned int shuffledLongList[highLevelShuffleLength_G2];
        shuffleArray_G2(shuffledLongList, highLevelShuffleLength_G2);
        int sample1, test1, sample2, test2;
        sample2 = 0;
        test2 = 0;
        //        int lastOdor1;
        //        int lastOdor2;
        for (currentTrial = 0; currentTrial < trialsPerSession && currentMiss < missLimit;) {
            shuffleArray_G2(shuffledList, 4);
            int iterOf4;
            for (iterOf4 = 0; iterOf4 < 4 && currentMiss < missLimit; iterOf4++) {
                //                wait_ms(1000);
                int index = shuffledList[iterOf4];
                switch (taskType_G2) {

                    case DNMS_TASK:
                        sample1 = (index == 0 || index == 2) ? taskParam.sample1s[0] : taskParam.sample1s[1];
                        test1 = (index == 1 || index == 2) ? taskParam.test1s[0] : taskParam.test1s[1];
                        break;
                    case SHAPING_TASK:
                        sample1 = (index == 0 || index == 2) ? taskParam.sample1s[0] : taskParam.sample1s[1];
                        test1 = (sample1 == taskParam.sample1s[0]) ? taskParam.test1s[0] : taskParam.test1s[1];
                        break;
                    case GONOGO_TASK:
                    case GONOGO_LR_TASK:
                        sample1 = (index == 0 || index == 2) ? taskParam.sample1s[0] : taskParam.sample1s[1];
                        test1 = 0;
                        break;

                    case VARY_ODOR_LENGTH_TASK:
                    {
                        static int varyLengths[] = {250, 500, 750, 1000};
                        sample1 = (index == 0 || index == 2) ? taskParam.sample1s[0] : taskParam.sample1s[1];
                        test1 = (index == 1 || index == 2) ? taskParam.test1s[0] : taskParam.test1s[1];
                        unsigned int idxO1 = shuffledLongList[currentTrial]&0x03;
                        unsigned int idxO2 = shuffledLongList[currentTrial] >> 2;
                        taskParam.sample1Length = varyLengths[idxO1];
                        taskParam.test1Length = varyLengths[idxO2];
                        //                        lcdWriteNumber(idxO1,2,13,2);
                        //                        lcdWriteNumber(idxO2,2,15,2);
                        //                        lcdWriteNumber(odors.odor1Length,4,1,2);
                        //                        lcdWriteNumber(odors.odor2Length,4,5,2);

                        break;
                    }


                    case ODPA_TASK:
                        sample1 = (index == 0 || index == 2) ? taskParam.sample1s[0] : taskParam.sample1s[1];
                        test1 = (index == 1 || index == 2) ? taskParam.test1s[0] : taskParam.test1s[1];
                        break;

                    case ODPA_SHAPING_TASK:
                        sample1 = (index == 0 || index == 2) ? taskParam.sample1s[0] : taskParam.sample1s[1];
                        test1 = (sample1 == taskParam.sample1s[0]) ? taskParam.test1s[1] : taskParam.test1s[0];
                        break;
                    case ODPA_RD_SHAPING_TASK:
                        sample1 = (index == 0 || index == 2) ? taskParam.sample1s[0] : taskParam.sample1s[1];
                        test1 = (sample1 == taskParam.sample1s[0]) ? taskParam.test1s[1] : taskParam.test1s[0];
                        if (currentTrial > 15) {
                            laser_G2.laserTrialType = laserDuringDelayChR2;
                        } else {
                            laser_G2.laserTrialType = LASER_OFF;
                        }
                        break;

                    case ODPA_RD_CATCH_LASER_TASK:
                        if ((taskParam.falsePunish & 0x03) != 0x03 || correctionRepeatCount > 2) {
                            sample1 = (index == 0 || index == 2) ? taskParam.sample1s[0] : taskParam.sample1s[1];
                            test1 = (index == 1 || index == 2) ? taskParam.test1s[0] : taskParam.test1s[1];
                            correctionRepeatCount = 0;

                        }
                        if ((laser_G2.laserSessionType != LASER_NO_TRIAL) && currentTrial > 15) {
                            laser_G2.laserTrialType = laserDuringDelayChR2;
                        } else {
                            laser_G2.laserTrialType = LASER_OFF;
                        }
                        break;

                    case DUAL_TASK_LEARNING:
                    case DUAL_TASK:
                        sample1 = (index == 0 || index == 2) ? taskParam.sample1s[0] : taskParam.sample1s[1];
                        test1 = (index == 1 || index == 2) ? taskParam.test1s[0] : taskParam.test1s[1];
                        switch (shuffledLongList[currentTrial] % 3) {
                            case 0:
                                taskParam.test2 = 0u;
                                break;
                            case 1:
                                taskParam.test2 = 7u;
                                break;
                            case 2:
                                taskParam.test2 = 8u;
                                break;
                        }
                        break;
                    case DUAL_TASK_EVERY_TRIAL:
                        sample1 = (index == 0 || index == 2) ? taskParam.sample1s[0] : taskParam.sample1s[1];
                        test1 = (index == 1 || index == 2) ? taskParam.test1s[0] : taskParam.test1s[1];
                        if (shuffledLongList[currentTrial] % 2)
                            taskParam.test2 = 7u;
                        else
                            taskParam.test2 = 8u;
                        break;


                    case DUAL_TASK_ON_OFF_LASER_TASK:
                        sample1 = (index == 0 || index == 2) ? taskParam.sample1s[0] : taskParam.sample1s[1];
                        test1 = (index == 1 || index == 2) ? taskParam.test1s[0] : taskParam.test1s[1];
                        switch (shuffledLongList[currentTrial] % 8) {
                            case 0:
                            case 1:
                            case 2:
                            case 3:
                                taskParam.test2 = 0u;
                                break;
                            case 4:
                            case 6:
                                taskParam.test2 = 7u;
                                break;
                            case 5:
                            case 7:
                                taskParam.test2 = 8u;
                                break;
                        }
                        break;

                    case DUAL_TASK_ODAP_ON_OFF_LASER_TASK:
                        taskParam.test2 = (index % 2) ? 7u : 8u;
                        switch (shuffledLongList[currentTrial] % 8) {
                            case 0:
                            case 1:
                            case 2:
                            case 3:
                                sample1 = test1 = 20u;
                                break;
                            case 4:
                                sample1 = taskParam.sample1s[0];
                                test1 = taskParam.test1s[0];
                                break;
                            case 5:
                                sample1 = taskParam.sample1s[0];
                                test1 = taskParam.test1s[1];
                                break;
                            case 6:
                                sample1 = taskParam.sample1s[1];
                                test1 = taskParam.test1s[0];
                                break;
                            case 7:
                                sample1 = taskParam.sample1s[1];
                                test1 = taskParam.test1s[1];
                                break;
                        }
                        break;
                    case DNMS_DUAL_TASK_LEARNING:
                    case DNMS_DUAL_TASK:
                        sample1 = (index == 0 || index == 2) ? taskParam.sample1s[0] : taskParam.sample1s[1];
                        test1 = (index == 1 || index == 2) ? taskParam.test1s[0] : taskParam.test1s[1];
                        switch (shuffledLongList[currentTrial] % 3) {
                            case 0:
                                taskParam.test2 = 0u;
                                break;
                            case 1:
                                taskParam.test2 = 7u;
                                break;
                            case 2:
                                taskParam.test2 = 8u;
                                break;
                        }
                        break;
                }
                LCDsetCursor(0, 0);
                LCD_Write_Char(odorTypes_G2[sample1]);
                LCD_Write_Char(odorTypes_G2[test1]);

                //                int laserCurrentTrial;

                switch (laser_G2.laserSessionType) {
                    case LASER_NO_TRIAL:
                        laser_G2.laserTrialType = LASER_OFF;
                        break;
                    case LASER_EVERY_TRIAL:
                        break;
                    case LASER_OTHER_TRIAL:
                        laser_G2.laserTrialType = (currentTrial % 2) == 0 ? LASER_OFF : laserOnType;
                        break;

                    case LASER_LR_EACH_QUARTER:
                        laser_G2.side = isLikeOdorA_G2(sample1) ? 1 : 2;
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
                        laser_G2.side = isLikeOdorA_G2(sample1) ? 1 : 2;
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
                        laser_G2.side = isLikeOdorA_G2(sample1) ? 1 : 2;
                        break;

                    case LASER_LR_EVERY_OTHER_TRIAL:
                        laser_G2.side = isLikeOdorA_G2(sample1) ? 1 : 2;

                        laser_G2.laserTrialType = (currentTrial % 2) == 0 ? LASER_OFF : laserOnType;
                        break;

                    case LASER_INCONGRUENT_CATCH_TRIAL:
                        if ((currentTrial > 3 && currentTrial < 8 && isLikeOdorA_G2(sample1) && isLikeOdorA_G2(test1))
                                || (currentTrial > 7 && currentTrial < 12 && isLikeOdorA_G2(sample1)&& !isLikeOdorA_G2(test1))
                                || (currentTrial > 11 && currentTrial < 16 && !isLikeOdorA_G2(sample1) && isLikeOdorA_G2(test1))
                                || (currentTrial > 15 && currentTrial < 20 && !isLikeOdorA_G2(sample1) && !isLikeOdorA_G2(test1))) {
                            laser_G2.side = isLikeOdorA_G2(sample1) ? 2 : 1;
                        } else {
                            laser_G2.side = isLikeOdorA_G2(sample1) ? 1 : 2;
                        }
                        break;
                    case LASER_DUAL_TASK_ON_OFF:
                        switch (shuffledLongList[currentTrial] % 8) {
                            case 0:
                            case 1:
                            case 4:
                            case 5:
                                laser_G2.laserTrialType = LASER_OFF;
                                break;
                            case 2:
                            case 3:
                            case 6:
                            case 7:
                                laser_G2.laserTrialType = laserCoverDistractor;
                                break;
                        }
                        break;
                    case LASER_DUAL_TASK_ODAP_ON_OFF:
                        laser_G2.laserTrialType = (index < 2) ? LASER_OFF : laserCoverDistractor;
                        break;
                    case LASER_OTHER_BLOCK:
                        if (rand()&0x1)
                            laser_G2.laserTrialType = currentTrial < (trialsPerSession / 2) ? LASER_OFF : laserOnType;
                        else
                            laser_G2.laserTrialType = currentTrial < (trialsPerSession / 2) ? laserOnType : LASER_OFF;
                        break;
                }
                //                zxLaserTrial_G2(laser_G2.laserTrialType, firstOdor, taskParam, taskParam.delay1, secondOdor, WaterLen, taskParam.ITI);
                zxLaserTrial_G2(sample1, test1, sample2, test2, laser_G2.laserTrialType);
                currentTrial++;
            }
        }
        protectedSerialSend_G2(SpSess, 0);

    }
    protectedSerialSend_G2(SpTrain, 0); // send it's the end
    u2Received = -1;
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
    protectedSerialSend_G2(SpLaserSwitch, 1);
}

void turnOffLaser_G2() {
    laser_G2.on = 0;
    LCDsetCursor(3, 0);
    LCD_Write_Char('.');
    protectedSerialSend_G2(SpLaserSwitch, 0);
}
