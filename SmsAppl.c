
#include "EepromAppl.h"
#include "SmsAppl.h"
#include "string.h"
#include "MotorAppl.h"
#include "ctype.h"

typedef unsigned char uint8_t;

#define MOTOR_START_CMD     "MSTART"
#define MOTOR_STOP_CMD      "MSTOP"
#define MOTOR_START_TIMER   "MSTART T"
#define MOTOR_STOP_TIMER    "MSTOP T"

uint8_t SmsAppplicationFunc(char * lMNumber, char * lSmsData)
{

uint8_t lIndex=0;
uint8_t lmobile_no[12];
char *pSearchString;
uint8_t ValidMobileNo=0;
unsigned int ltime=0;

    for(lIndex=0;lIndex<4;lIndex++)
    {
        RdPswrdMobileNumb(lmobile_no, (EPASSWRD_MOBILE_NUMB) lIndex );
        pSearchString=strstr( lMNumber, (const char *)lmobile_no);
        if(pSearchString !=0 )
        {
            ValidMobileNo=1;
            break;
        }
    }
    if(ValidMobileNo)
    {
        if(!(strncasecmp((const char *)lSmsData, (const char *)MOTOR_START_CMD, 3)))
        {         
              if(!(strncasecmp((const char *)lSmsData, (const char *)MOTOR_START_TIMER, 8)))
              {                  
                  lSmsData+=8;
                  if(*lSmsData != '\0')
                  {
                    if(isdigit(*lSmsData))
                    {
                        ltime=(*lSmsData)-48;
                        if(ltime==0)
                            MotorApplDataFromSms( MOTOR_OFF_STATE, ltime);
                        else
                        {
                            ltime=ltime*60*60;        // convert to seconds;
                            MotorApplDataFromSms( MOTOR_ON_STATE, ltime);
                        }                                                   
                    }                
                  }
                                    
              }
              else if(!(strncasecmp((const char *)lSmsData, (const char *)MOTOR_STOP_TIMER, 7)))
              {
                    MotorApplDataFromSms( MOTOR_OFF_STATE, 0 );  
              }              
              else if(!(strncasecmp((const char *)lSmsData, (const char *)MOTOR_START_CMD, 6)))
              {
                    MotorApplDataFromSms( MOTOR_ON_STATE, 0 );  
              }
              else if(!(strncasecmp((const char *)lSmsData, (const char *)MOTOR_STOP_CMD, 5)))
              {
                    MotorApplDataFromSms( MOTOR_OFF_STATE, 0 );    
              }
        }

    }
    return 0;
}
