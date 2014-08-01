#ifndef ADC_H_
#define ADC_H_
#include "loggerConfig.h"

int ADC_init(LoggerConfig *loggerConfig);

void ADC_sample_all(void);

void ADC_read_all(unsigned int *a0,
						unsigned int *a1,
						unsigned int *a2,
						unsigned int *a3,
						unsigned int *a4,
						unsigned int *a5,
						unsigned int *a6,
						unsigned int *a7 );

unsigned int ADC_read(unsigned int channel);



#endif /* ADC_H_ */
