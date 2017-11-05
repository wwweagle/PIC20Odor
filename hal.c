/*
 * File:   hal.c
 * Author: Tony Lab
 *
 * Created on August 7, 2017, 9:48 AM
 */


#include "hal.h"
#include "adc10.h"
#include "utils.h"


uint32_t timerCounterI, millisCounter;
int u2Received = -1;
volatile int adcdata;
volatile int isSending;
volatile int sendLick;
int lickThresh = 400; // larger is more sensitive

const _prog_addressT EE_Addr_G2 = 0x7ff000;

void initPorts() {
    TRISA = 0x39FF;
    LATA = 0;
    TRISB = 0xFCFF;
    LATB = 0x0300;
    TRISC = 0xFFFF;
    LATC = 0;
    TRISD = 0xCFF0;
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
    PORTAbits.RA15 = (millisCounter & 0x0100) >> 8;
}

inline int filtered_G2(void) {
    return (millisCounter > lick_G2.filter + 50u);
}

void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt(void) {

    tick(5u);

    if (laser_G2.on) {
        BNC_4 = 1;
    } else {
        BNC_4 = 0;
    }
    if (adcdata > lickThresh && lick_G2.current == 0 &&
            filtered_G2()) { // HIGH is lick
        lick_G2.filter = millisCounter;
        lick_G2.current = LICKING_DETECTED;
    } else if (adcdata > lickThresh && lick_G2.current == LICKING_DETECTED) {
        if (millisCounter > lick_G2.filter + 5) {
            lick_G2.stable = 1;
            lick_G2.LCount++;
            lick_G2.current=LICKING_SENT;
            if (isSending) {
                sendLick = 1;
            } else {
                protectedSerialSend_G2(0, 2);
                sendLick = 0;
            }
        }
    } else if (adcdata <= lickThresh) {
        lick_G2.current = 0;
        lick_G2.stable = 0;
        //  digitalWrite(38, LOW);
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





    IFS0bits.T1IF = 0;
}

void __attribute__((__interrupt__, no_auto_psv)) _ADCInterrupt(void) {
    adcdata = ADCBUF0; //RB8

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
    protectedSerialSend_G2(61, 0);
}

void __attribute__((interrupt, no_auto_psv)) _U2RXInterrupt(void) {
    if (U2STAbits.OERR == 1) {
        U2STAbits.OERR = 0;
    }
    if (0x2a == (u2Received = U2RXREG)) {
        protectedSerialSend_G2(61, 0);
        wait_ms(50);
        asm("RESET");
    }
    IFS1bits.U2RXIF = 0;
}

void protectedSerialSend_G2(int u2Type, int u2Value) {

    isSending = 1;
    while (BusyUART2());
    U2TXREG = (unsigned char) (u2Type & 0x7f);
    while (BusyUART2());
    U2TXREG = (unsigned char) (u2Value | 0x80);
    while (BusyUART2());

    if (sendLick) {
        U2TXREG = (unsigned char) (0);
        while (BusyUART2());
        U2TXREG = (unsigned char) (2 | 0x80);
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

void muxDis(int val) {
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
    ADCON1bits.ADSIDL = 0;
    ADCON1bits.FORM = 0;
    ADCON1bits.SSRC = 7;

    ADCON1bits.SAMP = 1;

    ADCON2bits.VCFG = 0;
    ADCON2bits.CSCNA = 1;
    ADCON2bits.SMPI = 2;

    ADCON2bits.BUFM = 0;
    ADCON2bits.ALTS = 0;

    ADCON3bits.SAMC = 31;
    ADCON3bits.ADRC = 1;

    ADCON3bits.ADCS = 31;

    ADCHSbits.CH0NB = 0;
    ADCHSbits.CH0NA = 0;
    ADCHSbits.CH0SA = 0x0F;
    ADCHSbits.CH0SB = 0;

    ADPCFG = 0x7FFF;
    ADCSSL = 0x8000;

    ADCON1bits.ASAM = 1;
    IFS0bits.ADIF = 1;
    IEC0bits.ADIE = 1;

    ADCON1bits.ADON = 1;
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
