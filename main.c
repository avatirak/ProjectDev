/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2014 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include "main.h"
#include "GSM_STACK.h"
#include "EepromAppl.h"
#include "MotorAppl.h"

#define PLLCON_SETTING      CLK_PLLCON_50MHz_HXT
#define PLL_CLOCK           50000000

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/

#define MYMOBILENUMB "8806951836"

volatile unsigned char mot_int_timer=0;
extern appl_motor_state_struct appl_motor_state_st;

stgsm_rx_data struc_gsm_rx_buff;
stgsm_rcv_data struc_data_buff;
char sblank[]="                    ";

volatile unsigned int gtimer_10ms=0;
volatile unsigned int u32Counter=0,u32LedOn=0;
volatile unsigned int ggsm_alive_timer=0;
volatile unsigned int gwait_timer_10ms=0;

volatile uint32_t g_u32comRbytes = 0;
volatile uint32_t g_u32comRtail  = 0;

void Delay(int32_t ms)
{
    int32_t i;

    for(i = 0; i < ms; i++)
        CLK_SysTickDelay(1000);/* SysTick to generate the delay time and the UNIT is in us.  */
}
void TMR0_IRQHandler(void)
{
    /* Clears Timer time-out interrupt flag */
    ggsm_alive_timer++;
    mot_int_timer++;
    if(mot_int_timer>500)
    {
       mot_int_timer=0; 
    }
    gtimer_10ms++;
		gwait_timer_10ms++;
    if(gtimer_10ms>100)
        gtimer_10ms=0;
    TIMER_ClearIntFlag(TIMER0);
}

void TMR0_Init(void)
{
    /* Configure timer to operate in periodic mode and frequency = 10Hz*/
    TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 100);

    /* Enable the Timer time-out interrupt */
    TIMER_EnableInt(TIMER0);

    /* Start Timer counting */
    TIMER_Start(TIMER0);

    /* Enable TMR0 Interrupt */
    NVIC_EnableIRQ(TMR0_IRQn);
}

void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Waiting for Internal RC clock ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_OSC22M_STB_Msk);

    /* Switch HCLK clock source to Internal RC and HCLK source divide 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_HIRC, CLK_CLKDIV_HCLK(1));

    /* Enable external XTAL 12MHz clock */
    CLK_EnableXtalRC(CLK_PWRCON_XTL12M_EN_Msk);

    /* Waiting for external XTAL clock ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_XTL12M_STB_Msk);

    /* Set core clock as PLL_CLOCK from PLL */
    CLK_SetCoreClock(PLL_CLOCK);

    /* Enable IP clock */
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_EnableModuleClock(SPI0_MODULE);
    CLK_EnableModuleClock(SPI1_MODULE);
    CLK_EnableModuleClock(ADC_MODULE);
    CLK_EnableModuleClock(I2C0_MODULE);
    CLK_EnableModuleClock(TMR0_MODULE);

    /* IP clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART_S_PLL, CLK_CLKDIV_UART(1));
    CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0_S_HXT, 0);

    /* Select HXT as the clock source of SPI1 */
    CLK_SetModuleClock(SPI1_MODULE, CLK_CLKSEL1_SPI1_S_HCLK, MODULE_NoMsk);

	

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CycylesPerUs automatically. */
    //SystemCoreClockUpdate();
    PllClock        = PLL_CLOCK;            // PLL
    SystemCoreClock = PLL_CLOCK / 1;        // HCLK
    CyclesPerUs     = PLL_CLOCK / 1000000;  // For SYS_SysTickDelay()

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Set P3 multi-function pins for UART0 RXD and TXD. Set P3.4 P3.5 for I2C  */
    SYS->P3_MFP = SYS_MFP_P30_RXD0 | SYS_MFP_P31_TXD0 | SYS_MFP_P34_SDA0 | SYS_MFP_P35_SCL0;

    /* Set P1.4, P1.5, P1.6, P1.7 for SPI0 */
    SYS->P1_MFP = SYS_MFP_P14_SPISS0 | SYS_MFP_P15_MOSI_0 | SYS_MFP_P16_MISO_0 | SYS_MFP_P17_SPICLK0;

    /* Set P0.4, P0.5, P0.6, P0.7 for SPI1 */
    SYS->P0_MFP = SYS_MFP_P04_SPISS1 | SYS_MFP_P05_MOSI_1 | SYS_MFP_P06_MISO_1 |
                  SYS_MFP_P07_SPICLK1;

    /* Set P4.0, P4.1 for PWM0, PWM1, PWM2, PWM3. Set P4.6, P4.7 for ICE */
    SYS->P4_MFP = SYS_MFP_P46_ICE_CLK | SYS_MFP_P47_ICE_DAT;

}

