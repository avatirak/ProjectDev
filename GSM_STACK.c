#include "main.h"
#include "GSM_STACK.h"
#include "Audio_Driver.h"
#include "MotorStarter.h"
#include "SmsAppl.h"

#ifndef AT_COMMANDS
#define AT_COMMANDS
const char COM_AT[]=        "AT";
const char COM_ATE[]=       "ATE1";
const char COM_AT_CMEE[]=   "AT+CMEE=2";
const char COM_AT_IPR[]=    "AT+IPR=115200";
const char COM_ATI[]=       "ATI";
const char COM_GSN[]=       "AT+GSN";
const char COM_CPIN[]=      "AT+CPIN?";
const char COM_CSQ[]=       "AT+CSQ";
const char COM_CREG[]=      "AT+CREG?";
const char COM_CGREG[]=     "AT+CGREG?";
const char COM_CLCC[]=      "AT+CLCC";
const char COM_QINISTAT[]=  "AT+QINISTAT";      			    // Query state of initialization
const char COM_QTONEDET[]=  "AT+QTONEDET=1";    			    //DTMF detection on
const char COM_ATA[]=       "ATA";
const char COM_CPMS[]=      "AT+CPMS=\"SM\",\"SM\",\"SM\"";   // define sms storeage at SIM card
const char COM_CSDH[]=      "AT+CSDH=0";                      // read sam in text mode
const char COM_CSCS[]=      "AT+CSCS=\"GSM\"";                //
const char COM_CMGR[12]=    "AT+CMGR=";             	        // Read sms at  index
const char COM_CMGD[12]=    "AT+CMGD=";                         // Delete sms at  index
const char COM_CMGF[]=      "AT+CMGF=1";
const char COM_QAUDCH[]=    "AT+QAUDCH=1";                     //0 is normal audio channel (default), 1 is headset channel, 2 is loudspeaker channel
const char COM_CLVL[]=      "AT+CLVL=80";                      //Loud speaker volume level
const char COM_QSIDET[]=    "AT+QSIDET=100";                   // Change the side tone gain level
const char COM_QMIC[]=      "AT+QMIC=1,10";                    //microphone gain level
const char COM_ATS0[]=		"ATS0=2";						   //rings before automatically answering
const char COM_ATH[]=       "ATH";                             //Disconnect exixting call
const char COM_ATD[]=		"ATD";							   //Dialing a number
#endif

    stmulti_commd StrucGsmInitArray[]={   {COM_AT, COM_AT},               {COM_ATE, COM_ATE}, 
                                            {COM_AT_CMEE , COM_AT_CMEE}, 	{COM_AT_IPR , COM_AT_IPR}, 														
                                            {COM_ATI, COM_ATI},             {COM_GSN, COM_GSN},
                                            {COM_CPIN, "+CPIN: READY"}, 	{COM_CSQ , COM_CSQ}
                                           };

    stmulti_commd StrucReadAllSmsArray[]={   {COM_ATE, COM_ATE}, 		{COM_CMGF, COM_CMGF },
                                        {COM_CSCS, COM_CSCS},	    {COM_CSDH, COM_CSDH}	
                                    };
                                    
    stmulti_commd StrucCommonCallStateArray[]= {   {COM_ATE, COM_ATE},     {COM_QAUDCH,COM_QAUDCH},
                                            {COM_CLVL,COM_CLVL},     {COM_QSIDET, COM_QSIDET},
                                            {COM_QMIC,COM_QMIC},    {COM_QTONEDET,COM_QTONEDET}
                                        };
    

static dtmf_st dtmf_data={0,0,0,0,0};		
static stgsm_call_data_struct lsgsm_call_rx_data;																				
																				
extern stgsm_rcv_data struc_data_buff;
extern char sblank[];

extern volatile unsigned int gtimer_10ms;
extern volatile unsigned int u32Counter,u32LedOn;
extern volatile unsigned int ggsm_alive_timer;
extern volatile unsigned int gwait_timer_10ms;


void GsmProcessingStack(void)
{
    static uint8_t lsgsm_stack_state=0;
    
    ReadDataFromGsm();
    
    switch(lsgsm_stack_state)
    {
        case STATE_ZERO:
            if( GsmInitializaion() )
            lsgsm_stack_state++;
            break;
        
        case STATE_ONE:
            if(ReadAllSms())
            lsgsm_stack_state++;    
            break;
        
        case STATE_TWO:
            if(!( GsmStateRunning() ) )
            {
                lsgsm_stack_state=0;
            }
            break;
        
        default:
            break;
    }
	
}


