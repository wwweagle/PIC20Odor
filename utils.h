/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef UTILS_H
#define	UTILS_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdint.h>
#include "emlist.h"

int getFuncNumber(int targetDigits, const char* message);


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#define SpLick 	 0 	//        !PORTDbits.RD8  
#define SpKey 	 1  	//Keypad #[ZX]



#define SpFalseAlarm		 4	// false alarm
#define SpCorrectRejection		 5	// correct rejection
#define SpMiss		 6	// miss
#define SpHit		 7 	// hit

#define SpWater 8

#define SpOdor_A             9         
#define SpOdor_B	10


#define SpTrialWait 	 20
#define SpPermInfo   21 
#define SpLickFreq   22


#define SpValHigh   23 
#define SpValLow   24 
#define SpLCD_SET_XY 25
#define SpLCD_Char 26
#define SpChartHigh   27
#define SpChartLow   28
#define SpTaskType         51
#define Sptrialtype     58
#define SpITI           59  // 1 start 0 end

#define SpSess          61  // 1 start 0 end
#define SpTrain         62  // 1 start 0 end
#define Splaser         65

#define SpLaserSwitch    79

#define SpResponseCue 83
#define SpAbortTrial 84
#define SpCorrectionCue 85




#define SHAPING_TASK 10
#define DNMS_LEARNING_TASK 19
#define DNMS_TASK 20
//#define _ASSOCIATE_SHAPING_TASK 24
//#define _ASSOCIATE_TASK 25
#define GONOGO_TASK 30
#define DNMS_2AFC_TEACH 38
#define DNMS_LR_LEARNING_TASK 39
#define DNMS_2AFC_TASK 40
#define GONOGO_2AFC_TEACH 49
#define GONOGO_LR_TASK 50
#define NO_ODOR_CATCH_TRIAL_TASK 60
#define VARY_ODOR_LENGTH_TASK 70
#define DUAL_TASK_LEARNING 90
#define DUAL_TASK 93
#define DUAL_TASK_ON_OFF_LASER_TASK 95
#define DUAL_TASK_ODAP_ON_OFF_LASER_TASK 96
#define DUAL_TASK_DISTRx3_TASK 98
#define ODPA_SHAPING_TASK 99
#define ODPA_RD_SHAPING_TASK 100
#define ODPA_RD_TASK 102
#define ODPA_TASK 110
#define DNMS_DUAL_TASK_LEARNING 120
#define DNMS_DUAL_TASK 125
#define DUAL_TASK_EVERY_TRIAL  130









//Laser Session Type, including ZJ's variety
#define LASER_OTHER_TRIAL 1
#define LASER_NO_TRIAL 2
#define LASER_EVERY_TRIAL 3
#define LASER_CATCH_TRIAL 10
//#define laserFollowOdorA 4
//#define laserFollowOdorB 5
//#define laser1and2Half 6
//#define laser3and4Quarter 10
#define LASER_LR_EACH_QUARTER 20
#define LASER_EACH_QUARTER 21
#define LASER_12s_LR_EACH_QUARTER 25
#define LASER_12s_EACH_QUARTER 26
#define LASER_VARY_LENGTH 30
#define LASER_LR_EVERYTRIAL 40
#define LASER_LR_EVERY_OTHER_TRIAL 42
#define LASER_INCONGRUENT_CATCH_TRIAL 45
#define LASER_DUAL_TASK_ON_OFF 60
#define LASER_DUAL_TASK_ODAP_ON_OFF 61
#define LASER_AFTER_DISTR_3X 65
#define LASER_OTHER_BLOCK 80
#define LASER_SESS_UNDEFINED 100




//laser trial type
#define LASER_OFF 0
#define laserDuring3Baseline 3
#define laserDuring4Baseline 4
#define laserDuringBaseline 5
#define laserDuringDelay_Odor2 6
#define laserDuringBaseAndResponse 7
#define laserDuringBaselineNDelay 9
#define laserDuringDelay 10
#define laserDuringDelayChR2 11