/*---------------------------------------------------------------------------------------------------------*/
/* Init UART                                                                                               */
/*---------------------------------------------------------------------------------------------------------*/
void UART0_Init()
{
    /* RESET IP */
    SYS_ResetModule(UART0_RST);
    /* Configure UART0 and set UART0 Baudrate */
    UART_Open(UART0, 115200);
    /* Enable Interrupt and install the call back function */
    UART_ENABLE_INT(UART0, (UART_IER_RDA_IEN_Msk  ));
    NVIC_EnableIRQ(UART0_IRQn);
}


/*---------------------------------------------------------------------------------------------------------*/
/* MAIN function                                                                                           */
/*---------------------------------------------------------------------------------------------------------*/

int main(void)
{

    /* Initialize GMS releated variable */
    struc_gsm_rx_buff.comRbytes=0;
    struc_gsm_rx_buff.buffer_cnt_intrpt=0;
    struc_gsm_rx_buff.buffer_cnt_nointrpt=0;
    struc_gsm_rx_buff.buffer_to_read=0;
    
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Init System, IP clock and multi-function I/O */
    SYS_Init();
	
    /* Lock protected registers */
    SYS_LockReg();
    
	/* Configure SPI1 as a master, MSB first, 8-bit transaction, SPI Mode-0 timing, clock is 6MHz */
    SPI_Open(SPI1, SPI_MASTER, SPI_MODE_2, 8, 6000000);

    /* Enable the automatic hardware slave select function. Select the SS0 pin and configure as low-active. */
    SPI_EnableAutoSS(SPI1, SPI_SS, SPI_SS_ACTIVE_LOW);
    /* Enable FIFO mode; set TX FIFO threshold and RX FIFO threshold to 2. */
    SPI_EnableFIFO(SPI1, 2, 2);
	
    /* Init UART0 for printf */
    UART0_Init();
   
    EEPROM_Init();


    /*---------------------------------------------------------------------------------------------------------*/
    /* Init Other peripherals                                                                                  */
    /*---------------------------------------------------------------------------------------------------------*/ 
    #ifdef Debug_Code
    LCD_Init();
    LCD_EnableBackLight();
    LCD_ClearScreen();
    LCD_Print(0, " GSM TEST    ");
	#endif
    WrPswrdMobileNumb( MYMOBILENUMB , GENERIC_MOBILE_ADDR );
    appl_motor_state_st.motor_state = RdMotorStateFromEeprom();
    appl_motor_state_st.motor_timer_value = RdMotorTimerValue();
    
    TMR0_Init();


    while(1)
    {	
        GsmProcessingStack();
        MotorStateAndTiming();        
    }
}



uint8_t FuncCmdResp(const char * ldata_cmd, const char * ldata_resp)
{
	int8_t lloc_rtn=1;
	uint8_t lfunc_rtn=0;
	static uint8_t gsfunc_cmd_resp_sts=0;
			
	switch(gsfunc_cmd_resp_sts)
	{
		case STATE_ZERO:
			StringToSendGsm(ldata_cmd);
			gtimer_10ms=0;
			gsfunc_cmd_resp_sts++;
			break;
		
		case STATE_ONE:
			if(struc_data_buff.buff_flag)
			lloc_rtn=strcmp(struc_data_buff.u8RecData, ldata_resp );
			if( lloc_rtn == 0 )
			{
				gsfunc_cmd_resp_sts++;
				struc_data_buff.buff_flag=0;
			}
			else
			{
				if(gtimer_10ms>10)
				{
					gtimer_10ms=0;
					StringToSendGsm(ldata_cmd);
				}
			}
			break;
		
		case STATE_TWO:
			if(struc_data_buff.buff_flag)
			lloc_rtn=strcmp(struc_data_buff.u8RecData, "OK" );
			if( lloc_rtn == 0 )
			{
				gsfunc_cmd_resp_sts=0;
				lfunc_rtn=1;
				struc_data_buff.buff_flag=0;
			}	
			else 
			{
				if(gtimer_10ms>10)
				{
					gtimer_10ms=0;
					StringToSendGsm(ldata_cmd);
				}
			}
			break;			
	}
	return lfunc_rtn;
}



