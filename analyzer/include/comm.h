#ifndef _COMM_H_
#define _COMM_H_

#include "configData.h"
#include "wx/ctb/serport.h"
#include "raceCapture/raceCaptureConfig.h"



class RaceAnalyzerComm {


	public:
		RaceAnalyzerComm();
		~RaceAnalyzerComm();
		void SetSerialPort(int port);
		void CloseSerialPort();
		wxString readScript();
		void writeScript(wxString &script);
		void reloadScript(void);
		void populateChannelConfig(ChannelConfig &cfg, wxString suffix, wxString &data);
		void populateChannelConfig(ChannelConfig &cfg, wxString &data);
		void readConfig(RaceCaptureConfig &config);
		void writeConfig(RaceCaptureConfig &config);

	private:

		static const int DEFAULT_TIMEOUT = 5000;
		wxLogStderr *_logger;
		FILE *_logfile;

		static const size_t SCRIPT_PAGE_LENGTH = 256;
		static const int SCRIPT_PAGES = 40;
		static const char EQUALS_HIDE_CHAR = 1;
		static const char SEMICOLON_HIDE_CHAR = 2;

		wxSerialPort* OpenSerialPort();
		wxSerialPort* GetSerialPort();
		void checkThrowResult(wxString &result);
		int FlushReceiveBuffer(wxSerialPort * comPort);
		int ReadLine(wxSerialPort * comPort, wxString &buffer, int timeout);
		int WriteLine(wxSerialPort * comPort, wxString &buffer, int timeout);
		wxString SendCommand(wxSerialPort *comPort, wxString &buffer, int timeout = DEFAULT_TIMEOUT);
		void unescape(wxString &data);
		void escape(wxString &data);
		int getIntParam(wxString &data, const wxString &name);
		float getFloatParam(wxString &data,const wxString &name);
		wxString getParam(wxString &data, const wxString &name);
		void hideInnerTokens(wxString &data);
		void unhideInnerTokens(wxString &data);
		void swapCharsInsideQuotes(wxString &data, char find,char replace);

		wxString AppendStringParam(wxString &cmd, wxString param);
		wxString AppendFloatParam(wxString &cmd, float param);
		wxString AppendIntParam(wxString &cmd, int param);
		wxString AppendUIntParam(wxString &cmd, unsigned int param);
		wxString AppendChannelConfig(wxString &cmd, ChannelConfig &channelConfig);

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
		static const int CMD_ERROR = 3;

		CommException(int errorStatus, wxString errorDetail = "");
		int GetErrorStatus();
		wxString GetErrorDetail();
		wxString GetErrorMessage();

	private:
		int _errorStatus;
		wxString _errorDetail;
};

#endif
