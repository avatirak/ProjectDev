
#include "M051Series.h"
#include "AudioPlay.h"


static void PWR_PRESENT_Exec(USER_INPUT_T);
static void FAULT_Exec(USER_INPUT_T);
static void MOTOR_ON_FOR_SETTIME_Exec(USER_INPUT_T);
static void MOTOR_ON_AFTER_SETTIME_Exec(USER_INPUT_T);
static void ADVANCE_SETTING_Exec(USER_INPUT_T keyValue);
static void MOTOR_START_AFTER_FAULT_CLEAR_Exec(USER_INPUT_T);




const MENU_NEXTSTATE_T MENU_NEXTSTATE[] =
{
	/*STATE     									INPUT						NEXT_STATE*/
	{PWR_PRESENT,  								ZERO,						UNKNOWN_STATE},
	{PWR_PRESENT,  								ONE,						MOTOR_ON_FOR_SETTIME},
	{PWR_PRESENT,  								TWO,						MOTOR_ON_AFTER_SETTIME},
	{PWR_PRESENT,  								THREE, 					ADVANCE_SETTING},
	{PWR_PRESENT,  								FOUR,  					PWR_PRESENT},
	
	{FAULT,												ZERO,						UNKNOWN_STATE},
	{FAULT,												ONE,						MOTOR_START_AFTER_FAULT_CLEAR},
	{FAULT,												TWO,						ADVANCE_SETTING},
	{FAULT,												THREE,					FAULT},
		
	{MOTOR_ON_FOR_SETTIME,				ZERO,						UNKNOWN_STATE},
	{MOTOR_ON_FOR_SETTIME,				ONE,						UNKNOWN_STATE},
	{MOTOR_ON_FOR_SETTIME,				TWO,						UNKNOWN_STATE},
	{MOTOR_ON_FOR_SETTIME,				THREE,					UNKNOWN_STATE},
	{MOTOR_ON_FOR_SETTIME,				FOUR,						UNKNOWN_STATE},
	{MOTOR_ON_FOR_SETTIME,				FIVE,						UNKNOWN_STATE},
	{MOTOR_ON_FOR_SETTIME,				SIX,						CUSTOM_TIME_SETTING},
	{MOTOR_ON_FOR_SETTIME,				SEVEN,					MOTOR_ON_FOR_SETTIME},
	{MOTOR_ON_FOR_SETTIME,				EIGHT,					PWR_PRESENT},
	
	{MOTOR_ON_AFTER_SETTIME,			ZERO,						UNKNOWN_STATE},
	{MOTOR_ON_AFTER_SETTIME,			ONE,						UNKNOWN_STATE},
	{MOTOR_ON_AFTER_SETTIME,			TWO,						UNKNOWN_STATE},
	{MOTOR_ON_AFTER_SETTIME,			THREE,					UNKNOWN_STATE},
	{MOTOR_ON_AFTER_SETTIME,			FOUR,						UNKNOWN_STATE},
	{MOTOR_ON_AFTER_SETTIME,			FIVE,						UNKNOWN_STATE},
	{MOTOR_ON_AFTER_SETTIME,			SIX,						CUSTOM_TIME_SETTING},
	{MOTOR_ON_AFTER_SETTIME,			SEVEN,					MOTOR_ON_AFTER_SETTIME},
	{MOTOR_ON_AFTER_SETTIME,			EIGHT,					PWR_PRESENT},
	
	{ADVANCE_SETTING,							ZERO,						UNKNOWN_STATE},
	{ADVANCE_SETTING,							ONE,						UNKNOWN_STATE},
	{ADVANCE_SETTING,							TWO,						UNKNOWN_STATE},
	{ADVANCE_SETTING,							THREE,					UNKNOWN_STATE},
		
	{MOTOR_START_AFTER_FAULT_CLEAR,ZERO,					MOTOR_ON_FOR_SETTIME},
	{MOTOR_START_AFTER_FAULT_CLEAR,ONE,						MOTOR_ON_AFTER_SETTIME},
	{MOTOR_START_AFTER_FAULT_CLEAR,TWO,						MOTOR_START_AFTER_FAULT_CLEAR},
	{MOTOR_START_AFTER_FAULT_CLEAR,THREE,					FAULT},
	
	{CUSTOM_TIME_SETTING,					ZERO,					  UNKNOWN_STATE},
	
	{(STATE_ID_T)0,					(USER_INPUT_T)0,			(STATE_ID_T)0}
	
};

