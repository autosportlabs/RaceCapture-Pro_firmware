#ifndef _COMM_H_
#define _COMM_H_

#include "configData.h"
#include "wx/ctb-0.13/serport.h"




class RaceAnalyzerComm {


	public:
		RaceAnalyzerComm();
		~RaceAnalyzerComm();
		void SetSerialPort(int port);
		void CloseSerialPort();
		wxString readScript();
		void writeScript(wxString &script);

	private:

		static const int DEFAULT_TIMEOUT = 1000;
		wxLogStderr *_logger;
		FILE *_logfile;

		static const size_t SCRIPT_PAGE_LENGTH = 256;
		static const int SCRIPT_PAGES = 40;

		wxSerialPort* OpenSerialPort();
		wxSerialPort* GetSerialPort();
		int FlushReceiveBuffer(wxSerialPort * comPort);
		int ReadLine(wxSerialPort * comPort, wxString &buffer, int timeout);
		int WriteLine(wxSerialPort * comPort, wxString &buffer, int timeout);
		int WriteCommand(wxSerialPort *comPort, wxString &buffer, int timeout);


		void CheckThrowIncompatibleVersion();

		int 					_serialPortNumber;
		wxSerialPort* 			_serialPort;

		const char 				* GetSerialPortDevName(int comPort);
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
