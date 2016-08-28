#ifndef _AUDIO_PLAY
#define _AUDIO_PLAY
#include "MotorStarter.h"


typedef enum
{
	ZERO,
	ONE,
	TWO,
	THREE,
	FOUR,
	FIVE,
	SIX,
	SEVEN,
	EIGHT,
	NINE,
	TEN
}USER_INPUT_T;

/*State Messsage ID*/
typedef enum {
	PWR_PRESENT=1,
	FAULT,
	MOTOR_ON_FOR_SETTIME,
	MOTOR_ON_AFTER_SETTIME,
	ADVANCE_SETTING,	
	MOTOR_START_AFTER_FAULT_CLEAR,
	CUSTOM_TIME_SETTING,
	UNKNOWN_STATE
}STATE_ID_T;



/*Menu Next Step*/
typedef struct{
	STATE_ID_T State;
	USER_INPUT_T Key;
	STATE_ID_T NextState;	
}MENU_NEXTSTATE_T;


/*Menu State*/
typedef struct{
	STATE_ID_T State;
	void (*pFunc)(USER_INPUT_T);
}MENU_STATE_T;


void MenuInit(void);
void MenuUpdate(USER_INPUT_T keyValue);
void MenuExec(USER_INPUT_T keyValue);
unsigned char GsmApplCallFunc( stgsm_call_data_struct );
#endif

