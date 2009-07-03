/*
 * exceptions.h
 *
 *  Created on: May 19, 2009
 *      Author: brent
 */

#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

#include "wx/wxprec.h"
#include "logging.h"

class RuntimeException{

	public:
		RuntimeException(wxString msg){
			m_msg = msg;
		}
		wxString GetMessage(){
			return m_msg;
		}
		void SetMessage(wxString msg){
			m_msg = msg;
		}

	private:
		wxString m_msg;
};

class FileAccessException : public RuntimeException{
public:
	FileAccessException(wxString msg, wxString filePath): RuntimeException(wxString::Format("%s : (%s)",msg.ToAscii(), filePath.ToAscii())){
		VERBOSE(FMT("FileAccessException thrown: %s",GetMessage().ToAscii()));
	}
};

#endif /* EXCEPTIONS_H_ */