uint8_t GsmInitializaion(void)
{
    static uint8_t lsgsm_init_state=0;
    char lgsm_init_trm=0;
    uint8_t lstate_succeded=0;
    uint8_t lstate_flow=0;
    static uint8_t lsindex_init=0;
	

    switch(lsgsm_init_state)
    {
        case STATE_ZERO:
            if(lsindex_init < 8)
            {
                if( FuncCmdResp(StrucGsmInitArray[lsindex_init].cmd_at ,StrucGsmInitArray[lsindex_init].resp_at) )
                {
                    lsindex_init++;
                    lstate_succeded=1;
                }
                lstate_flow=WaitStateLogic( lsindex_init , lstate_succeded );                         
                if(lstate_flow==2)
                lsindex_init=0;
            }
            else 
            {
                lsgsm_init_state++;
                lsindex_init=0;
            }
            break;
            
        case STATE_ONE:
            StringToSendGsm(COM_CREG);                                   /* use AT+CREG? /AT+CGREG? to query the network registration status */
            lsgsm_init_state++;
            break;	
				
        case STATE_TWO:
            if(struc_data_buff.buff_flag)
            {
                if(!(strcmp(struc_data_buff.u8RecData , "+CREG: 0,1" ) ) )
                {
                    lsgsm_init_state++;  
                    lstate_succeded=1;
                    struc_data_buff.buff_flag=0;
                }

                else if(!(strcmp(struc_data_buff.u8RecData , "+CREG: 0,5" ) ) )
                {
                    lsgsm_init_state++;  
                    lstate_succeded=1;   
                    struc_data_buff.buff_flag=0;
                }
            }
            lstate_flow=WaitStateLogic( lsgsm_init_state , lstate_succeded );            
            if(lstate_flow==1)
                lsgsm_init_state--;                                          
            if(lstate_flow==2)
                lsgsm_init_state=0;                                                               // Terminate Outging call       
            break;
            
        case STATE_THREE:
            StringToSendGsm(COM_QINISTAT);													/* use AT+QINISTAT to check SMS initial status */
            lsgsm_init_state++; 
            break;

        case STATE_FOUR:
            if(struc_data_buff.buff_flag)
            {
                if(('\0'!=strstr( struc_data_buff.u8RecData , "+QINISTAT: 3" )))
                {
                    lstate_succeded=1;
                    lsgsm_init_state++;
                    struc_data_buff.buff_flag=0;
                }
                else if(('\0'!=strstr( struc_data_buff.u8RecData , "+QINISTAT: 2" )))
                {
                    lstate_succeded=1;
                    lsgsm_init_state++;
                    struc_data_buff.buff_flag=0;
                }
                else if (('\0'!=strstr( struc_data_buff.u8RecData , "+QINISTAT: 1" )))
                {
                    lstate_succeded=1;
                    lsgsm_init_state=0;
                    struc_data_buff.buff_flag=0;
                }
                else if (('\0'!=strstr( struc_data_buff.u8RecData , "+QINISTAT: 0" )))
                {
                    lstate_succeded=1;
                    lsgsm_init_state=0;
                    struc_data_buff.buff_flag=0;
                }
            }

            lstate_flow=WaitStateLogic( lsgsm_init_state , lstate_succeded );            
            if(lstate_flow==1)
                lsgsm_init_state--;                                        
            if(lstate_flow==2)
                lsgsm_init_state=0;
            break;	
                  
        default:
            #ifdef Debug_Code
            LCD_Print(0, " GSM TEST Done");		
            #endif
            lgsm_init_trm=1;
            lsgsm_init_state=0;
            break;   
     }
	return lgsm_init_trm;
}