const MENU_STATE_T MENU_STATE[] = 
{
	{PWR_PRESENT,											PWR_PRESENT_Exec},
	{FAULT,														FAULT_Exec},
	{MOTOR_ON_FOR_SETTIME,						MOTOR_ON_FOR_SETTIME_Exec},
	{MOTOR_ON_AFTER_SETTIME,					MOTOR_ON_AFTER_SETTIME_Exec},
	{ADVANCE_SETTING,									ADVANCE_SETTING_Exec},
	{MOTOR_START_AFTER_FAULT_CLEAR,		MOTOR_START_AFTER_FAULT_CLEAR_Exec}
};



STATE_ID_T PreviousTopState, State;
void (*pStateFunc)(USER_INPUT_T);

/*Variable Common too multiple state Need to Initalize its value At State Init Time*/
static USER_INPUT_T audioCMD;
static uint8_t noInputCount, wrongInputCount,userWaitSec;


/**
  * @brief  GSM Command Receive 
  */

unsigned char GsmApplCallFunc( stgsm_call_data_struct gsmCallStatus)
{
	static uint8_t mInit=0;
	if(mInit)
	{
		mInit=0;
		MenuInit();
	}
	//gsmCallStatus.MobileNo;
	//gsmCallStatus.dtmf_value;
	//gsmCallStatus.Curr_Call_Status;
	//gsmCallStatus.Call_DIR;
	
	if(gsmCallStatus.currt_call_sts == 1)
	{
		MenuExec((USER_INPUT_T)gsmCallStatus.dtmf_value);
        MenuUpdate((USER_INPUT_T)gsmCallStatus.dtmf_value);
	}
	else
	{
		mInit=1;
	}
	return 0;
}



/**
  * @brief  Init When Call Get Receives 
  */
void MenuInit(void)
{
	audioCMD = ZERO;
	wrongInputCount = 0;
	noInputCount = 0;
	pStateFunc = PWR_PRESENT_Exec;
	PreviousTopState = (STATE_ID_T)0;
  State = PWR_PRESENT;
}

/**
  * @brief  Execute Continue when call ON
  * @param  keyValue
  * @retval None
  */
void MenuExec(USER_INPUT_T keyValue)
{
	 if (pStateFunc) /* Check if there is a state function to be executed */
   {
			pStateFunc(keyValue); /* Execute the state function */
	 }
}

/**
  * @brief  Updates When User press valid Input
  * @param  keyValue
  * @retval None
  */
void MenuUpdate(USER_INPUT_T keyValue)
{
  uint8_t i = 0;
  STATE_ID_T NextState = (STATE_ID_T)0;

  for (i = 0; MENU_NEXTSTATE[i].State; i++)
  {
    if (MENU_NEXTSTATE[i].State == State && MENU_NEXTSTATE[i].Key == keyValue)
    {
      NextState = MENU_NEXTSTATE[i].NextState;
      break;
    }
  }

  /* if next state exists */
  if (8 != NextState)
  {
    PreviousTopState = State;
    State = NextState; /* Update current state */

    /* Execute the function of state if there is one*/
    for (i = 0; MENU_STATE[i].State; i++)
    {
      if (MENU_STATE[i].State == State)
      {
        pStateFunc = MENU_STATE[i].pFunc;
        if (pStateFunc) /* Check if there is a state function to be executed */
        {
					audioCMD = ZERO;
					wrongInputCount = 0;
					noInputCount = 0;
          pStateFunc(keyValue); /* Execute the state function */
        }
      }
    }
  }
}




