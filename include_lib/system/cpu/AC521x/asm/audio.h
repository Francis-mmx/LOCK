#ifndef CPU_AUDIO_H
#define CPU_AUDIO_H

#include "typedef.h"
#include "asm/ladc.h"
#include "asm/dac.h"
#include "asm/iis.h"


struct audio_pf_data {
    const struct adc_platform_data *adc_pf_data;
    const struct dac_platform_data *dac_pf_data;
    const struct iis_platform_data *iis_pf_data;
};

#endif
