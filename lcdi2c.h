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
#ifndef LCD_I2C_H
#define	LCD_I2C_H

#include <xc.h> // include processor files - each processor file is guarded.  


void LCD_Init(void);
void LCD_Write_Char(char message);
void LCD_Write_Str(const char *message);

void LCDclear(void);
void LCDhome(void);

void LCDdisplayOff(void);
void LCDdisplayOn(void);
void LCDblinkOff(void);
void LCDblinkOn(void);
void LCDcursorOff(void);
void LCDcursorOn(void);
void LCDscrollDisplayLeft(void);
void LCDscrollDisplayRight(void);
void LCDleftToRight(void);
void LCDrightToLeft(void);
void LCDnoBacklight(void);
void LCDbacklight(void);
void LCDautoscroll(void);
void LCDnoAutoscroll(void);
void LCDcreateChar(unsigned char location, unsigned char charmap[]);
void LCDsetCursor(unsigned char col, unsigned char row);

inline void LCDcommandWrite(unsigned char value);
inline unsigned char LCDcommandRead(void);
inline void LCDdataWrite(unsigned char value);
inline unsigned char LCDdataRead(void);
unsigned char LCDbusy(void);
unsigned char LCDaddressCounter(void);
unsigned char LCDreadDDRam(unsigned char address);
unsigned char LCDreadCGRam(unsigned char address);
void DelayMicroseconds(unsigned short t);
void lcdWriteNumber_G2(int val, int x, int y);
void initI2C();
void splash_G2(const char* line1, const char* line2);


// TODO Insert appropriate #include <>

// TODO Insert C++ class definitions if appropriate

// TODO Insert declarations

// Comment a function and leverage automatic documentation with slash star star
/**
    <p><b>Function prototype:</b></p>
  
    <p><b>Summary:</b></p>

    <p><b>Description:</b></p>

    <p><b>Precondition:</b></p>

    <p><b>Parameters:</b></p>

    <p><b>Returns:</b></p>

    <p><b>Example:</b></p>
    <code>
 
    </code>

    <p><b>Remarks:</b></p>
 */
// TODO Insert declarations or function prototypes (right here) to leverage 
// live documentation

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

