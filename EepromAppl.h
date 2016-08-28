#ifndef EEPROMAPPL_H
#define EEPROMAPPL_H
#include "MotorAppl.h"

typedef enum
{
    FIRST_MOBILE_ADDR=0,
    SECOND_MOBILE_ADDR,
    THIRD_MOBILE_ADDR,
    GENERIC_MOBILE_ADDR,
    FIRST_PASSWARD_EPRM,
    SECOND_PASSWARD_EPRM,
    GENERIC_PASSWARD_EPRM,    
}EPASSWRD_MOBILE_NUMB;


void WrPswrdMobileNumb(unsigned char * lmobile_no, EPASSWRD_MOBILE_NUMB lnumb_id );

void RdPswrdMobileNumb(unsigned char * lmobile_no, EPASSWRD_MOBILE_NUMB lnumb_id );

void WrMotorStateToEeprom(unsigned char lmotor_state);

MOTOR_STATE_ENUM RdMotorStateFromEeprom(void);

void WrMotorTimerValue(unsigned int lMtimer);

unsigned int RdMotorTimerValue(void);

void WrSystemParaValue(unsigned char lSys_para);

unsigned char RdSystemParaValue(void);


#endif

