#ifndef _COMM_H_
#define _COMM_H_

#include "configData.h"
#include "wx/wx.h"
#include "wx/ctb/serport.h"




class RaceAnalyzerComm {

		
	public:
		RaceAnalyzerComm();
		~RaceAnalyzerComm();
		void SetSerialPort(int port);
		void CloseSerialPort();

	private:
	
		wxLogStderr *_logger;	
		FILE *_logfile;
		
		wxSerialPort* OpenSerialPort();
		wxSerialPort* GetSerialPort();
		int FlushReceiveBuffer(wxSerialPort * comPort);
		int ReadResponse(wxSerialPort * comPort, unsigned char * response, int length, int timeout);
		
		void CheckThrowIncompatibleVersion();
		
		int 					_serialPortNumber;
		wxSerialPort* 			_serialPort;

		char 					* GetSerialPortDevName(int comPort);
		wxMutex					_commMutex;
};

class CommException{
	
	public:
		static const int OPEN_PORT_FAILED = 0;
		static const int DATA_ERROR = 1;
		static const int COMM_TIMEOUT = 2;
		
		CommException(int errorStatus);
		int GetErrorStatus();
		wxString GetErrorMessage();
		
	private:
		int _errorStatus;
};

#endif 
