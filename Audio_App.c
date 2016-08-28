#include "Audio_App.h"
#include "Audio_Driver.h"
#include <stdio.h>
#include "lcd_Driver.h"
uint8_t data_Print=0;
char gsp[15] = "SPI:";  

#define MICRO_MAX_NO   122
#define VM_MAX_NO 55
#define VP_MAX_NO 64

uint8_t gdata_to_play10[]={64,55,56,57,58,59,60,61,62,63,0x0};
uint8_t Zero_VP[]={64,0x0};
uint8_t One_VP[]={55,0x0};
uint8_t Two_VP[]={56,0x0};
uint8_t Three_VP[]={57,0x0};
uint8_t Four_VP[]={58,0x0};
uint8_t Five_VP[]={59,0x0};
uint8_t Six_VP[]={60,0x0};
uint8_t Seven_VP[]={61,0x0};
uint8_t Eight_VP[]={62,0x0};
uint8_t Nine_VP[]={63,0x0};

uint8_t * gplay_data_ref[]={    Zero_VP, 
                                One_VP,
                                Two_VP,
                                Three_VP,
                                Four_VP,
                                Five_VP,
                                Six_VP,
                                Seven_VP,
                                Eight_VP,
                                Nine_VP,
                                gdata_to_play10,
                            }; 
                        
uint8_t * pdata_to_play;
uint8_t lsdata_to_play=0;



/**
  * @brief  Take input from appliction layer to play specific voice as per apllication manu
  * @param  take index of specific voice menu
  * @return None.
  * @note   
  */

void PlayAudioOnGsm(uint8_t laudio_index)
{
	if( VM_MAX_NO >= laudio_index )
	{
		lsdata_to_play = laudio_index;
		VoicePlayOnGsm(1);
	}	
	else if( laudio_index <= VP_MAX_NO )
	{
		laudio_index=laudio_index-VM_MAX_NO;
    pdata_to_play=gplay_data_ref[laudio_index];	
		lsdata_to_play=0;	
		VoicePlayOnGsm(1);
	}
	
}


/**
  * @brief  Stack of Audio IC, mentain audio ic in proper state to play voice if need
  * @param  set lsts_change 1 if new command is to play or set 0 to process audio IC static
  * @return None.
  * @note   To play voice this function call contious at approoriate location
  */

void VoicePlayOnGsm(uint8_t lsts_change)
{
	static uint8_t lsvoice_play_state=3;
	uint8_t lvm_fd=0;
	
	if(lsts_change==1)
		lsvoice_play_state=0;

	switch(lsvoice_play_state)
	{
		
		case 0:
			PlayAudioFeedback(PLAYING_STARTED);
			AudioIcSpiCmd(POWER_UP);
			AudioIcSpiCmd(STOP);
			lsvoice_play_state++;
			break;
		
		case 1:
			if(lsdata_to_play != 0)
			{
				if(ReadStatusAudioIc())
				{	
					AudioIcSpiCmd(POWER_UP);
					AudioIcSpiCmd(STOP);
					ReadIntrptAudioIc();
					lvm_fd=AudioPlayVm(0 , lsdata_to_play);
					if( lvm_fd )
					lsvoice_play_state++;
				}
			}
			else
			{
				if(*pdata_to_play !=0)
				{
					if(ReadStatusAudioIc())
					{	
						AudioIcSpiCmd(POWER_UP);
						AudioIcSpiCmd(STOP);
						ReadIntrptAudioIc();
						AudioPlayVp(0 , *pdata_to_play);
						pdata_to_play++;
					}
				}
				else
				{
					lsvoice_play_state++;
				}
			}		
			break;
		case 2:
			if(ReadStatusAudioIc())
			{
				lsvoice_play_state++;
        PlayAudioFeedback(PLAYING_STOP);
			}
			break;
			
		default:

			break;
	}
	
}

/**
  * @brief  Provide feedback if any voice command is processing
  * @param  set lfeed_inpt 0 to read status or 0XFE or OXFF to Alter status
  * @return return 1 if ic buzy or return 0
  * @note   To play voice this function call contious at approoriate location
  */


uint8_t PlayAudioFeedback(uint8_t lfeed_inpt)
{
	static uint8_t lplay_rtn=0;
	if(lfeed_inpt==PLAYING_STARTED)
	{
		lplay_rtn=1;
	}
	else if(lfeed_inpt==PLAYING_STOP)
	{
		lplay_rtn=0;
	}
	return lplay_rtn;
}


