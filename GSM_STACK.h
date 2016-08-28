#ifndef GSM_STACK_H_
#define GSM_STACK_H_

#define RX_BUFF_SIZE_MAX                60
#define TOTAL_RX_BUFF                   5


typedef enum
{
    UNKNOWN=0,
    ACTIVE,
    CALL_END	
}eCURR_CALL_STS;

typedef enum
{
	IDEAL=0,
	IN_COMING,
	OUT_GOING	
} eGSM_CALL_DIRC;

typedef struct
{
	char u8RecData[RX_BUFF_SIZE_MAX];
	unsigned char buff_flag:1;
}stgsm_rcv_data;

typedef struct
{
	stgsm_rcv_data struc_gsm_data[TOTAL_RX_BUFF];                              // Buffer bank to  receive rx data fron GSM
	unsigned char buffer_to_read;                                   // Number of buffer to read left
	volatile unsigned char buffer_cnt_intrpt;                          // Buffer Index inside Interrupt
	unsigned char buffer_cnt_nointrpt;                                   // Buffer Index inside local data read
	volatile unsigned char comRbytes;								// Buffer Array Index
}stgsm_rx_data;


typedef struct
{
	const char * cmd_at;
	const char * resp_at;
}stmulti_commd;

typedef struct
{
	char mobile_numb[14];                                // Mobile Number
	unsigned char dtmf_value;                           // DTMF Value
	eCURR_CALL_STS currt_call_sts:2;                  // Current call stauts incoming or out 
	eGSM_CALL_DIRC call_dir:2;
}stgsm_call_data_struct;

typedef struct 
{
    unsigned char live_cmd_send:1;
    unsigned char live_count: 7;
}stgsm_live_struct;

typedef struct
{
	unsigned char array[4];
	unsigned char push_index;			
}dtmf_st;

void GsmProcessingStack(void);
unsigned char GsmStateRunning(void);
unsigned char GsmInitializaion(void);
unsigned char ReadAllSms(void);
char ReadNewSms(unsigned char Num_SMS);
unsigned char GsmCallRxStateFunc(void);
unsigned char GsmAliveStatusCheckFunc(void);
unsigned char CallStateCommonCommd(void);
unsigned char WaitStateLogic(unsigned char CURR_STATE , unsigned char SUCCIDE );
unsigned char GsmCallTxStateFunc (void );
unsigned char GsmSmsRxStateFunc( unsigned char SMS_Number_Read);
extern unsigned char GsmApplCallFunc( stgsm_call_data_struct );
void DtmfValuePush( unsigned char DtmfValue );
unsigned char DtmfValuePop( void );
unsigned char TerminateGsmCall(void);

#endif