void StringToSendGsm(const char * lcdata)
{
	unsigned char lstr_len=0;
	char lsdata_string[30];
	
	strcpy(lsdata_string , lcdata);
	strcat(lsdata_string , "\r");
	lstr_len=strlen(lsdata_string);
	UART_Write(UART0,(uint8_t *)lsdata_string,lstr_len);
}


void ReadDataFromGsm(void)
{
	if(struc_gsm_rx_buff.buffer_to_read!=0)
	{
		/*-------------------------------------------*/
		NVIC_DisableIRQ(UART0_IRQn);
		struc_gsm_rx_buff.buffer_to_read--;
		strcpy( struc_data_buff.u8RecData , struc_gsm_rx_buff.struc_gsm_data[struc_gsm_rx_buff.buffer_cnt_nointrpt].u8RecData );
		struc_data_buff.buff_flag=1;
		struc_gsm_rx_buff.struc_gsm_data[struc_gsm_rx_buff.buffer_cnt_nointrpt].buff_flag=0;
		struc_gsm_rx_buff.buffer_cnt_nointrpt++;
		if(struc_gsm_rx_buff.buffer_cnt_nointrpt> (TOTAL_RX_BUFF-1))
		{
				struc_gsm_rx_buff.buffer_cnt_nointrpt=0;
		}
		NVIC_EnableIRQ(UART0_IRQn);	
		/*--------------------------------------------*/
	}
}

/*---------------------------------------------------------------------------------------------------------*/
/* ISR to handle UART Channel 0 interrupt event                                                            */
/*---------------------------------------------------------------------------------------------------------*/
void UART0_IRQHandler(void)
{
    UartIRQHandler();
	UART_ClearIntFlag(UART0,UART_ISR_RLS_INT_Msk);
}


/*---------------------------------------------------------------------------------------------------------*/
/* UART Callback function                                                                                  */
/*---------------------------------------------------------------------------------------------------------*/
void UartIRQHandler()
{
    uint8_t u8InChar = 0xFF;
    uint32_t u32IntSts = UART0->ISR;

    if(u32IntSts & UART_ISR_RDA_INT_Msk)
    {
        /* Get all the input characters */
			while(UART_IS_RX_READY(UART0))
			{
					u8InChar = UART_READ(UART0);
					if(u8InChar!='\r')
					{
							if(u8InChar != '\n')
							{
									struc_gsm_rx_buff.struc_gsm_data[struc_gsm_rx_buff.buffer_cnt_intrpt].u8RecData[struc_gsm_rx_buff.comRbytes] = u8InChar;
									if(struc_gsm_rx_buff.comRbytes<=58)
									struc_gsm_rx_buff.comRbytes++;
							}
							else
							{		
									if(struc_gsm_rx_buff.comRbytes!=0)
									{											
											struc_gsm_rx_buff.struc_gsm_data[struc_gsm_rx_buff.buffer_cnt_intrpt].u8RecData[struc_gsm_rx_buff.comRbytes] = '\0';
											struc_gsm_rx_buff.struc_gsm_data[struc_gsm_rx_buff.buffer_cnt_intrpt].buff_flag=1;
											struc_gsm_rx_buff.comRbytes=0;
											struc_gsm_rx_buff.buffer_cnt_intrpt++;
											struc_gsm_rx_buff.buffer_to_read++;
											if(struc_gsm_rx_buff.buffer_cnt_intrpt> (TOTAL_RX_BUFF-1))
											{
												struc_gsm_rx_buff.buffer_cnt_intrpt=0;
											}											
									}
							}
					}

			}
        
    }

    if(u32IntSts & UART_ISR_THRE_INT_Msk)
    {				

    }
		
}



