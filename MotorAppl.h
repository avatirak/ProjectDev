#ifndef MotorAppl_H
#define MotorAppl_H

#define Motor_Debug_Code 

typedef enum
{
    MOTOR_OFF_STATE=0,
    MOTOR_ON_STATE=1
}MOTOR_STATE_ENUM;


typedef struct
{
    MOTOR_STATE_ENUM motor_state;
    unsigned int    motor_timer_value;
}appl_motor_state_struct;

void MotorApplDataFromSms(MOTOR_STATE_ENUM MotorSt, unsigned int lTimerValue );
void MotorStateAndTiming(void);

#endif









