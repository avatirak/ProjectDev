/**************************************************************************//**
 * @file     spi_flash_fifo.c
 * @version  V0.10
 * $Revision: 4 $
 * $Date: 14/01/28 10:49a $
 * @brief    M051 series SPI Flash driver source file
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include "M051Series.h"
#include "Audio_Driver.h"
#include "AUDIO_APP.h"
#include <stdio.h>

//#define SPI_Degug

#ifdef SPI_Degug
#include "lcd_Driver.h"

char gspi_value[15] = "SPI:";                         /* this variable are used for debuging purpose  */
char str_clear_spi_value[15] = "SPI:           ";      /* this variable are used for debuging purpose  */
#endif


staudio_commands saudio_cmd; 
staudio_commands saudio_rsp;

uint8_t gspi_audio_cmd[]={0X2A, 0X10 , 0X12 , 0X14 };    //  0x2A= STOP_AUDIO  , 0X10 Power up

staudio_cmd_size gread_status={0X40,3};
staudio_cmd_size gread_intrpt={0X46,3};
staudio_cmd_size gplay_vp={0XA6,3};                    //gplay_vp=command,HighIndex,LowIndex
staudio_cmd_size gexe_vm={0XB0,3};                     //gexe_vm=Command,HighIndex,LowIndex
staudio_cmd_size gplay_sl_audio_ic={0XA8,2};		    		//Play_Sl=Command,NumberOfTimes*32ms


/**
  * @brief  Perform voice prompt play on audio ic
  * @param  take lower and higher index of VP
  * @return None.
  * @note   
  */
uint8_t AudioPlayVp(uint8_t lhigh_index , uint8_t llower_index)
{
    saudio_cmd.sarray[0]=gplay_vp.audio_commd;
    saudio_cmd.sarray[1]=lhigh_index;
    saudio_cmd.sarray[2]=llower_index;
    saudio_cmd.array_size=gplay_vp.audio_commd_size;
    SpiAudioICReadWrite(SPI1, saudio_rsp.sarray ,saudio_cmd.sarray ,saudio_cmd.array_size); 
    #ifdef SPI_Degug
    sprintf(gspi_value + 4, "%d %d %d ",  saudio_rsp.sarray[0],saudio_rsp.sarray[1],saudio_rsp.sarray[2]);
    LCD_Print(1, str_clear_spi_value);
    LCD_Print(1, gspi_value);
    #endif
    return 0;
}

/**
  * @brief  Perform voice micro play on audio ic
  * @param  take lower and higher index of VM
  * @return None.
  * @note   
  */

uint8_t AudioPlayVm(uint8_t lhigh_index , uint8_t llower_index)
{
    saudio_cmd.sarray[0]=gexe_vm.audio_commd;
    saudio_cmd.sarray[1]=lhigh_index;
    saudio_cmd.sarray[2]=llower_index;
    saudio_cmd.array_size=gexe_vm.audio_commd_size;
    SpiAudioICReadWrite(SPI1, saudio_rsp.sarray ,saudio_cmd.sarray ,saudio_cmd.array_size); 
    #ifdef SPI_Degug
    sprintf(gspi_value + 4, "%d %d %d ",  saudio_rsp.sarray[0],saudio_rsp.sarray[1],saudio_rsp.sarray[2]);
    LCD_Print(1, str_clear_spi_value);
    LCD_Print(1, gspi_value);
    #endif
		if(saudio_rsp.sarray[0] == 0X20)
			return 1;
		else if ( saudio_rsp.sarray[0] == 0XC0 )
		  return 0;
		else
		  return 2;
}

/**
  * @brief  Check the status of Voice ic , playing or ideal
  * @param  None
  * @return if playing return 0 or return 1
  * @note   
  */
uint8_t ReadStatusAudioIc(void)
{
    saudio_cmd.sarray[0]=gread_status.audio_commd;
    saudio_cmd.sarray[1]=0;
    saudio_cmd.sarray[2]=0;
    saudio_cmd.array_size=gread_status.audio_commd_size;
    SpiAudioICReadWrite(SPI1, saudio_rsp.sarray ,saudio_cmd.sarray ,saudio_cmd.array_size); 
    #ifdef SPI_Degug
    sprintf(gspi_value + 4, "%d %d %d ",  saudio_rsp.sarray[0],saudio_rsp.sarray[1],saudio_rsp.sarray[2]);
    LCD_Print(1, str_clear_spi_value);
    LCD_Print(1, gspi_value);
	#endif
    
    if(saudio_rsp.sarray[0]==48 || saudio_rsp.sarray[0]==192)
    return 1;
    else
    return 0;
}