#define laserRampDuringDelay 14
//#define laserLDuringDelay 15
//#define laserRDuringDelay 16
#define laserDuringOdor 20
#define laserDuring1stOdor 21
#define laserDuring2ndOdor 22
#define laserDuringBeginningToOneSecInITI 30
#define laserDuringEarlyDelay 40
#define laserDuringMidDelay 50
#define laserDuringLateDelay 60
#define laserNoDelayControlShort 69
#define laserNoDelayControl 70
#define laserDuringEarlyHalf 80

#define laserDuringLateHalf 90
#define laserDuring1Quarter 91
#define laserDuring2Quarter 92
#define laserDuring3Quarter 93
#define laserDuring4Quarter 94
#define laserDuringResponseDelay 95

#define laserDuring12s1Quarter 96
#define laserDuring12s2Quarter 97
#define laserDuring12s3Quarter 98
#define laserDuring12s4Quarter 99

#define trialTypeDnmsSwitch 100
#define trialTypeGoNogoSwitch 110
#define laser4sRamp 121
#define laser2sRamp 122
#define laser1sRamp 123
#define laser_5sRamp 124
#define laserSufficiency 130
#define laserBeforeDistractor 140
#define laserCoverDistractor 145
#define laserAfterDistractorMax 151
#define laserAfterMultiDistractor 160






//#define atTrialStart 10
#define at4SecBeforeS1 4
#define at3SecBeforeS1 5
#define at1SecBeforeS1 10
#define at500msBeforeS1 18
#define atS1Beginning 20
#define atS1End 30
#define atDelayBegin 40
#define atDelay500MsIn 42
#define atDelay1SecIn 200
#define atDelay1_5SecIn 205
#define atDelay2SecIn 210
#define atDelay2_5SecIn 212
#define atDelay3SecIn 214
#define atDelay3_5SIn 216
#define atDelay4_5SIn 218
#define atPreDualTask 220
#define atPostDualTask 222
#define atDelay1sToMiddle 224
#define atDelay500msToMiddle 225
#define atDelayMiddle 230
#define atDelayMid2Sec 235
#define atDelayMid2_5Sec 240
#define atDelayMid3Sec 245
#define atDelayLast2_5SecBegin 250
#define atDelayLast2SecBegin 255
#define atDelayLast1_5SecBegin 61
#define atDelayLastSecBegin 63
#define atDelayLast500mSBegin 65
#define atSecondOdorBeginning 70
#define atSecondOdorEnd 80
#define atResponseCueBeginning 90
#define atResponseCueEnd 95

#define atRewardBeginning 100
//#define atRewardBeginning 110
#define atRewardEnd 120


typedef struct {
    volatile unsigned int current;
    volatile uint32_t filter;
    unsigned int portSide;
    volatile unsigned int LCount;
    volatile unsigned int RCount;
} LICK_T_G2;

typedef struct {
    int *sample1s;
    int *test1s;
    int pairs1Count;
    int sample2;
    int test2;
    int respCue;
    int sample1Length;
    int sample2Length;
    int test1Length;
    int test2Length;
    int respCueLength;
    int falsePunish;
    int correctionCue;
    int correctionCueLength;
    int delay1;
    int delay2;
    int delay3;
    int ITI;
    int waitForTrial;

} TASK_T;

typedef struct {
    unsigned int laserSessionType;
    unsigned int laserTrialType;
    volatile unsigned int timer;
    unsigned int onTime;
    unsigned int offTime;
    unsigned int on;
    unsigned int side;
} LASER_T_G2;


extern LICK_T_G2 lick_G2;

extern LASER_T_G2 laser_G2;

extern TASK_T taskParam;

extern int waterLen;

extern int currentMiss, correctRatio;

extern int hit, miss, falseAlarm, correctRejection, abortTrial;

extern unsigned int highLevelShuffleLength_G2;

#define LICKING_LEFT 2
#define LICKING_RIGHT 3
#define LICKING_BOTH 127

void feedWaterFast_G2();
void setWaterPortOpen(int i);
void sendLargeValue(int val);
void shuffleArray_G2(unsigned int * orgArray, unsigned int arraySize);
int waitTaskTimer(unsigned int dTime);
void assertLaser_G2(int type, int step);
void waitTrial_G2();
void turnOnLaser_G2();
void turnOffLaser_G2();

#endif	/* XC_HEADER_TEMPLATE_H */

