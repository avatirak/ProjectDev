#ifndef __Audio_Driver_H__
#define __Audio_Driver_H__

#include "M051Series.h"

/* Variable declaration */
typedef  struct 
{
	uint8_t sarray[4];
	uint8_t array_size;
}staudio_commands;

typedef  struct 
{
	uint8_t audio_commd;
	uint8_t audio_commd_size;
}staudio_cmd_size;

typedef enum
{
    STOP=0,
    POWER_UP,
    Power_DOWN,
    RESET  
}ETAUDIO_COMMD_NAME;

typedef enum
{
    PLAYING_STATUS=0,
    PLAYING_STARTED=0XFE,
    PLAYING_STOP=0XFD   
}ETPLAYING_STATE;

/* Function prototype declaration */

uint8_t AudioPlayVp( uint8_t High_index , uint8_t llower_index );
uint8_t AudioPlayVm( uint8_t High_index , uint8_t llower_index );
uint8_t ReadStatusAudioIc( void );
uint8_t ReadIntrptAudioIc( void );
uint8_t AudioIcSpiCmd( ETAUDIO_COMMD_NAME lcommd);


void SpiAudioICReadWrite(SPI_T *spi, uint8_t *Data_TO_Read, uint8_t *Data_TO_Write , uint8_t u32ByteCount);
void SpiAudio_Write(SPI_T *spi, uint8_t *au8DataBuffer, uint8_t u32ByteCount);

#endif //__SPI_FLASH_H__


