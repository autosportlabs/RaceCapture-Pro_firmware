#include "modp_atonum.h"

#define MAX_DOUBLE_DECIMAL_PLACES 10
#define MAX_FLOAT_DECIMAL_PLACES 10

unsigned int modp_atoui(const char *str){
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

int modp_atoi(const char *str){

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

double modp_atod(const char *str){
	
	int negative = 0;
	int decimalPlaces = 0;
	double doubleValue = 0.0;
	unsigned int wholeValue = 0;
	unsigned int fractionalValue = 0;
	int div = 1;
	while (*str && decimalPlaces < MAX_DOUBLE_DECIMAL_PLACES){
		if (*str == '-'){
			negative = 1;
		} else if (*str == '.'){
			decimalPlaces = 1;
		}else if (*str >= '0' && *str <= '9'){
			if (decimalPlaces == 0){
				wholeValue = wholeValue * 10;
				wholeValue = wholeValue + (*str - '0');
			} else{
				fractionalValue = fractionalValue * 10;
				fractionalValue = fractionalValue + (*str - '0');
				div = div * 10;
				decimalPlaces++;
			}
		}
		str++;
	}
	doubleValue = (((double)fractionalValue) / ((double)div));
	doubleValue += (double)wholeValue;
	if (negative) doubleValue = -doubleValue;
	return doubleValue;	
	
}

float modp_atof(const char *str){
	
	int negative = 0;
	int decimalPlaces = 0;
	float floatValue = 0.0;
	unsigned int wholeValue = 0;
	unsigned int fractionalValue = 0;
	int div = 1;
	while (*str && decimalPlaces < MAX_FLOAT_DECIMAL_PLACES){
		if (*str == '-'){
			negative = 1;
		} else if (*str == '.'){
			decimalPlaces = 1;
		}else if (*str >= '0' && *str <= '9'){
			if (decimalPlaces == 0){
				wholeValue = wholeValue * 10;
				wholeValue = wholeValue + (*str - '0');
			} else{
				fractionalValue = fractionalValue * 10;
				fractionalValue = fractionalValue + (*str - '0');
				div = div * 10;
				decimalPlaces++;
			}
		}
		str++;
	}
	floatValue = ((float)wholeValue) + (((float)fractionalValue) / ((float)div));
	if (negative) floatValue = -floatValue;
	return floatValue;	
}



unsigned char modp_xtoc(const char *str){
	unsigned char value = 0;
    unsigned char digit1 = str[0];
    unsigned char digit2 = str[1];
    digit1 = digit1 - (digit1 >= 48 && digit1 <= 57 ? 48 : (digit1 >= 65 && digit1 <= 70 ? 55 : digit1));
    digit2 = digit2 - (digit2 >= 48 && digit2 <= 57 ? 48 : (digit2 >= 65 && digit2 <= 70 ? 55 : digit2));
    value = (digit1 << 4) + digit2;
    return value;
}
