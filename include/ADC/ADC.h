#ifndef ADC_H_
#define ADC_H_
#include "loggerConfig.h"

int ADC_init(LoggerConfig *loggerConfig);

void ADC_sample_all(void);

float ADC_read(unsigned int channel);

float ADC_get_voltage_range(size_t channel);

#endif /* ADC_H_ */