uint8_t ReadAllSms(void)
{
    static uint8_t lsread_all_sms_state=0;
    static uint8_t lssms_numb_read=0;
    static uint8_t lssms_to_read=1;
    char * pinpt_string;
    char lsms_read_trm=0;
    static uint8_t lsindex_all_sms=0;
    uint8_t lstate_succeded=0;
    uint8_t lstate_flow=0;

    switch(lsread_all_sms_state)
    {
        case STATE_ZERO:
            if(lsindex_all_sms < 4)
            {
                if( FuncCmdResp(StrucReadAllSmsArray[lsindex_all_sms].cmd_at ,StrucReadAllSmsArray[lsindex_all_sms].resp_at) )
                {
                    lsindex_all_sms++;
                    lstate_succeded=1;
                }
                
                lstate_flow=WaitStateLogic( lsindex_all_sms , lstate_succeded );                         
                if(lstate_flow==2)
                    lsindex_all_sms=0;
            }
            else 
            {
                lsread_all_sms_state++;
                lsindex_all_sms=0;
            }
            break;              

        case STATE_ONE:
            StringToSendGsm(COM_CPMS);                                           //inquire the information of short messages  AT+CPMS="SM","SM","SM"
            lsread_all_sms_state++;
            lssms_to_read=1;                                              			// Initializing SMS index for reding purpose
            break;

        case STATE_TWO:
            if(struc_data_buff.buff_flag)
            {
                pinpt_string=strstr(struc_data_buff.u8RecData,"+CPMS: ");                                  //+CPMS: 8,50,8,50,8,50   
                if(pinpt_string!='\0')
                {
                    pinpt_string=pinpt_string+7;
                    while(*pinpt_string != ',')
                    {
                        pinpt_string ++;
                    }
                    *pinpt_string='\0';
                    lssms_numb_read=atoi(struc_data_buff.u8RecData+7);                                    // struc_data_buff+7 for "+CPMS: "
                    lsread_all_sms_state++;
                    lstate_succeded=1;
                    struc_data_buff.buff_flag=0;
                }
            }
            lstate_flow=WaitStateLogic( lsread_all_sms_state , lstate_succeded );  
            if(lstate_flow==1)	
            lsread_all_sms_state--;
            break;

        case STATE_THREE:
            if(lssms_numb_read!=0)
            {
                if( ReadNewSms(lssms_to_read) )
                lssms_to_read++;

                if( lssms_to_read > (lssms_numb_read+1) )                
                lsread_all_sms_state++;                 
            }
            else
            {
                lsread_all_sms_state++;
            }					
            break;

        default:
            lsms_read_trm=1;
            lsread_all_sms_state=0;
            break;			
    }
    return lsms_read_trm;
}

