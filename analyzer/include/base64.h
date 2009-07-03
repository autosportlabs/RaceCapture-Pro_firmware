/*
 * base64.h
 *
 *  Created on: Apr 28, 2009
 *      Author: brent
 */

#ifndef BASE64_H_
#define BASE64_H_
#include "wx/wxprec.h"

class Base64{

public:
	static void Decode(wxString &data,wxString &buffer);
	static void Encode(wxString &data,wxString &buffer);
};

#endif /* BASE64_H_ */
