#include "app_config.h"
#include "printf.h"
#include "device/audio_dev.h"


#ifdef ECHO_CANCELL_ENABLE

int BlueTooth_Calling_QueryBufSize(void);
int BlueTooth_Calling_init(void *buf, int var1, int var2, const int *eq_tab);
int BlueTooth_Calling_run(void *ptr, short *mic_buf, short *far_buf, short *out, int *cur_gain);
int BlueTooth_Calling_SetVariable(void *ptr, const char *VarName, int Var);
int BlueTooth_Calling_GetVariable(void *ptr, const char *VarName, int *var);
const char **BlueTooth_Calling_GetVariableList(int *nVar);
//DisableBit:
/*
0:NLMS
1:NLP
2:ANS
3:ComfortNoise
4:TwoPath
5:ARS
6:UseCohDE


//DisableBit:
0: AEC
1: Echo Suppress
2: Noise Reduce
3: Comfort Noise
4: Two Path (define for earphone,default enable)
5: ARS      (define for speaker,when mic and loudspeaker in the same space)
6: UseCohDE (define for speaker,another Echo Suppress coefficient)

*/



#if 0
static const int eq_tab[] = {
    32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768,
    32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768,
    32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768,
    32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768,
    32768, 32768, 32768, 32768, 32768, 32768,
};
#endif

static char echo_cancell_state;
static char echo_cancell_buf[18000];
static short ref_dac_buf[128];

static void echo_cancell_init(void)
{
    int suppress_coeff1 = 2, suppress_coeff2 = 0;//1000;
    int aec_ctl = 0;

    while (sizeof(echo_cancell_buf) < BlueTooth_Calling_QueryBufSize()) {
        printf("echo_cancell_buf not enough = %d,  \r\n", sizeof(echo_cancell_buf), BlueTooth_Calling_QueryBufSize());
    }

//BlueTooth_Calling_init( echo_cancell_buf, suppress_coeff1, suppress_coeff2, eq_tab );
    BlueTooth_Calling_init(echo_cancell_buf, suppress_coeff1, suppress_coeff2, NULL);
    BlueTooth_Calling_SetVariable(echo_cancell_buf, "NLPClearThr", suppress_coeff2);
    BlueTooth_Calling_SetVariable(echo_cancell_buf, "AnalogAGC_BaseGain", 0);
    BlueTooth_Calling_SetVariable(echo_cancell_buf, "DisableFlag", aec_ctl);
//BlueTooth_Calling_SetVariable(echo_cancell_buf,"NLPClearThr",value[i]);
//BlueTooth_Calling_SetVariable(echo_cancell_buf,"SpeechSuppress",value[i]);
}


void dac_irq_output(s16 *data, int len, int sample)
{
    if (sample != 8000) {
        printf("echo_cancell dac sample must be 8KHZ ,,, %d\r\n", sample)	;
        return;
    }

    if (len == 512) {
        u32 i;
        for (i = 0; i < len / 4; i++) {
            ref_dac_buf[i] = data[i * 2];
        }
    }
    /*else if(len == 256)*/
    /*{*/
    /*memcpy(ref_dac_buf,dac_buf,len);	*/
    /*}*/
    else {
        printf("echo_cancell dac len error ,,, %d\r\n", len)	;
    }
}

REGISTER_AUDIO_OUTPUT_DATA_HANDLER(dac_irq_output);

void adc_irq_input(s16 *data, int len, int sample, s16 **out_data)
{
    static short res_buf[128];

    if (sample != 8000) {
        printf("echo_cancell adc sample must be 8KHZ ,,, %d\r\n", sample)	;
        return;
    }

    if (len != 256) {
        printf("Calling_run buf_len must 256*,,,%d\r\n", len);
        return ;
    }

    if (echo_cancell_state == 0) {
        echo_cancell_init();
        echo_cancell_state = 1;
    }

    /*static char test_buf[32];*/
    /*if(!memcmp(test_buf,ref_dac_buf,sizeof(test_buf)))*/
    /*putchar('$');*/

    BlueTooth_Calling_run(echo_cancell_buf, data, ref_dac_buf, res_buf, NULL); //200+ us

    memset(ref_dac_buf, 0, sizeof(ref_dac_buf));

    *out_data = res_buf;
}

REGISTER_AUDIO_INPUT_DATA_HANDLER(adc_irq_input);

#endif