char ReadNewSms(unsigned char SMS_NUM)
{
    char sms_data_string[60];
	static char lssms_cmd_string[]="    ";
	char * lpsms_cmd_string;
	static char lssms_read_commd[]="                    ";
	char * lpsms_read_commd;
    static unsigned char lssms_new_fetch_state=0;
	unsigned char lsms_numb_hundrd=0;
	unsigned char lsms_numb_tenth=0;
	unsigned char lthird_quate=0;   		                     //This variable is to find lthird_quate (") in SMS respond stream to identify mobile number
	char lsms_mobile_numb[14];
	char * psms_mobile_numb;
    uint8_t lread_new_sms_trm=0;
	
	
	switch(lssms_new_fetch_state)
	{
        case STATE_ZERO:		
            lpsms_cmd_string=lssms_cmd_string;
            if(SMS_NUM!=0)
            {
                if(SMS_NUM>99 )
                {
                    lsms_numb_hundrd=SMS_NUM/100;
                    SMS_NUM=SMS_NUM%100;
                    lsms_numb_tenth=SMS_NUM/10;
                    SMS_NUM=SMS_NUM%10;
                    *lpsms_cmd_string=lsms_numb_hundrd+48;
                    lpsms_cmd_string++;
                    *lpsms_cmd_string=lsms_numb_tenth+48;
                    lpsms_cmd_string++;
                }
                else if(SMS_NUM>9 )
                {
                    lsms_numb_tenth=SMS_NUM/10;
                    SMS_NUM=SMS_NUM%10;
                    *lpsms_cmd_string=lsms_numb_tenth+48;
                    lpsms_cmd_string++;
                }
                *lpsms_cmd_string=SMS_NUM+48;
                lpsms_cmd_string++;
                *lpsms_cmd_string='\0';
            }
            strcpy(lssms_read_commd , COM_CMGR);
            strcat(lssms_read_commd , lssms_cmd_string);
            lssms_new_fetch_state++;
            break;
			
        case STATE_ONE:
            StringToSendGsm(lssms_read_commd );
            lssms_new_fetch_state++;
            gtimer_10ms=0;
            break;
			
        case STATE_TWO:
            psms_mobile_numb = lsms_mobile_numb;
            if(struc_data_buff.buff_flag)
            {
                lpsms_read_commd=strstr(struc_data_buff.u8RecData,"+CMGR: ");		
                if(lpsms_read_commd!='\0')
                {
                    while( (*lpsms_read_commd!='\0') && (lthird_quate!=3) )
                    {
                        if(*lpsms_read_commd=='\"')
                        {
                            lthird_quate++;
                        }
                        lpsms_read_commd++;
                    }
                    while( (*lpsms_read_commd!='\"') && (*lpsms_read_commd!='\0') )
                    {
                        *psms_mobile_numb = *lpsms_read_commd;
                        psms_mobile_numb++;
                        lpsms_read_commd++;
                    }
                    *psms_mobile_numb='\0';
                    #ifdef Debug_Code
                    LCD_Print(1, lsms_mobile_numb);
                    #endif
                    lssms_new_fetch_state++;
                    gtimer_10ms=0;
                    struc_data_buff.buff_flag=0;
                }
            }

            if(gtimer_10ms > 50)
            {
                gtimer_10ms=0;
                if(struc_data_buff.buff_flag)
                {    
                    if(!(strcmp(struc_data_buff.u8RecData , "OK" ) ) )
                    {
                        lssms_new_fetch_state=10;  
                        struc_data_buff.buff_flag=0;                        
                    }
                }
                lssms_new_fetch_state--;
            }					
          
            break;
            
        case STATE_THREE:
            if(struc_data_buff.buff_flag)
            {
                lpsms_read_commd=strstr(struc_data_buff.u8RecData,"+CMGR: ");
                if(lpsms_read_commd == '\0')
                {
                    strcpy(sms_data_string , struc_data_buff.u8RecData);
                    #ifdef Debug_Code
                    LCD_Print(2, sms_data_string);	  
                    #endif
                    lssms_new_fetch_state++;
                    struc_data_buff.buff_flag=0;
                    
                }
            }
            break;
					
        case STATE_FOUR:
            if(struc_data_buff.buff_flag)
            {
                lpsms_read_commd=strstr(struc_data_buff.u8RecData,"OK");
                if(lpsms_read_commd != '\0')
                {	               
                    
                }
                else
                {
                    strcpy(sms_data_string , struc_data_buff.u8RecData); 
                }
                lssms_new_fetch_state=lssms_new_fetch_state+1;
                struc_data_buff.buff_flag=0;
                
                SmsAppplicationFunc(lsms_mobile_numb ,sms_data_string);
            }
            break;
											
        case STATE_FIVE:
            strcpy(lssms_read_commd , COM_CMGD);                        //  Delete read SMS
            strcat(lssms_read_commd , lssms_cmd_string);
            StringToSendGsm(lssms_read_commd );
            lssms_new_fetch_state++;
            break;
            
        case STATE_SIX: 
            if(struc_data_buff.buff_flag)
            {
                lpsms_read_commd=strstr(struc_data_buff.u8RecData,"AT+CMGD=");		
                if(lpsms_read_commd!='\0')
                {
                    lssms_new_fetch_state++;
                    struc_data_buff.buff_flag=0;
                }
            }
            break;
            
        default:
            lssms_new_fetch_state=0;
            lread_new_sms_trm=1;
            break;		
	}	
    return lread_new_sms_trm;
}

uint8_t GsmStateRunning(void)
{
	static uint8_t lsgsm_run_state=0;
    static char lssms_numb_to_read;
    uint8_t lgsm_run_state_rtn=1;
	char * pinpt_string;
	
	switch(lsgsm_run_state)
	{
		
		case STATE_ONE:
            if(GsmCallRxStateFunc())
            lsgsm_run_state=4;
            break;
		
		case STATE_TWO:
            if(GsmSmsRxStateFunc( lssms_numb_to_read ))
			lsgsm_run_state=4;		
			break;
            
        case STATE_THREE:   
            GsmCallTxStateFunc ();
            break;
		
		default:  
            if(!( GsmAliveStatusCheckFunc () ) )
            {
                lgsm_run_state_rtn=0;             // GSM STop responding take appropriate action like reinitiate      
            }
            else
            {
                    #ifdef Debug_Code
                    LCD_Print(0, " Phone Ideal  ");
                    #endif
            }
							
            if(struc_data_buff.buff_flag)
            {
                if( ! (strcmp(struc_data_buff.u8RecData, "RING" ) ) )
                {
                    lsgsm_run_state=1;
                    struc_data_buff.buff_flag=0;
                    break;
                }
                
                pinpt_string=strstr( struc_data_buff.u8RecData , "+CMTI: \"SM\"" );         // +CMTI: "SM",1
                if(pinpt_string!='\0')
                {
                    while( (*pinpt_string != ',') && (*pinpt_string != '\0') )
                    {
                        pinpt_string++;
                    }
                    pinpt_string++;
                    lssms_numb_to_read=atoi(pinpt_string);
                    lsgsm_run_state=2;
                    struc_data_buff.buff_flag=0;
                    break;
                }
            }

            break;
	}
    
    return lgsm_run_state_rtn;
}