static void PWR_PRESENT_Exec(USER_INPUT_T keyValue)
{
	uint8_t ISD2360_status;
	static uint8_t initTimer;
	
	
	if((1 > keyValue) && (4 < keyValue))
	{
		if(++wrongInputCount>7)
		{
			/*Excedded Limit Message*/
			audioCMD = TEN;				/*This Command is only used to End Call*/
		}
		else
		{
			/*Wrong Input Message*/
			audioCMD = ZERO;	
		}
	}

	ISD2360_status = PlayAudioFeedback(0);
	if(0 == ISD2360_status)		//Audio Not Playing
	{
		switch(audioCMD)
		{
			case ZERO:	
					/*POWER Status Message*/
					PlayAudioOnGsm(7);
					audioCMD = ONE;					
				break;
			
			case ONE:
					/*MOTOR_ON_FOR_SETTIME Message*/
					PlayAudioOnGsm(8);
					audioCMD = TWO;
				break;
			
			case TWO:
					/*MOTOR_ON_AFTER_SETTIME Message*/
					PlayAudioOnGsm(9);
					audioCMD = THREE;
				break;
			
			case THREE:
					/*ADVANCE_SETTING Message*/
					PlayAudioOnGsm(10);
					audioCMD = FOUR;
				break;
			
			case FOUR:
					/*Current Menu Repeat Message*/
					PlayAudioOnGsm(11);
					audioCMD = 0;
				break;
			
			case FIVE:
					/*Input Not Come then*/
					PlayAudioOnGsm(5);
					/*Start Timer*/
					if(initTimer)
					{
						initTimer = 0;
						userWaitSec = 0;
						noInputCount = 0;
					}			
					else if(10 < userWaitSec)
					{
						userWaitSec = 0;
						if(++noInputCount>2)
							/*Excedded Limit Message*/
							audioCMD = TEN;
						else{
							/* No Input Message*/	
						}
					}
				break;
					
			case TEN:
					/*End Call*/
				break;
			
			default:
				break;
			
		}
	}		
}

static void FAULT_Exec(USER_INPUT_T keyValue)
{
	uint8_t ISD2360_status;
	static uint8_t initTimer;
	
	
	if((1 > keyValue) && (3 < keyValue))
	{
		if(++wrongInputCount>7)
		{
			/*Excedded Limit Message*/
			audioCMD = TEN;				/*This Command is only used to End Call*/
		}
		else
		{
			/*Wrong Input Message*/
			audioCMD = ZERO;	
		}
	}

	ISD2360_status = PlayAudioFeedback(0);
	if(0 == ISD2360_status)		//Audio Not Playing
	{
		switch(audioCMD)
		{
			case ZERO:	
					/*Fault Status Message*/
					audioCMD = ONE;					
				break;
			
			case ONE:
					/*MOTOR_START_AFTER_FAULT_CLEAR Message*/
					audioCMD = TWO;
				break;
			
			case TWO:
					/*ADVANCE_SETTING Message*/
					audioCMD = THREE;
				break;
			
			case THREE:
					/*Current Menu Repeat Message*/
					audioCMD = FOUR;
				break;
				
			case FOUR:
					/*Input Not Come then*/
					/*Start Timer*/
					if(initTimer)
					{
						initTimer = 0;
						userWaitSec = 0;
						noInputCount = 0;
					}			
					else if(10 < userWaitSec)
					{
						userWaitSec = 0;
						if(++noInputCount>2)
							/*Excedded Limit Message*/
							audioCMD = TEN;
						else{
							/* No Input Message*/
						}
					}
				break;
					
			case TEN:
					/*End Call*/
				break;
			
			default:
				break;
			
		}
	}			
}

