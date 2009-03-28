
#include "comm.h"
#include "wx/ctb/expect.h"
#include "wx/ctb/getopt.h"
#include "wx/ctb/serport.h"
#include "wx/ctb/timer.h"
#include <stdio.h>
#include <wx/log.h>

CommException::CommException(int errorStatus){
	_errorStatus = errorStatus;
}

int CommException::GetErrorStatus(){
	return _errorStatus;
}

wxString CommException::GetErrorMessage(){

	switch (_errorStatus){
		case OPEN_PORT_FAILED:
			return "Could not open port";
		case DATA_ERROR:
			return "A data error occured";
		case COMM_TIMEOUT:
			return "A communications timeout occured";
		default:
			return "An unspecified error occured";
	}
}

RaceAnalyzerComm::RaceAnalyzerComm() : _serialPortNumber(0), _serialPort(NULL) {

	//_logfile = fopen("mjljConfigLog.txt", "w");
	_logfile = 0;
	_logger = new wxLogStderr(_logfile);
	wxLog::SetActiveTarget(_logger);
}

RaceAnalyzerComm::~RaceAnalyzerComm(){
	CloseSerialPort();
}

void RaceAnalyzerComm::CloseSerialPort(){

	wxMutexLocker lock(_commMutex);

	wxLogMessage("closing serial port");
	if (_serialPort){
		if (_serialPort->IsOpen()) _serialPort->Close();
		delete _serialPort;
		_serialPort = NULL;
	}
}

wxSerialPort* RaceAnalyzerComm::GetSerialPort(){

	//CloseSerialPort();

	if ( _serialPort){
		if ( ! _serialPort->IsOpen()){
			delete _serialPort;
			_serialPort = NULL;
		}
	}


	if (! _serialPort){
		_serialPort = OpenSerialPort();
	}

	return _serialPort;
}

wxSerialPort* RaceAnalyzerComm::OpenSerialPort(){

	wxLogMessage("Open COM%d" ,_serialPortNumber + 1);
	wxSerialPort* comPort = new wxSerialPort();
	char *devName = GetSerialPortDevName(_serialPortNumber);

	wxSerialPort_DCS dcs;
	dcs.baud=wxBAUD_38400;
	dcs.stopbits=1;
	dcs.parity = wxPARITY_NONE;
	dcs.wordlen = 8;
	dcs.xonxoff = false;
	dcs.rtscts = false;

		if (comPort->Open(devName,&dcs)<0){
			delete ( comPort );
			throw CommException(CommException::OPEN_PORT_FAILED);
		}
	return comPort;
}

void RaceAnalyzerComm::SetSerialPort(int port){

	wxMutexLocker lock(_commMutex);

	if (_serialPortNumber != port){

		if (_serialPort){
			if (_serialPort->IsOpen()) _serialPort->Close();
			delete _serialPort;
			_serialPort = NULL;
		}
		_serialPortNumber = port;
	}
}

char * RaceAnalyzerComm::GetSerialPortDevName(int comPort){

	switch (comPort){
		case 0:
			return wxCOM1;
		case 1:
			return wxCOM2;
		case 2:
			return wxCOM3;
		case 3:
			return wxCOM4;
		case 4:
			return wxCOM5;
		case 5:
			return wxCOM6;
		case 6:
			return wxCOM7;
		case 7:
			return wxCOM8;
		case 8:
			return wxCOM9;
		case 9:
			return wxCOM10;
		case 10:
			return wxCOM11;
		case 11:
			return wxCOM12;
		case 12:
			return wxCOM13;
		case 13:
			return wxCOM14;
		case 14:
			return wxCOM15;
		case 15:
			return wxCOM16;
		case 16:
			return "COM17";
		case 17:
			return "COM18";
		case 18:
			return "COM19";
		case 19:
			return "COM20";
		default:
			throw CommException(CommException::OPEN_PORT_FAILED);
	}
}

int RaceAnalyzerComm::FlushReceiveBuffer(wxSerialPort* comPort){

	char data;
	int buffer = 0;
	if (comPort && comPort->IsOpen()){
		while (1){
			if (comPort->Read(&data,1)){
				//#ifdef __WXDEBUG__
				wxLogMessage("flushed %0x", (int)data);
				//#endif
				//wxBell();
				buffer++;
			}
			else{
				break;
			}
		}
	//	#ifdef __WXDEBUG__
		if (buffer > 0) wxLogMessage("flushed %d bytes",buffer);
		//#endif
	}
	return buffer;
}

int RaceAnalyzerComm::ReadResponse(wxSerialPort * comPort, unsigned char * response, int length, int timeout){

	int to = 0;
	timer t(timeout, &to, NULL);

	t.start();

	char value;


/*	for (int i = 0; i < length && !to;){
		if (comPort->Readv(&value,1,&to,true)){
			response[i] = value;
			i++;
//			Sleep(1);
		}
	}
*/

	int result = comPort->Readv((char *)response,length,&to,true);

	if (result < 0){
		wxLogMessage("error in readv");
		CloseSerialPort();
	}
	t.stop();

	wxString bytes;

//	#ifdef __WXDEBUG__
		if (to){
			wxLogMessage("timed out");
			CloseSerialPort();
		}

	//	for (int i = 0; i<length; i++){
		//	bytes += wxString::Format("%0x,",response[i]);
	//	}

		//wxLogMessage("response: %s",bytes.ToAscii());
//	#endif

	return to;
}