uint8_t GsmCallTxStateFunc ()
{
    static uint8_t lsgsm_tx_call_state=0;
    uint8_t lstate_flow;
    uint8_t lstate_succeded=0;
	uint8_t lgsm_tx_call_trm=0;
    static uint8_t lsindex_call_tx=5;
    char lmobile_numb_dial[]="             ";
	
    
    switch( lsgsm_tx_call_state )
    {
        case STATE_ZERO:
            if(lsindex_call_tx < 8)
            {
                if( FuncCmdResp(StrucGsmInitArray[lsindex_call_tx].cmd_at ,StrucGsmInitArray[lsindex_call_tx].resp_at) )
                {
                    lsindex_call_tx++;
                    lstate_succeded=1;
                }
                lstate_flow=WaitStateLogic( lsindex_call_tx , lstate_succeded );                         
                if(lstate_flow==2)
                lsindex_call_tx=0;
            }
            else 
            {
                lsgsm_tx_call_state++;
                lsindex_call_tx=5;
            }                          
            break;
        
        case STATE_ONE:
            StringToSendGsm(COM_CREG);                                   
            lsgsm_tx_call_state++;       
            break;
        
        case STATE_TWO:
            if(struc_data_buff.buff_flag)
            {  
                if(!(strcmp(struc_data_buff.u8RecData , "+CREG: 0,1" ) ) )
                {
                    lsgsm_tx_call_state++;  
                    lstate_succeded=1;  
                    struc_data_buff.buff_flag=0;
                }

                else if(!(strcmp(struc_data_buff.u8RecData , "+CREG: 0,5" ) ) )
                {
                    lsgsm_tx_call_state++;  
                    lstate_succeded=1;  
                    struc_data_buff.buff_flag=0;
                }
            }
            
            lstate_flow=WaitStateLogic( lsgsm_tx_call_state , lstate_succeded );            
            if(lstate_flow==1)
            lsgsm_tx_call_state--;                                         // Terminate Outging call              
            if(lstate_flow==2)
            lsgsm_tx_call_state=16;                                         // Terminate Outging call     
            break;
            
        case STATE_THREE:
            lstate_flow=CallStateCommonCommd();
            if(lstate_flow==1)
            lsgsm_tx_call_state++;
            else if(lstate_flow==2)
            lsgsm_tx_call_state=16;  
            break;

        case STATE_FOUR:
            strcpy(lmobile_numb_dial, COM_ATD);
            break;
            
        case STATE_FIVE:		
            if(FuncCmdResp(lmobile_numb_dial, lmobile_numb_dial))
            {
                lsgsm_tx_call_state++;
                lstate_succeded=1;
            }
            lstate_flow=WaitStateLogic( lsgsm_tx_call_state , lstate_succeded );                         
            if(lstate_flow==2)
            lsgsm_tx_call_state=1;
            break;
					
        default:
            lsgsm_tx_call_state=0;
            lgsm_tx_call_trm=1;
            break;
					
    }
	return lgsm_tx_call_trm;
}

