/*
 * File:   hal.c
 * Author: Tony Lab
 *
 * Created on August 7, 2017, 9:48 AM
 */


#include "hal.h"
#include "adc10.h"
#include "utils.h"
#include <i2c.h>


uint32_t timerCounterI, millisCounter, taskTimeCounter;
int u2Received = -1;
volatile int adcdataL;
volatile int adcdataR;
volatile int isSending;
volatile int sendLick;
int lickThreshL = 1023; // larger is more sensitive
int lickThreshR = 0; // larger is more sensitive
unsigned char LCD_PCF8574_ADDR = 0x7E;


const _prog_addressT EE_Addr_G2 = 0x7ff000;

unsigned char getLCDAddr() {
    unsigned char addr = 0x7E;
    unsigned char altAddr = 0x4E;
    IdleI2C();
    StartI2C(); /* Wait till Start sequence is completed */
    while (I2CCONbits.SEN); /* Clear interrupt flag */
    IFS0bits.MI2CIF = 0; /* Write Slave address and set master for transmission */
    MasterWriteI2C(addr); /* Wait till address is transmitted */
    while (I2CSTATbits.TBF); // 8 clock cycles
    while (!IFS0bits.MI2CIF); // Wait for 9th clock cycle
    IFS0bits.MI2CIF = 0; // Clear interrupt flag 
    timerCounterI = 0;
    while (I2CSTATbits.ACKSTAT && timerCounterI<50);
    if (timerCounterI < 50) {
        StopI2C(); /* Wait till stop sequence is completed */
        while (I2CCONbits.PEN);
        return addr;
    } else {
        StopI2C(); /* Wait till stop sequence is completed */
        while (I2CCONbits.PEN);
        return altAddr;
    }
}

void initPorts() {
    TRISA = 0x39FF;
    LATA = 0;
    TRISB = 0xFC0F;
    LATB = 0x030F;
    TRISC = 0xFFFF;
    LATC = 0;
    TRISD = 0x0F00;
    LATD = 0x3000;
    TRISE = 0;
    LATE = 0;
    TRISF = 0xFFFF;
    LATF = 0x00;
    TRISG = 0xFCFF;
    LATG = 0;

}

void initTMR1(void) {

    TMR1 = 0;
    PR1 = 5000; // 5ms @ 1K counter++ per ms 
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;
    T1CON = 0x8010; //FCY @ 1:8 prescale, 1K counter++ per ms
    ConfigIntTimer1(T1_INT_PRIOR_7 & T1_INT_ON);
    timerCounterI = 0u;
    millisCounter = 0u;
}

inline void tick(unsigned int i) {

    timerCounterI += (uint32_t) i;
    millisCounter += (uint32_t) i;
    PORTDbits.RD14 = (millisCounter & 0x0200) >> 9;
    Nop();
    Nop();
    PORTDbits.RD15 = (millisCounter & 0x0200) >> 9;
    Nop();
    Nop();

}

void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt(void) {

    tick(5u);

    if (laser_G2.on) {
        BNC_4 = 1;
    } else {
        BNC_4 = 0;
    }
    volatile int sel = (int) ((((double) (adcdataL - adcdataR)) / (adcdataL + adcdataR) + 1)*512);

    //    if ((adcdataL > lickThreshL && adcdataR > lickThreshR)
    //            || (adcdataL <= lickThreshL && adcdataR <= lickThreshR)) {
    if (sel <= lickThreshL && sel >= lickThreshR) {
        lick_G2.current = 0;
        lick_G2.stable = 0;
        BNC_1 = 0;
    } else if (lick_G2.current == 0) {//(Lick left XOR lick right)
        lick_G2.filter = millisCounter;
        lick_G2.current = LICKING_DETECTED;
    } else if (lick_G2.current == LICKING_DETECTED) {
        if (millisCounter > lick_G2.filter + 10) {
            BNC_1 = 1;
            char sendSide = 2;
            if (sel > lickThreshL) {
                lick_G2.LCount++;
                sendSide = 'L';
                lick_G2.stable = 'L';
            } else {
                lick_G2.RCount++;
                lick_G2.stable = 'R';
                sendSide = 'R';
            }
            lick_G2.current = LICK_SENT;
            if (isSending) {
                sendLick = sendSide;
            } else {
                serialSend(0, sendSide);
                sendLick = 0;
            }
        }
    }
    //  if (Serial.peek() == 0x2a) {
    //    protectedSerialSend_G2(61, 0);
    //    Timer1.detachInterrupt();
    //    callResetGen2();
    //  }
    //  if(sendFID && (millis()>fidTimeStamp+500)){
    //    fidTimeStamp=millis();
    //    FIDtest();
    //  }

    if ((millisCounter % 500u == 0) && (!isSending)) {
        if (lick_G2.refreshLickReading) {
            sendChart(sel, 0);
            //            sendChart(adcdataR, 1);
        }
        if (sel > lickThreshL) {
            serialSend(SpLickDisp, 1);
        }
        if (sel < lickThreshR) {
            serialSend(SpLickDisp, 2);
        }
    }



    IFS0bits.T1IF = 0;
}