/**
  * @brief  Clear the intrupt of audio IC if any
  * @param  None
  * @return 0
  * @note   
  */

uint8_t ReadIntrptAudioIc(void)
{
    saudio_cmd.sarray[0]=gread_intrpt.audio_commd;
    saudio_cmd.sarray[1]=0;
    saudio_cmd.sarray[2]=0;
    saudio_cmd.array_size=gread_intrpt.audio_commd_size;
    SpiAudioICReadWrite(SPI1, saudio_rsp.sarray ,saudio_cmd.sarray ,saudio_cmd.array_size); 
    #ifdef SPI_Degug
    sprintf(gspi_value + 4, "%d %d %d ",  saudio_rsp.sarray[0],saudio_rsp.sarray[1],saudio_rsp.sarray[2]);
    LCD_Print(1, str_clear_spi_value);
    LCD_Print(1, gspi_value);
    #endif
    return 0;
}

/**
  * @brief  Send cmd to spi like stop, reset,power up and power down
  * @param  Command index
  * @return 0
  * @note   
  */

uint8_t AudioIcSpiCmd( ETAUDIO_COMMD_NAME lcommd )
{
    saudio_cmd.sarray[0]=gspi_audio_cmd[(uint8_t)lcommd];
    
    SpiAudioICReadWrite(SPI1, saudio_rsp.sarray ,saudio_cmd.sarray , 1); 
    #ifdef SPI_Degug
    sprintf(gspi_value + 4, "%d ",  saudio_rsp.sarray[0]);
    LCD_Print(1, str_clear_spi_value);
    LCD_Print(1, gspi_value);
    #endif
    return 0;
}




/**
  * @brief  Read SPI Audio Driver register.
  * @param  spi is the base address of SPI module.
  * @return Status register value.
  * @note   Before calling this function, the transaction length (data width) must be configured as 8 bits.
  */
void SpiAudioICReadWrite(SPI_T *spi, uint8_t *Data_TO_Read, uint8_t *Data_TO_Write , uint8_t u32ByteCount)
{
	
		uint8_t u32RxCounter, u32TxCounter;

    /* /CS: active */
    SPI_SET_SS_LOW(spi);
	
		/* Check the BUSY status and TX empty flag */
    while((spi->CNTRL & (SPI_CNTRL_GO_BUSY_Msk | SPI_CNTRL_TX_EMPTY_Msk)) != SPI_CNTRL_TX_EMPTY_Msk);
    /* Clear RX FIFO */
    spi->FIFO_CTL |= SPI_FIFO_CTL_RX_CLR_Msk;

    u32RxCounter = 0;
    u32TxCounter = 0;

    while(u32RxCounter < u32ByteCount)
    {
        while(SPI_GET_RX_FIFO_EMPTY_FLAG(spi) == 0)
        {
            Data_TO_Read[u32RxCounter] = SPI_READ_RX0(spi);
            u32RxCounter++;
        }
        if((SPI_GET_TX_FIFO_FULL_FLAG(spi) != 1) && (u32TxCounter < u32ByteCount))
        {
            SPI_WRITE_TX0(spi, Data_TO_Write[u32TxCounter]);
            u32TxCounter++;
        }
    }

    /* /CS: inactive */
    SPI_SET_SS_HIGH(spi);
}



/**
  * @brief  Perform SPI Audio IC Write
  * @param  spi is the base address of SPI module.
  * @param  au8DataBuffer is the pointer of source data.
  * @return None.
  * @note   Before calling this function, the transaction length (data width) must be configured as 8 bits.
  */
void SpiAudio_Write(SPI_T *spi, uint8_t *au8DataBuffer, uint8_t u32ByteCount)
{
    uint8_t u32Counter=0;

    /* /CS: active */
    SPI_SET_SS_LOW(spi);
    u32Counter = 0;
    while(u32Counter < u32ByteCount)
    {
        if(SPI_GET_TX_FIFO_FULL_FLAG(spi) != 1)
            SPI_WRITE_TX0(spi, au8DataBuffer[u32Counter++]);
    }
    /* Check the BUSY status and TX empty flag */
    while((spi->CNTRL & (SPI_CNTRL_GO_BUSY_Msk | SPI_CNTRL_TX_EMPTY_Msk)) != SPI_CNTRL_TX_EMPTY_Msk);
    /* Clear RX FIFO */
    spi->FIFO_CTL |= SPI_FIFO_CTL_RX_CLR_Msk;
    /* /CS: inactive */
    SPI_SET_SS_HIGH(spi);
}



/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/
