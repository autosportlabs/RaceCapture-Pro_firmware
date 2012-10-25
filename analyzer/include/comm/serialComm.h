/*
 * serialComm.h
 *
 *  Created on: Oct 25, 2012
 *      Author: brent
 */

#ifndef SERIALCOMM_H_
#define SERIALCOMM_H_
#include "wx/wxprec.h"
#include "comm_win32.h"

class SerialException{

	public:
		SerialException(int errorStatus): _errorStatus(errorStatus), _errorDetail(""){}
		SerialException(int errorStatus, wxString errorDetail = "") : _errorStatus(errorStatus), _errorDetail(errorDetail){};
		int GetErrorStatus() {return _errorStatus;}
		wxString GetErrorDetail() {return _errorDetail;}
		wxString GetErrorMessage() {return wxString::Format("%s (%d)", _errorDetail.ToAscii(), _errorStatus);}
	private:
		int _errorStatus;
		wxString _errorDetail;
};



#endif /* SERIALCOMM_H_ */
