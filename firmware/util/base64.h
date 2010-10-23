#ifndef B64_BASE64_H
#define B64_BASE64_H
#include "FreeRTOS.h"

static const char sBase64Table[] = \
// 0000000000111111111122222222223333333333444444444455555555556666
// 0123456789012345678901234567890123456789012345678901234567890123
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char cFillChar = '=';
static const char DecodeTable[] = {
  // 0   1   2   3   4   5   6   7   8   9 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  //   0 -   9
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  //  10 -  19
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  //  20 -  29
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  //  30 -  39
    -1, -1, -1, 62, -1, -1, -1, 63, 52, 53,  //  40 -  49
    54, 55, 56, 57, 58, 59, 60, 61, -1, -1,  //  50 -  59
    -1, -1, -1, -1, -1,  0,  1,  2,  3,  4,  //  60 -  69
     5,  6,  7,  8,  9, 10, 11, 12, 13, 14,  //  70 -  79
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24,  //  80 -  89
    25, -1, -1, -1, -1, -1, -1, 26, 27, 28,  //  90 -  99
    29, 30, 31, 32, 33, 34, 35, 36, 37, 38,  // 100 - 109
    39, 40, 41, 42, 43, 44, 45, 46, 47, 48,  // 110 - 119
    49, 50, 51, -1, -1, -1, -1, -1, -1, -1,  // 120 - 129
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 130 - 139
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 140 - 149
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 150 - 159
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 160 - 169
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 170 - 179
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 180 - 189
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 190 - 199
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 200 - 209
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 210 - 219
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 220 - 229
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 230 - 239
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 240 - 249
    -1, -1, -1, -1, -1, -1                  // 250 - 256
  };


//Encodes a base64 encoded string
//Allocates and returns a buffer
//with the encoded string
//Returns NULL if no memory can be allocated
char * base64encode(const char *sString,size_t nLength) {
	
  // Allocate memory for the converted string
	size_t bufferSize = nLength * 8 /6 + 1;
	char *sResult = pvPortMalloc(bufferSize);
	if (NULL == sResult) return NULL; //if no memory available...

	char *currentResultChar = sResult;
	
  	for(size_t nPos = 0; nPos < nLength; nPos++) {
    char cCode;
  
    // Encode the first 6 bits
    cCode = (sString[nPos] >> 2) & 0x3f;
    *(currentResultChar++) = sBase64Table[(int)cCode];

    // Encode the remaining 2 bits with the next 4 bits (if present)
    cCode = (sString[nPos] << 4) & 0x3f;
    if(++nPos < nLength) cCode |= (sString[nPos] >> 4) & 0x0f;
    
    *(currentResultChar++) = sBase64Table[(int)cCode];

    if(nPos < nLength) {
      cCode = (sString[nPos] << 2) & 0x3f;
      if(++nPos < nLength) cCode |= (sString[nPos] >> 6) & 0x03;
      *(currentResultChar++) = sBase64Table[(int)cCode]; 
    } else {
      ++nPos;
      *(currentResultChar++) = cFillChar;
    }

    if(nPos < nLength) {
      cCode = sString[nPos] & 0x3f;
      *(currentResultChar++) = sBase64Table[(int)cCode]; 
    } else {
		*(currentResultChar++) = cFillChar;
    }
  }
  *currentResultChar='\0';
  return sResult;
}

//Decodes a base64 encoded string
//Allocates and returns a buffer
//with the decoded string
//Returns NULL if no memory can be allocated
char * base64decode(const char * sString, size_t nLength) {

	if (nLength == 0) return "";

	char * sResult = pvPortMalloc(nLength);
	if (NULL == sResult) return NULL; //if no memory available...
	
	char *currentResultChar = sResult;
	
	for(size_t nPos = 0; nPos < nLength; nPos++) {
	    unsigned char c, c1;
	
	    c = (char) DecodeTable[(unsigned char)sString[nPos]];
	    nPos++;
	    c1 = (char) DecodeTable[(unsigned char)sString[nPos]];
	    c = (c << 2) | ((c1 >> 4) & 0x3);
	    *(currentResultChar++) = c;
	
	    if(++nPos < nLength) {
			c = sString[nPos];
			if(cFillChar == c) break;
			c = DecodeTable[(unsigned char)sString[nPos]];
			c1 = ((c1 << 4) & 0xf0) | ((c >> 2) & 0xf);
			*(currentResultChar++) = c1;
	    }
	
	    if(++nPos < nLength) {
			c1 = sString[nPos];
			if(cFillChar == c1) break;
			c1 = DecodeTable[(unsigned char)sString[nPos]];
			c = ((c << 6) & 0xc0) | c1;
			*(currentResultChar++) = c;
	    }
	}
	*currentResultChar = '\0';
	return sResult;
}

#endif // B64_BASE64_H
