#ifndef __AUDIO_APP_H__
#define __AUDIO_APP_H__

#include "M051Series.h"

/* Function used by driver layer only */
/* Playing voice state machine, Pass argument as 1 only if new voice command want to play, else pass 0 */
void VoicePlayOnGsm(uint8_t lsts_change);

/* Function used by Application layer */
/* Pass voice index to be played */
void PlayAudioOnGsm(uint8_t laudio_index);

/* Function used by Application layer and driver layer 
   This fution take lfeed_inpt argument and return current status of voice playing on IC 
   
   lfeed_inpt= 0; for application layer
   lfeed_inpt= 0XCC; voice processing start for driver layer 
   lfeed_inpt= 0XDD; voice processing end for driver layer 
   
   return 0 = voice processing
   return 1 = voice processing Done
*/
uint8_t PlayAudioFeedback(uint8_t lfeed_inpt);


#endif

