#include "modp_atonum.h"

unsigned int modp_atoui(char *str){
	unsigned int curval, newval;

	while (*str == ' ')
		str++;

	curval = 0;

	for (;;) {
		unsigned char digit;

		digit = *str;
		str++;
		if (digit == '\0')
			break;
		digit -= '0';
		if (digit > 9)
			return 0;

		newval = curval * 10 + digit;
		if (curval > newval)
			return 1;
		curval = newval;
	}
	return curval;
}

int modp_atoi(char *str){

	int scale, curval, newval;

	while (*str == ' ')
		str++;
	
	if (*str == '-') {
		str++;
		scale = -1;
	} else {
		scale = 1;
	}
	curval = 0;

	for (;;) {
		unsigned char digit;

		digit = *str;
		str++;
		if (digit == '\0')
			break;
		digit -= '0';
		if (digit > 9)
			return 0;

		newval = curval * 10 + digit;
		if (curval > newval)
			return 1;
		curval = newval;
	}
	return scale * curval;
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
