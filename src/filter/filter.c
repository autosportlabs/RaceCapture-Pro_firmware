#include "filter.h"

//Implements a fast Exponential Moving Average IIR filter

//This macros defines an alpha value between 0 and 1
#define DSP_EMA_I32_ALPHA(x) ( (unsigned short)(x * 65535) )

static int dsp_ema_i32(int in, int average, unsigned short alpha)
{
    long long tmp0; //calcs must be done in 64-bit math to avoid overflow
    tmp0 = (long long)in * (alpha) + (long long)average * (65536 - alpha);
    return (int)((tmp0 + 32768) / 65536); //scale back to 32-bit (with rounding)
}

void init_filter(Filter *filter, float alpha)
{
    filter->alpha = alpha;
    filter->current_value = 0;
}

int update_filter(Filter *filter, int value)
{
    int current_value = filter->current_value;
    current_value = dsp_ema_i32(value, current_value, DSP_EMA_I32_ALPHA(filter->alpha));
    filter->current_value = current_value;
    return current_value;
}
