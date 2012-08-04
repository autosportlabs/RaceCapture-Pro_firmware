#ifndef _COMM_H_
#define _COMM_H_

#include "configData.h"
#include "comm_win32.h"
#include "raceCapture/raceCaptureConfig.h"
#include "raceCapture/raceCaptureRuntime.h"

#include "commonEvents.h"

class RaceAnalyzerCommCallback : public ProgresssReceiver{
public:
	virtual void ReadConfigComplete(bool success, wxString msg) = 0;
	virtual void WriteConfigComplete(bool success, wxString msg) = 0;
	virtual void FlashConfigComplete(bool success, wxString msg) = 0;
};

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
		void flashCurrentConfig();
		void readRuntime(RuntimeValues *values);
		void readConfig(RaceCaptureConfig *config, RaceAnalyzerCommCallback *callback);
		void writeConfig(RaceCaptureConfig *config, RaceAnalyzerCommCallback *callback);
		void updateWriteConfigPct(int count, RaceAnalyzerCommCallback *callback);

	private:

		static const int DEFAULT_TIMEOUT = 1000;
		wxLogStderr *_logger;
		FILE *_logfile;

		static const size_t SCRIPT_PAGE_LENGTH = 256;
		static const int SCRIPT_PAGES = 40;
		static const char EQUALS_HIDE_CHAR = 1;
		static const char SEMICOLON_HIDE_CHAR = 2;

		CComm* OpenSerialPort();
		CComm* GetSerialPort();
		void CheckThrowResult(wxString &result);
		int FlushReceiveBuffer(CComm * comPort);
		int ReadLine(CComm * comPort, wxString &buffer, int timeout);
		int WriteLine(CComm * comPort, wxString &buffer, int timeout);
		wxString SendCommand(CComm *comPort, const wxString &buffer, int timeout = DEFAULT_TIMEOUT);
		void Unescape(wxString &data);
		void Escape(wxString &data);
		int GetIntParam(wxString &data, const wxString &name);
		float GetFloatParam(wxString &data,const wxString &name);
		wxString GetParam(wxString &data, const wxString &name);
		void HideInnerTokens(wxString &data);
		void UnhideInnerTokens(wxString &data);
		void SwapCharsInsideQuotes(wxString &data, char find,char replace);

		wxString AppendStringParam(wxString &cmd, wxString param);
		wxString AppendFloatParam(wxString &cmd, float param);
		wxString AppendIntParam(wxString &cmd, int param);
		wxString AppendUIntParam(wxString &cmd, unsigned int param);
		wxString AppendChannelConfig(wxString &cmd, ChannelConfig &channelConfig);

		void CheckThrowIncompatibleVersion();

		int 			_serialPortNumber;
		CComm* 			_serialPort;

		wxMutex			_commMutex;
};

class AsyncRaceAnalyzerComm : public wxThread{

public:

	static const int ACTION_READ_CONFIG = 0;
	static const int ACTION_WRITE_CONFIG = 1;
	static const int ACTION_FLASH_CONFIG = 2;

	AsyncRaceAnalyzerComm(RaceAnalyzerComm *comm, RaceCaptureConfig *config, RaceAnalyzerCommCallback *progress);
	void RunReadConfig(void);
	void RunWriteConfig(void);
	void RunFlashConfig(void);
	void Run(int action);
	void * Entry();

private:
	void WriteConfig();

	int m_action;
	RaceAnalyzerComm *m_comm;
	RaceCaptureConfig *m_config;
	RaceAnalyzerCommCallback *m_callback;
	wxWindow *m_parent;
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
