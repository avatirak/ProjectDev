#include <stdio.h>
#include "M051Series.h"
#include "EEPROM_24LC64.h"
#include "EepromAppl.h"
#include "string.h"

#define FIRST_MOBILE_EPRM_ADDR              1
#define SECOND_MOBILE_EPRM_ADDR             FIRST_MOBILE_EPRM_ADDR+14
#define THIRD_MOBILE_EPRM_ADDR              SECOND_MOBILE_EPRM_ADDR+14
#define GENERIC_MOBILE_EPRM_ADDR            THIRD_MOBILE_EPRM_ADDR+14
#define FIRST_PASSWARD_EPRM_ADDR            GENERIC_MOBILE_EPRM_ADDR+14
#define SECOND_PASSWARD_EPRM_ADDR           FIRST_PASSWARD_EPRM_ADDR+5
#define GENERIC_PASSWARD_EPRM_ADDR          SECOND_PASSWARD_EPRM_ADDR+5
#define MOTOR_STATE_EPRM_ADDR               GENERIC_PASSWARD_EPRM_ADDR+5
#define TIMER_VALUE_EPRM_ADDR               MOTOR_STATE_EPRM_ADDR+1
#define SYSTEM_PARAM_EPRM_ADDR              TIMER_VALUE_EPRM_ADDR +2



uint8_t geeprom_addr[]={    FIRST_MOBILE_EPRM_ADDR,
                            SECOND_MOBILE_EPRM_ADDR,
                            THIRD_MOBILE_EPRM_ADDR,
                            GENERIC_MOBILE_EPRM_ADDR,
                            FIRST_PASSWARD_EPRM_ADDR,
                            SECOND_PASSWARD_EPRM_ADDR,
                            GENERIC_PASSWARD_EPRM_ADDR,
                        };

                        

/**
  * @brief  Write mobile number and passward to eeprom memory
  * @param  Pointer to mobile no and passward string and rank of mobile number or passward
  * @return None.
  * @note   
  */                        
                        
void WrPswrdMobileNumb(unsigned char * lmobile_no, EPASSWRD_MOBILE_NUMB lnumb_id )
{

    uint8_t lmobile_addr=0;
    
    lmobile_addr=geeprom_addr[(uint8_t)lnumb_id];
    while(*lmobile_no != '\0')       
    {      
        EEPROM_Write(lmobile_addr, *lmobile_no);   /* Write data to EEPROM */ 
        lmobile_no++;
        lmobile_addr++;
    }   
}


/**
  * @brief  Read mobile number and passward from eeprom memory
  * @param  Pointer to mobile no and passward string and rank of mobile number or passward
  * @return None.
  * @note   
  */

void RdPswrdMobileNumb(unsigned char * lmobile_no, EPASSWRD_MOBILE_NUMB lnumb_id )
{
    uint8_t lmobile_addr=0;
    lmobile_addr=geeprom_addr[(uint8_t)lnumb_id];
    
    while(*lmobile_no != '\0')       
    {       
        *lmobile_no=EEPROM_Read(lmobile_addr );   /* Read data from EEPROM */ 
        lmobile_no++;
        lmobile_addr++;
    }   
}

void WrMotorStateToEeprom(unsigned char lmotor_state)
{
    EEPROM_Write( MOTOR_STATE_EPRM_ADDR, lmotor_state );   /* Write data to EEPROM */
}

MOTOR_STATE_ENUM RdMotorStateFromEeprom()
{
    unsigned char lMotor_st=0;
    lMotor_st=EEPROM_Read( MOTOR_STATE_EPRM_ADDR );
    return (MOTOR_STATE_ENUM)lMotor_st;
    
}

void WrMotorTimerValue(unsigned int lMtimer)
{
    unsigned char lMtimer_l=0;
    unsigned char lMtimer_h=0;
    lMtimer_l=(unsigned char)(0xFF & lMtimer);
    EEPROM_Write( TIMER_VALUE_EPRM_ADDR, lMtimer_l );   /* Write data to EEPROM */
    lMtimer_h=(unsigned char)(lMtimer>>8);
    EEPROM_Write( TIMER_VALUE_EPRM_ADDR+1, lMtimer_h );   /* Write data to EEPROM */
}

unsigned int RdMotorTimerValue()
{
    unsigned int lMotoTimer=0;
    unsigned char lMotoTimer_l=0;
    
    lMotoTimer_l = EEPROM_Read( TIMER_VALUE_EPRM_ADDR ); 
    lMotoTimer = lMotoTimer_l;
    lMotoTimer_l = EEPROM_Read( TIMER_VALUE_EPRM_ADDR+1 );
    lMotoTimer = lMotoTimer | lMotoTimer_l<<8;
    return lMotoTimer;
}

void WrSystemParaValue(unsigned char lSys_para)
{
    EEPROM_Write( SYSTEM_PARAM_EPRM_ADDR, lSys_para );   /* Write data to EEPROM */
}

unsigned char RdSystemParaValue()
{
    return EEPROM_Read( SYSTEM_PARAM_EPRM_ADDR );
}
