/*
 * File:   hal.c
 * Author: Tony Lab
 *
 * Created on August 7, 2017, 9:48 AM
 */


#include "hal.h"


uint32_t timerCounterI, timerCounterJ;
int u2Received = -1;

//void flashRB(int port) {
//
//    switch (port) {
//        case 8:
//            PORTBbits.RB8 = 1;
//            wait_ms(50);
//            PORTBbits.RB8 = 0;
//            break;
//        case 9:
//            PORTBbits.RB9 = 1;
//            wait_ms(50);
//            PORTBbits.RB9 = 0;
//            break;
//        case 10:
//            PORTBbits.RB10 = 1;
//            wait_ms(50);
//            PORTBbits.RB10 = 0;
//            break;
//        case 11:
//            PORTBbits.RB11 = 1;
//            wait_ms(50);
//            PORTBbits.RB11 = 0;
//            break;
//            //        case 10:
//            //            PORTBbits.RB8 = 1;
//            //            wait_ms(50);
//            //            PORTBbits.RB8 = 0;
//            //            break;
//            //        case 8:
//            //            PORTBbits.RB8 = 1;
//            //            wait_ms(50);
//            //            PORTBbits.RB8 = 0;
//            //            break;
//            //        case 8:
//            //            PORTBbits.RB8 = 1;
//            //            wait_ms(50);
//            //            PORTBbits.RB8 = 0;
//            //            break;
//            //        case 8:
//            //            PORTBbits.RB8 = 1;
//            //            wait_ms(50);
//            //            PORTBbits.RB8 = 0;
//            //            break;
//            //        case 8:
//            //            PORTBbits.RB8 = 1;
//            //            wait_ms(50);
//            //            PORTBbits.RB8 = 0;
//            //            break;
//    }
//
//}

void initPorts() {
    TRISA = 0x3FFF;
    LATA = 0;
    ADPCFG = 0x0300;
    TRISB = 0xFCFF;
    LATB = 0;
    TRISC = 0xFFF5;
    LATC = 0x0A;
    TRISD = 0xCFFF;
    TRISD = 0;
    TRISE = 0;
    LATE = 0xFF;
    TRISF = 0xFFFC;
    LATF = 0x03;
    TRISG = 0xFF33;
    LATG = 0x00;


}

void initTMR1(void) {

    TMR1 = 0;
    PR1 = 10000; // 5ms @ 2K counter++ per ms
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;
    T1CON = 0x8010; //FCY @ 1:8 prescale, 2K counter++ per ms
    ConfigIntTimer1(T1_INT_PRIOR_7 & T1_INT_ON);
    timerCounterI = 0u;
    timerCounterJ = 0u;

}

//void setP10(int pin, int value) {
//    switch (pin) {
//        case 1:
//            P10_1 = value;
//            break;
//        case 2:
//            P10_2 = value;
//            break;
//        case 3:
//            P10_3 = value;
//            break;
//        case 4:
//            P10_4 = value;
//            break;
//        case 5:
//            P10_5 = value;
//            break;
//        case 6:
//            P10_6 = value;
//            break;
//        case 7:
//            P10_7 = value;
//            break;
//        case 8:
//            P10_8 = value;
//            break;
//    }
//    Nop();
//    Nop();
//    Nop();
//    Nop();
//
//}

inline void tick(unsigned int i) {

    timerCounterI += (uint32_t) i;
    timerCounterJ += (uint32_t) i;
    PORTAbits.RA15 = (timerCounterJ & 0x0100) >> 8;
}

void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0;
    tick(5u);
}

//void __attribute__((__interrupt__, no_auto_psv)) _U2TXInterrupt(void)
// {
//    IFS1bits.U2TXIF = 0;
//}

void wait_ms(unsigned int time) {
    uint32_t t32 = (uint32_t) time;
    timerCounterI = 0u;
    while (timerCounterI < t32);
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
    u2send(61, 0);
}

void __attribute__((interrupt, no_auto_psv)) _U2RXInterrupt(void) {
    if (U2STAbits.OERR == 1) {
        U2STAbits.OERR = 0;
    }
    if (0x2a == (u2Received = U2RXREG)) {
        u2send(61, 0);
        wait_ms(50);
        asm("RESET");
    }
    IFS1bits.U2RXIF = 0;
}

void u2send(int u2Type, int u2Value) {
    static unsigned char u2Send[] = {0x55, 0x00, 0x00, 0xAA};
    u2Send[1] = (unsigned char) u2Type;
    u2Send[2] = (unsigned char) u2Value;
    static int count = sizeof (u2Send);
    int i = 0;
    for (; i < count; i++) {
        while (BusyUART2());
        U2TXREG = u2Send[i];
    }
}

void setP10_4bit(int val) {
    P10_1 = (val & 0x1);
    Nop();
    Nop();
    P10_3 = ((val & 0x2) > 1);
    Nop();
    Nop();
    P10_5 = ((val & 0x4) > 2);
    Nop();
    Nop();
    P10_7 = ((val & 0x8) > 3);
    Nop();
    Nop();
}

void setP10_dis(int val) {
    PORTDbits.RD12 = (val & 0x1);
    Nop();
    Nop();
    PORTDbits.RD13 = ((val & 0x2) > 1);
    Nop();
    Nop();
    PORTBbits.RB8 = ((val & 0x4) > 2);
    Nop();
    Nop();
    PORTBbits.RB9 = ((val & 0x8) > 3);
    Nop();
    Nop();
}

