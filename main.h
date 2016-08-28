#ifndef MAIN_H_
#define MAIN_H_

#include "M051Series.h"
#include "EEPROM_24LC64.h"
#include "spi_flash.h"
#include "string.h"
#include "stdlib.h"

#ifndef cTrue 
#define cTrue           1
#endif

#ifndef cFalse 
#define cFalse          0
#endif 


#define Debug_Code   

#ifdef Debug_Code
#include "lcd_Driver.h"
#endif

typedef enum
{
    STATE_ZERO=0,
    STATE_ONE,
    STATE_TWO,
    STATE_THREE,
    STATE_FOUR,
    STATE_FIVE,
    STATE_SIX,
    STATE_SEVEN,
    STATE_EIGHT,
    STATE_NINE,
    STATE_TEN,
    STATE_ELEVEN,
    STATE_TWELVE,
    STATE_THIRTEEN,
    STATE_FOURTEEN,
    STATE_FIFTEEN,
    STATE_SIXTEEN
}SWITCH_STATE;


void UartIRQHandler(void);
void StringToSendGsm(const char * lcdata);
void ReadDataFromGsm(void);
uint8_t FuncCmdResp(const char * ldata_cmd, const char * ldata_resp);

#endif