void __attribute__((__interrupt__, no_auto_psv)) _ADCInterrupt(void) {
    adcdataR = ADCBUF0; //RB14
    adcdataL = ADCBUF1; //RB15
    IFS0bits.ADIF = 0; //After conversion ADIF is set to 1 and must be cleared
}

//void __attribute__((__interrupt__, no_auto_psv)) _U2TXInterrupt(void)
// {
//    IFS1bits.U2TXIF = 0;
//}

void wait_ms(int time) {
    uint32_t t32 = (uint32_t) time;
    timerCounterI = 0u;
    while (timerCounterI < t32);
}

void wait_Sec(int time) {
    while (time--) {
        wait_ms(1000);
    }
}

void initUART2(void) {
    unsigned int baudvalue;
    unsigned int U2MODEvalue;
    unsigned int U2STAvalue;
    CloseUART2();
    ConfigIntUART2(UART_RX_INT_EN & UART_RX_INT_PR1 &
            UART_TX_INT_DIS & UART_TX_INT_PR1);
    U2MODEvalue = UART_EN & UART_IDLE_CON &
            UART_DIS_WAKE & UART_DIS_LOOPBACK &
            UART_DIS_ABAUD & UART_NO_PAR_8BIT&
            UART_1STOPBIT;

    U2STAvalue = UART_INT_TX &
            UART_TX_PIN_NORMAL &
            UART_TX_ENABLE & UART_INT_RX_CHAR &
            UART_ADR_DETECT_DIS &
            UART_RX_OVERRUN_CLEAR;

    baudvalue = ((FCY / 16) / BAUDRATE) - 1;
    OpenUART2(U2MODEvalue, U2STAvalue, baudvalue);
    serialSend(61, 0);
}

void __attribute__((interrupt, no_auto_psv)) _U2RXInterrupt(void) {
    if (U2STAbits.OERR == 1) {
        U2STAbits.OERR = 0;
    }
    if (0x2a == (u2Received = U2RXREG)) {
        serialSend(61, 0);
        wait_ms(50);
        asm("RESET");
    }
    IFS1bits.U2RXIF = 0;
}

void serialSend(int u2Type, int u2Value) {

    isSending = 1;
    while (BusyUART2());
    U2TXREG = (unsigned char) (u2Type & 0x7f);
    while (BusyUART2());
    U2TXREG = (unsigned char) (u2Value | 0x80);
    while (BusyUART2());

    if (sendLick > 0) {
        U2TXREG = (unsigned char) (0);
        while (BusyUART2());
        U2TXREG = (unsigned char) (sendLick | 0x80);
        while (BusyUART2());
    }
    sendLick = 0;
    isSending = 0;



}

void set4076_4bit(int val) {
    PORTDbits.RD0 = (val & 0x1);
    Nop();
    Nop();
    PORTDbits.RD1 = ((val & 0x2) >> 1);
    Nop();
    Nop();
    PORTDbits.RD2 = ((val & 0x4) >> 2);
    Nop();
    Nop();
    PORTDbits.RD3 = ((val & 0x8) >> 3);
    Nop();
    Nop();
}

void muxOff(int val) {
    PORTDbits.RD12 = (val & 0x1);
    Nop();
    Nop();
    PORTDbits.RD13 = ((val & 0x2) >> 1);
    Nop();
    Nop();
    PORTBbits.RB8 = ((val & 0x4) >> 2);
    Nop();
    Nop();
    PORTBbits.RB9 = ((val & 0x8) >> 3);
    Nop();
    Nop();
}

