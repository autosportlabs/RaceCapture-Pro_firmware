/*
 * base64.cpp
 *
 *  Created on: Apr 28, 2009
 *      Author: brent
 */
#include "base64.h"


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
void Base64::Encode(wxString &data, wxString &result) {

	result="";

	size_t nLength = data.Len();
  	for(size_t nPos = 0; nPos < nLength; nPos++) {
    char cCode;

    const char * dataRaw = data.ToAscii();

    // Encode the first 6 bits
    cCode = (dataRaw[nPos] >> 2) & 0x3f;

    result += sBase64Table[(int)cCode];

    // Encode the remaining 2 bits with the next 4 bits (if present)
    cCode = (dataRaw[nPos] << 4) & 0x3f;
    if(++nPos < nLength) cCode |= (dataRaw[nPos] >> 4) & 0x0f;

    result += sBase64Table[(int)cCode];

    if(nPos < nLength) {
      cCode = (dataRaw[nPos] << 2) & 0x3f;
      if(++nPos < nLength) cCode |= (dataRaw[nPos] >> 6) & 0x03;
      result += sBase64Table[(int)cCode];
    } else {
      ++nPos;
      result += cFillChar;
    }

    if(nPos < nLength) {
      cCode = dataRaw[nPos] & 0x3f;
      result += sBase64Table[(int)cCode];
    } else {
		result += cFillChar;
    }
  }
}

//Decodes a base64 encoded string
//Allocates and returns a buffer
//with the decoded string
//Returns NULL if no memory can be allocated
void Base64::Decode(wxString &data, wxString &result){ // const char * sString, size_t nLength) {

	result = "";
	size_t nLength = data.Len();
	for(size_t nPos = 0; nPos < nLength; nPos++) {
	    unsigned char c, c1;

	    c = (char) DecodeTable[(unsigned char)data[nPos]];
	    nPos++;
	    c1 = (char) DecodeTable[(unsigned char)data[nPos]];
	    c = (c << 2) | ((c1 >> 4) & 0x3);
	    result += c;

	    if(++nPos < nLength) {
			c = data[nPos];
			if(cFillChar == c) break;
			c = DecodeTable[(unsigned char)data[nPos]];
			c1 = ((c1 << 4) & 0xf0) | ((c >> 2) & 0xf);
			result += c1;
	    }

	    if(++nPos < nLength) {
			c1 = data[nPos];
			if(cFillChar == c1) break;
			c1 = DecodeTable[(unsigned char)data[nPos]];
			c = ((c << 6) & 0xc0) | c1;
			result += c;
	    }
	}
}