uint8_t GsmCallRxStateFunc(void)
{
	
    static uint8_t lsgsm_rx_call_state=0;
    static uint16_t  lscall_state_ideal_wait=0;
    char * pinpt_string;
    char lmobile_numb[15];
    char *pmobile_numb;
    int ldtmf_value;
    uint8_t lindex_call_rx=0;
    uint8_t lstate_flow=0;
    uint8_t lstate_succeded=0;
    uint8_t lgsm_rx_call_trm=0;
	
    switch(lsgsm_rx_call_state)
    {
        case STATE_ZERO:
            #ifdef Debug_Code
            LCD_Print(0, " CALL Receive  ");
            #endif
            AudioIcSpiCmd(RESET);
            AudioIcSpiCmd(POWER_UP);
            lsgsm_rx_call_state++;
            lsgsm_call_rx_data.currt_call_sts = UNKNOWN;	
            lsgsm_call_rx_data.call_dir=IN_COMING;
            break;
            
        case STATE_ONE:
            lindex_call_rx=CallStateCommonCommd();
            if(lindex_call_rx==1)
            lsgsm_rx_call_state++;
            else if(lindex_call_rx==2)
            lsgsm_rx_call_state=15;   
        					
        case STATE_TWO:
            if(struc_data_buff.buff_flag)
            {
                if( ! (strcmp(struc_data_buff.u8RecData, "RING" ) ) )
                {
                    lsgsm_rx_call_state++;
                    struc_data_buff.buff_flag=0;
                }
            }
            if( ggsm_alive_timer > 850)
            {
                ggsm_alive_timer=0;
                lsgsm_rx_call_state=15;
            }
            break;
					
						
        case STATE_THREE:
            StringToSendGsm(COM_ATA);
            lsgsm_rx_call_state++;			
            break;
            
        case STATE_FOUR: 
            if(struc_data_buff.buff_flag)
            {
                if( strcmp( struc_data_buff.u8RecData , "OK" ) )
                {
                    lsgsm_rx_call_state++;	 
                    lstate_succeded=1;	
                    struc_data_buff.buff_flag=0;                    
                }
                else if ( strcmp( struc_data_buff.u8RecData , "NO CARRIER" ) )
                {
                    lsgsm_rx_call_state=15;
                    lstate_succeded=1;   
                    struc_data_buff.buff_flag=0;                    
                }
            }
            
            lstate_flow=WaitStateLogic( lsgsm_rx_call_state , lstate_succeded);
            if(lstate_flow==1)
            lsgsm_rx_call_state--;   
            else if(lstate_flow==2)
            lsgsm_rx_call_state=15;                                         // Terminate Outging call
            lscall_state_ideal_wait =0; 
						
            break;
						
        case STATE_FIVE:
            StringToSendGsm(COM_CLCC);
            lsgsm_rx_call_state++;
            break;
        
        case STATE_SIX:
            if(struc_data_buff.buff_flag)
            {
                pinpt_string=strstr(struc_data_buff.u8RecData,"+CLCC: ");
                if(pinpt_string!='\0')
                {
                    pinpt_string= strstr(struc_data_buff.u8RecData,"\"");
                    pinpt_string++;
                    lindex_call_rx=0;
                    while(*pinpt_string!='\"')
                    {
                        lmobile_numb[lindex_call_rx]= *pinpt_string;
                        lindex_call_rx++;
                        pinpt_string++;
                    }
                    lmobile_numb[lindex_call_rx]='\0';	
                    pmobile_numb=lmobile_numb;
                    #ifdef Debug_Code                             
                    LCD_Print(3, lmobile_numb);	
                    #endif
                    if(lindex_call_rx>10)                           // This is to exact exact mobile number with out 0 and 91
                    {
                       pmobile_numb= pmobile_numb+(lindex_call_rx-10);
                    }
                    lsgsm_rx_call_state++;                   
                    strcpy(lsgsm_call_rx_data.mobile_numb, pmobile_numb);
                    lsgsm_call_rx_data.currt_call_sts=ACTIVE;	
                    lsgsm_call_rx_data.call_dir=IN_COMING;								
                    lstate_succeded=1;
                    struc_data_buff.buff_flag=0;
                }
            }
            lstate_flow=WaitStateLogic( lsgsm_rx_call_state , lstate_succeded);
            if(lstate_flow==1)
            lsgsm_rx_call_state--;   
            else if(lstate_flow==2)
            lsgsm_rx_call_state=15;                                         
            break;						
             
        case STATE_SEVEN:
            if(struc_data_buff.buff_flag)
            {
                pinpt_string=strstr(struc_data_buff.u8RecData,"+QTONEDET: ");
                if(pinpt_string != '\0')
                {
                    lscall_state_ideal_wait=0;               
                    pinpt_string=pinpt_string+11;
                    ldtmf_value=atoi(pinpt_string);
                    struc_data_buff.buff_flag=0;
                    switch(ldtmf_value)
                    {
                        case 49:
                        u32LedOn = 0x01;
                        GPIO_SET_OUT_DATA(P2, ~u32LedOn); /* Set GPIO Port OUT Data */

                        break;
                        case 50:
                        u32LedOn = 0x02;
                        GPIO_SET_OUT_DATA(P2, ~u32LedOn); /* Set GPIO Port OUT Data */

                        break;
                        case 51:
                        u32LedOn = 0x04;
                        GPIO_SET_OUT_DATA(P2, ~u32LedOn); /* Set GPIO Port OUT Data */
                                            
                        break;
                        case 52:
                        u32LedOn = 0x08;
                        GPIO_SET_OUT_DATA(P2, ~u32LedOn); /* Set GPIO Port OUT Data */
                                            
                        break;
                        case 53:
                        u32LedOn = 0x10;
                        GPIO_SET_OUT_DATA(P2, ~u32LedOn); /* Set GPIO Port OUT Data */
                        break;
                        
                        case 54:
                        u32LedOn = 0x20;
                        GPIO_SET_OUT_DATA(P2, ~u32LedOn); /* Set GPIO Port OUT Data */
                        break;
                        
                        case 55:
                        u32LedOn = 0x40;
                        GPIO_SET_OUT_DATA(P2, ~u32LedOn); /* Set GPIO Port OUT Data */
                        break;
                        
                        case 56:
                        u32LedOn = 0x80;
                        GPIO_SET_OUT_DATA(P2, ~u32LedOn); /* Set GPIO Port OUT Data */
                        break;
                        
                        case 57:
                        u32LedOn = 0x03;
                        GPIO_SET_OUT_DATA(P2, ~u32LedOn); /* Set GPIO Port OUT Data */
                        break;
                        
                        case 48:
                        u32LedOn = 0x07;
                        GPIO_SET_OUT_DATA(P2, ~u32LedOn); /* Set GPIO Port OUT Data */
                        break;
                    }								            //LCD_Print(0, pinpt_string);
                    DtmfValuePush( ldtmf_value );                
                }
            }
            if(gtimer_10ms > 50)
            {
                gtimer_10ms =0;
                lscall_state_ideal_wait++;
                if(lscall_state_ideal_wait > 360)
                {
                   lscall_state_ideal_wait=0;
                   lsgsm_rx_call_state=14;
                }
            } 
						// "CALL_END" will set if TerminateGsmCall fuction is called from AudioPlay.c
						if( lsgsm_call_rx_data.currt_call_sts == CALL_END )  
						{
							lsgsm_rx_call_state=15;
						}
            if(struc_data_buff.buff_flag)
            {
                if( ! (strcmp(struc_data_buff.u8RecData,"NO CARRIER" ) ) )
                {
                   
                    lsgsm_rx_call_state=15;
                    struc_data_buff.buff_flag=0;
									  lsgsm_call_rx_data.currt_call_sts = CALL_END;	
										lsgsm_call_rx_data.call_dir=IDEAL;
                }
            }
            break;
            
        case STATE_FOURTEEN: 
            if( TerminateGsmCall() )
            {							
                lsgsm_rx_call_state++;
            }							
            break;        
            
        default:
            lsgsm_rx_call_state=0;
            lgsm_rx_call_trm=1;
            break;
    }	
        
    GsmApplCallFunc(lsgsm_call_rx_data);
    VoicePlayOnGsm(0);
    return lgsm_rx_call_trm;        
}