void initADC(void) {

    ADPCFG = 0x3FFF;
    ADCON2bits.SMPI = 1; //Interrupt on 2nd sample
    ADCON2bits.CHPS = 0; //Sample Channel CH0
    ADCON2bits.BUFM = 0; //    bit 1 BUFM: Buffer Mode Select bit
    //1 = Buffer configured as two 8-word buffers ADCBUF(15...8), ADCBUF(7...0)
    //0 = Buffer configured as one 16-word buffer ADCBUF(15...0.)


    //    ADCHSbits.CH0SA = 0x0F;///CH0SA<3:0>: Channel 0 Positive Input Select for MUX A Multiplexer Setting bits
    ADCHSbits.CH0NA = 0; //Select VREF- for CH0- input
    ADCON2bits.CSCNA = 1; //Scan Input Selections for CH0+ S/H Input for MUX A Input Multiplexer Setting bit
    //    The CSCNA bit
    //(ADCON2<10>) enables the CH0 channel inputs to be scanned across a selected number of
    //analog inputs. When CSCNA is set, the CH0SA<3:0> bits are ignored.

    ADCSSL = 0xC000; //CSSL<15:0>: A/D Input Pin Scan Selection bits
    //1 = Select ANx for input scan
    //0 = Skip ANx for input scan



    ADCON1bits.ADSIDL = 1; //If ADSIDL = 1, the module will stop in Idle.
    ADCON1bits.FORM = 0; //00 = Integer (DOUT = 0000 00dd dddd dddd)
    ADCON1bits.SSRC = 7; //Conversion Trigger Source Select bits,Internal counter ends sampling and starts conversion (auto convert)

    ADCON1bits.SAMP = 1; //SAMP: A/D Sample Enable bit //1 = At least one A/D sample/hold amplifier is sampling
    ADCON1bits.ASAM = 1; //ASAM: A/D Sample Auto-Start bit
    //1 = Sampling begins immediately after last conversion completes. SAMP bit is auto set
    //0 = Sampling begins when SAMP bit set
    ADCON2bits.VCFG = 0; //VCFG<2:0>: Voltage Reference Configuration bits 
    //1 = Scan inputs
    //0 = Do not scan inputs

    ADCON2bits.ALTS = 0; //ALTS: Alternate Input Sample Mode Select bit
    //1 = Uses MUX A input multiplexer settings for first sample, then alternate between MUX B and MUX A input
    //multiplexer settings for all subsequent samples
    //0 = Always use MUX A input multiplexer settings

    ADCON3bits.SAMC = 31; //Auto-Sample Time bits
    //11111 = 31 TAD
    //·····
    //00001 = 1 TAD
    //00000 = 0 TAD (only allowed if performing sequential conversions using more than one S/H amplifier)
    ADCON3bits.ADRC = 0; //bit 7 ADRC: A/D Conversion Clock Source bit
    //1 = A/D internal RC clock
    //0 = Clock derived from system clock

    ADCON3bits.ADCS = 31; //ADCS<5:0>: A/D Conversion Clock Select bits
    //                    111111 = TCY/2 ? (ADCS<5:0> + 1) = 32 ? TCY
    //                    ······
    //                    000001 = TCY/2 ? (ADCS<5:0> + 1) = TCY
    //                    000000 = TCY/2 ? (ADCS<5:0> + 1) = TCY/2

    //For correct A/D conversions, the A/D conversion clock (TAD) must be selected to ensure a
    //minimum TAD time of 83.33 nsec





    IFS0bits.ADIF = 1;
    IEC0bits.ADIE = 1;

    ADCON1bits.ADON = 1; //ADON: A/D Operating Mode bit
    //1 = A/D converter module is operating
    //0 = A/D converter is off
}

void write_eeprom_G2(int offset, int value) {

    _erase_eedata(EE_Addr_G2 + offset, _EE_WORD);
    _wait_eedata();
    _write_eedata_word(EE_Addr_G2 + offset, value);
    _wait_eedata();
}

int read_eeprom_G2(int offset) {
    int temp;
    _memcpy_p2d16(&temp, EE_Addr_G2 + offset, 2);
    return temp;
}

int checkKeyboard() {

    int out = 0;
    if (PORTBbits.RB0 == 0) {
        out += 1;
    }
    if (PORTBbits.RB1 == 0) {
        out += 2;
    }
    if (PORTBbits.RB2 == 0) {
        out += 4;
    }
    if (PORTBbits.RB3 == 0) {
        out += 8;
    }
    return out;
}