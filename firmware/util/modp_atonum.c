#include "modp_atonum.h"


int modp_atoi(char *str){
	
	int negative = 0;
	int intValue =0;
	while (*str){
		if (*str == '-'){
			negative = 1;		
		}else if (*str >= 48 && *str <= 57){
			intValue = intValue * 10;
			intValue = intValue + (*str - 48);
		}
		str++;
	}
	if (negative) intValue = -intValue;
	return intValue;		
}

double modp_atod(char *str){
	
	int negative = 0;
	int fractionMode = 0;
	double doubleValue = 0.0;
	unsigned int wholeValue = 0;
	unsigned int fractionalValue = 0;
	int div = 1;
	while (*str){
		if (*str == '-'){
			negative = 1;
		} else if (*str == '.'){
			fractionMode = 1;		
		}else if (*str >= '0' && *str <= '9'){
			if (! fractionMode){
				wholeValue = wholeValue * 10;
				wholeValue = wholeValue + (*str - '0');
			} else{
				fractionalValue = fractionalValue * 10;
				fractionalValue = fractionalValue + (*str - '0');
				div = div * 10;
			}
		}
		str++;
	}
	doubleValue = (((double)fractionalValue) / ((double)div));
	doubleValue += (double)wholeValue;
	if (negative) doubleValue = -doubleValue;
	return doubleValue;	
	
}

float modp_atof(char *str){
	
	int negative = 0;
	int fractionMode = 0;
	float floatValue = 0.0;
	unsigned int wholeValue = 0;
	unsigned int fractionalValue = 0;
	int div = 1;
	while (*str){
		if (*str == '-'){
			negative = 1;
		} else if (*str == '.'){
			fractionMode = 1;		
		}else if (*str >= '0' && *str <= '9'){
			if (! fractionMode){
				wholeValue = wholeValue * 10;
				wholeValue = wholeValue + (*str - '0');
			} else{
				fractionalValue = fractionalValue * 10;
				fractionalValue = fractionalValue + (*str - '0');
				div = div * 10;
			}
		}
		str++;
	}
	floatValue = ((float)wholeValue) + (((float)fractionalValue) / ((float)div));
	if (negative) floatValue = -floatValue;
	return floatValue;	
}