uint8_t GsmAliveStatusCheckFunc(void)
{
    uint8_t lalive_state_rtn=1;
    static stgsm_live_struct lsgsm_live_state;
    
    if( ggsm_alive_timer >= 500 )                               // To check if GSM is alive
    {
        ggsm_alive_timer=0;
        StringToSendGsm(COM_AT); 
        lsgsm_live_state.live_cmd_send=cTrue;
    }

    if(struc_data_buff.buff_flag)
    {
        if( ! (strcmp(struc_data_buff.u8RecData, "OK" ) ) )
        {
            strcpy(struc_data_buff.u8RecData , sblank);
            lsgsm_live_state.live_cmd_send=cFalse;
            lsgsm_live_state.live_count=0;
            struc_data_buff.buff_flag=0;
        }
    }
    if( lsgsm_live_state.live_cmd_send==cTrue ) 
    {
        lsgsm_live_state.live_cmd_send=cFalse;
        lsgsm_live_state.live_count++;       
        if(lsgsm_live_state.live_count > 6)
        {
            lsgsm_live_state.live_count =0;
            lalive_state_rtn=0;
            #ifdef Debug_Code
            LCD_Print(0, "   GSM NOT RSP   ");
            #endif
            //Take Appropriate action to solve GSM disconnection
        }
    } 
    return  lalive_state_rtn;   
}


