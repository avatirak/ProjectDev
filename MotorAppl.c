#include "MotorAppl.h"
#include "EepromAppl.h"

#ifdef Motor_Debug_Code
#include "lcd_Driver.h"
#include "stdio.h"
char Motor_string[15]="Motor On:";
#endif


extern volatile unsigned char mot_int_timer;
appl_motor_state_struct appl_motor_state_st={MOTOR_OFF_STATE,0};


void MotorApplDataFromSms(MOTOR_STATE_ENUM MotorSt, unsigned int lTimerValue )
{

    if(MotorSt==MOTOR_OFF_STATE)
    {
        appl_motor_state_st.motor_timer_value=0;
        appl_motor_state_st.motor_state = MotorSt;
    }
    else
    {
        appl_motor_state_st.motor_timer_value = lTimerValue;
        appl_motor_state_st.motor_state = MotorSt;
    }
    WrMotorTimerValue(appl_motor_state_st.motor_timer_value);
    WrMotorStateToEeprom((unsigned char)appl_motor_state_st.motor_state);
}

/**
  * @brief  writing the state of motor timer after 5 min and status in eeprom and decrement time after every one sec
  *
  * @param  None.
  * @return None.
  * @note   
  */
void MotorStateAndTiming(void)
{
    static unsigned int previous_timer=0;
    
    if( appl_motor_state_st.motor_state == MOTOR_ON_STATE)
    {
        if( appl_motor_state_st.motor_timer_value !=0 )
        {
            if(mot_int_timer>99)
            {              
               mot_int_timer=0; 
                
               if( previous_timer < appl_motor_state_st.motor_timer_value) 
                   previous_timer = appl_motor_state_st.motor_timer_value;
               
               if (previous_timer >= (appl_motor_state_st.motor_timer_value + 300))
               {    
                    previous_timer = appl_motor_state_st.motor_timer_value;                   
                    WrMotorTimerValue(appl_motor_state_st.motor_timer_value);
               }
               
               if(appl_motor_state_st.motor_timer_value <10)
               {
                   appl_motor_state_st.motor_timer_value=0;
                   appl_motor_state_st.motor_state=MOTOR_OFF_STATE;
                   WrMotorTimerValue(appl_motor_state_st.motor_timer_value);
                   WrMotorStateToEeprom(appl_motor_state_st.motor_state);
               }  

               appl_motor_state_st.motor_timer_value--; 
               #ifdef Motor_Debug_Code
               sprintf(Motor_string + 9, "%d", appl_motor_state_st.motor_timer_value);
               LCD_Print(3, Motor_string);
               #endif               
            }
        }
        else
        {
            if(mot_int_timer>100)
            { 
                mot_int_timer=0;                     
                LCD_Print(3, "Motor ON      ");
            }
        }              
    }
    else
    {
        if(mot_int_timer>100)
        { 
            mot_int_timer=0;                     
            LCD_Print(3, "Motor OFF     ");
        } 
    }  
    
}