static void MOTOR_ON_FOR_SETTIME_Exec(USER_INPUT_T keyValue)
{
	uint8_t ISD2360_status;
	static uint8_t initTimer;
	
	
	if((1 > keyValue) && (8 < keyValue))
	{
		if(++wrongInputCount>7)
		{
			/*Excedded Limit Message*/
			audioCMD = TEN;				/*This Command is only used to End Call*/
		}
		else
		{
			/*Wrong Input Message*/
			audioCMD = ZERO;	
		}
	}

	ISD2360_status = PlayAudioFeedback(0);
	if(0 == ISD2360_status)		//Audio Not Playing
	{
		switch(audioCMD)
		{
			case ZERO:	
					/*Start Motor without Timer Message*/
					PlayAudioOnGsm(0);
					audioCMD = ONE;					
				break;
			
			case ONE:
					/*Motor ON After 1 Hour Message*/
					PlayAudioOnGsm(1);
					audioCMD = TWO;
				break;
			
			case TWO:
					/*Motor ON After 2 Hour Message*/
					PlayAudioOnGsm(2);
					audioCMD = THREE;
				break;
			
			case THREE:
					/*Motor ON After 3 Hour Message*/
					PlayAudioOnGsm(3);
					audioCMD = FOUR;
				break;
			
			case FOUR:
					/*Motor ON After 4 Hour Message*/
					PlayAudioOnGsm(4);
					audioCMD = FIVE;
				break;
						
			case FIVE:
					/*Motor ON After 5 Hour Message*/
					PlayAudioOnGsm(5);
					audioCMD = SIX;
				break;
						
			case SIX:
					/*Motor ON After Custom Hour Message*/
					PlayAudioOnGsm(6);
					audioCMD = SEVEN;
				break;
			
			case SEVEN:
					/*State Repete Message*/
					PlayAudioOnGsm(7);
					audioCMD = EIGHT;
				break;
			
			case EIGHT:
					/*Go to Parent Menu Message*/
					PlayAudioOnGsm(8);
					audioCMD = NINE;
				break;
			
			case NINE:
					/*Input Not Come then*/
					/*Start Timer*/
                    PlayAudioOnGsm(9);
					if(initTimer)
					{
						initTimer = 0;
						userWaitSec = 0;
						noInputCount = 0;
					}			
					else if(10 < userWaitSec)
					{
						userWaitSec = 0;
						if(++noInputCount>2)
							/*Excedded Limit Message*/
							audioCMD = TEN;
						else{
							/* No Input Message*/
						}
					}
				break;
					
			case TEN:
					/*End Call*/
				break;
			
			default:
				break;
			
		}
	}				
}

static void MOTOR_ON_AFTER_SETTIME_Exec(USER_INPUT_T keyValue)
{
	uint8_t ISD2360_status;
	static uint8_t initTimer;
	
	
	if((1 > keyValue) && (8 < keyValue))
	{
		if(++wrongInputCount>7)
		{
			/*Excedded Limit Message*/
			audioCMD = TEN;				/*This Command is only used to End Call*/
		}
		else
		{
			/*Wrong Input Message*/
			audioCMD = ZERO;	
		}
	}

	ISD2360_status = PlayAudioFeedback(0);
	if(0 == ISD2360_status)		//Audio Not Playing
	{
		switch(audioCMD)
		{
			case ZERO:	
					/*Start Motor without Timer Message*/
					audioCMD = ONE;					
				break;
			
			case ONE:
					/*Motor ON After 1 Hour Message*/
					audioCMD = TWO;
				break;
			
			case TWO:
					/*Motor ON After 2 Hour Message*/
					audioCMD = THREE;
				break;
			
			case THREE:
					/*Motor ON After 3 Hour Message*/
					audioCMD = FOUR;
				break;
			
			case FOUR:
					/*Motor ON After 4 Hour Message*/
					audioCMD = FIVE;
				break;
						
			case FIVE:
					/*Motor ON After 5 Hour Message*/
					audioCMD = SIX;
				break;
						
			case SIX:
					/*Motor ON After Custom Hour Message*/
					audioCMD = SEVEN;
				break;
			
			case SEVEN:
					/*State Repete Message*/
					audioCMD = EIGHT;
				break;
			
			case EIGHT:
					/*Go to Parent Menu Message*/
					audioCMD = NINE;
				break;
			
			case NINE:
					/*Input Not Come then*/
					/*Start Timer*/
					if(initTimer)
					{
						initTimer = 0;
						userWaitSec = 0;
						noInputCount = 0;
					}			
					else if(10 < userWaitSec)
					{
						userWaitSec = 0;
						if(++noInputCount>2)
							/*Excedded Limit Message*/
							audioCMD = TEN;
						else{
							/* No Input Message*/
						}
					}
				break;
					
			case TEN:
					/*End Call*/
				break;
			
			default:
				break;
			
		}
	}
}

static void ADVANCE_SETTING_Exec(USER_INPUT_T keyValue)
{
	
}

static void MOTOR_START_AFTER_FAULT_CLEAR_Exec(USER_INPUT_T keyValue)
{
	
}





