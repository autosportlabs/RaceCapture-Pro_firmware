
#include "comm.h"
#include "wx/ctb-0.13/getopt.h"
#include "wx/ctb-0.13/serport.h"
#include "wx/ctb-0.13/timer.h"
#include <stdio.h>
#include <wx/log.h>
#include "base64.h"


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
	const char *devName = GetSerialPortDevName(_serialPortNumber);

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

wxString RaceAnalyzerComm::readScript(){

	wxString script = "";
	int scriptPage = 0;
	int to = 0;
	wxLogMessage("readScript");
	wxSerialPort *serialPort = GetSerialPort();
	while(!to){
		wxString cmd = wxString::Format("println(getScriptPage(%d))",scriptPage++);
		to = WriteCommand(serialPort, cmd, DEFAULT_TIMEOUT);
		wxString buffer;
		to = ReadLine(serialPort, buffer, DEFAULT_TIMEOUT);
		wxString scriptFragment;
		Base64::Decode(buffer,scriptFragment);
		size_t scriptFragmentLen = scriptFragment.Length();
		if (scriptFragmentLen > 0 ) script+=scriptFragment;
		//the last page is a 'partial page'
		if (scriptFragmentLen < SCRIPT_PAGE_LENGTH ) break;
	}
	if (to){
		throw CommException(CommException::COMM_TIMEOUT);
	}
	return script;
}

void RaceAnalyzerComm::writeScript(wxString &script){

	size_t index = 0;
	int page,to;
	page = 0;
	to = 0;
	size_t length = script.Length();

	wxSerialPort *serialPort = GetSerialPort();

	while(index < length && page < SCRIPT_PAGES && !to){
		wxString scriptFragment;
		if (index + SCRIPT_PAGE_LENGTH > length){
			scriptFragment = script.Mid(index);
		}else{
			scriptFragment = script.Mid(index, SCRIPT_PAGE_LENGTH);
		}
		wxString data;
		Base64::Encode(scriptFragment,data);
		wxString cmd = wxString::Format("updateScriptPage(%d,\"%s\")", page,data.ToAscii());
		to = WriteCommand(serialPort, cmd, DEFAULT_TIMEOUT);
		page++;
		index += SCRIPT_PAGE_LENGTH;
	}
	//did we write fewer than the max number of script pages?
	//note we're subtracting script pages by one to account for integer divide truncation
	if ((length / SCRIPT_PAGE_LENGTH) < SCRIPT_PAGES - 1 ){
		//write a null to the next page
		wxString cmd = wxString::Format("updateScriptPage(%d,\"\")", page);
		to = WriteCommand(serialPort, cmd, DEFAULT_TIMEOUT);
	}
	if (to){
		throw CommException(CommException::COMM_TIMEOUT);
	}
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

const char * RaceAnalyzerComm::GetSerialPortDevName(int comPort){

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
			return wxCOM17;
		case 17:
			return wxCOM18;
		case 18:
			return wxCOM19;
		case 19:
			return wxCOM20;
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
	if (buffer > 0) wxLogMessage("flushed %d bytes",buffer);
	}
	return buffer;
}

int RaceAnalyzerComm::WriteCommand(wxSerialPort *comPort, wxString &buffer, int timeout){
	FlushReceiveBuffer(comPort);
	WriteLine(comPort,buffer,timeout);
	wxString echo;
	int to = ReadLine(comPort,echo,timeout);
	if (echo != buffer){
		wxLogMessage("echoed buffer: %s", buffer.ToAscii());
		//we expected the command echoed back
		//throw CommException(CommException::DATA_ERROR);
	}
	return to;
}

int RaceAnalyzerComm::WriteLine(wxSerialPort * comPort, wxString &buffer, int timeout){

	wxLogMessage("writeLine: %s", buffer.ToAscii());
	char *tempBuff = (char*)malloc(buffer.Len() + 2);
	strcpy(tempBuff,buffer.ToAscii());
	strcat(tempBuff,"\r\n");
	char *buffPtr = tempBuff;

	int to = 0;
	timer t(timeout, &to, NULL);
	t.start();

	size_t len = strlen(buffPtr);
	while (len > 0 && !to){
		size_t written = comPort->Writev(buffPtr,len,&to,true);
		buffPtr+=written;
		len -= written;
	}
	t.stop();
	free(tempBuff);

	return to;
}

int RaceAnalyzerComm::ReadLine(wxSerialPort * comPort, wxString &buffer, int timeout){

	int to = 0;
	timer t(timeout, &to, NULL);

	t.start();

	wxString dataLine = "";
	char value;

	while(!to){
		if (comPort->Readv(&value,1,&to,true)){
			if ('\n' == value) break;
			buffer += value;
		}
	}
	t.stop();

	if (to){
		wxLogMessage("timed out");
		CloseSerialPort();
	}

//	wxLogMessage("readLine: ");
//	wxLogMessage(buffer.T);
	return to;
}