uint8_t CallStateCommonCommd(void)
{
    
    uint8_t lstate_flow=0;
    uint8_t lstate_succeded=0;
    uint8_t lcall_state_common_trm=0;
    static uint8_t lsindex_call_common=0;

    if(lsindex_call_common < 6)
    {
        if( FuncCmdResp(StrucCommonCallStateArray[lsindex_call_common].cmd_at ,StrucCommonCallStateArray[lsindex_call_common].resp_at) )
        {
            lsindex_call_common++;
            lstate_succeded=1;
        }
        lstate_flow=WaitStateLogic( lsindex_call_common , lstate_succeded );                         
        if(lstate_flow==2)
        lsindex_call_common=0;
    }
    else 
    {
        lcall_state_common_trm=1;
        lsindex_call_common=0;
    }

return lcall_state_common_trm;
}


uint8_t GsmSmsRxStateFunc( uint8_t SMS_Number_Read)
{
 	static uint8_t lsgsm_sms_rx_state=0;
    static uint8_t lsindex_sms_state=0;   
    uint8_t lstate_flow=0;
    uint8_t lstate_succeded=0;
    uint8_t lgsm_sms_rx_trm=0;
    
    switch(lsgsm_sms_rx_state)
    {
        case STATE_ZERO:
            if(lsindex_sms_state < 4)
            {
                if( FuncCmdResp(StrucReadAllSmsArray[lsindex_sms_state].cmd_at ,StrucReadAllSmsArray[lsindex_sms_state].resp_at) )
                {
                    lsindex_sms_state++;
                    lstate_succeded=1;
                }

                lstate_flow=WaitStateLogic( lsindex_sms_state , lstate_succeded );                         
                if(lstate_flow==2)
                lsindex_sms_state=0;
            }
            else 
            {
                lsgsm_sms_rx_state++;
                #ifdef Debug_Code
                LCD_Print(0, "   SMS Receive   ");
                #endif
                lsindex_sms_state=0;
            }
            break;

        case STATE_ONE:
            if( ReadNewSms(SMS_Number_Read) )
            {
                lsgsm_sms_rx_state=0; 
                lgsm_sms_rx_trm=1;
            }							
            break;						
    }
    return lgsm_sms_rx_trm;
}

uint8_t WaitStateLogic(uint8_t CURR_STATE , uint8_t SUCCIDE )
{
    static uint8_t PREV_STATE=220;  
    static uint8_t STATE_RETRAY_COUNT=0;
    uint8_t FEED_WAIT=0;
    
    if( (PREV_STATE != CURR_STATE) &&  (SUCCIDE!=1) )                //when monitering state is successful then then SUCCIDE =1
    {
        gwait_timer_10ms=0;
        PREV_STATE=CURR_STATE;
        STATE_RETRAY_COUNT=0;
    }
    if(SUCCIDE==1)
    {
        gwait_timer_10ms=0;
        PREV_STATE=220;
        STATE_RETRAY_COUNT=0;   
    }  
    if(gwait_timer_10ms> 30)
    {
        gwait_timer_10ms=0;
        FEED_WAIT=1;
        STATE_RETRAY_COUNT++;
        if(STATE_RETRAY_COUNT >=3)
        {
            FEED_WAIT=2;
            STATE_RETRAY_COUNT=0;
        }        
    }
    return FEED_WAIT;
}

void DtmfValuePush( unsigned char d_value )
{
	unsigned char index= 0;
	if( dtmf_data.push_index <= 3 )
	{
		dtmf_data.array[ dtmf_data.push_index ] = d_value;
		dtmf_data.push_index++;
	}
	else
	{
		for( index= 0 ; index<=2; index++ )
		{
			dtmf_data.array[ index ] = dtmf_data.array[ index + 1];
		}
		dtmf_data.array[3] = d_value;		
	}
}
unsigned char DtmfValuePop( void )
{
	unsigned char d_value=0,index=0;
	d_value = dtmf_data.array[0];
	for( index= 0 ; index<=2; index++ )
	{
		if( dtmf_data.array[ index + 1] != 0 )
		{
			dtmf_data.array[ index ] = dtmf_data.array[ index + 1];
		}
	}
	dtmf_data.array[3]=0;
	if( dtmf_data.push_index != 0)
	{
		dtmf_data.push_index--;
	}
	return d_value;
}

unsigned char TerminateGsmCall(void)
{
	unsigned char call_term=0;
	if( FuncCmdResp(COM_ATH,"ATH") )
	{							
		lsgsm_call_rx_data.currt_call_sts = CALL_END;	
		lsgsm_call_rx_data.call_dir=IDEAL;
		call_term=1;
	}	
	return call_term;
}
