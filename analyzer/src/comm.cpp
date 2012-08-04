
#include "comm.h"
#include <stdio.h>
#include <wx/log.h>
#include "base64.h"
#include <wx/tokenzr.h>
#include <wx/hashmap.h>

CommException::CommException(int errorStatus, wxString errorDetail){
	_errorStatus = errorStatus;
	_errorDetail = errorDetail;
}

int CommException::GetErrorStatus(){
	return _errorStatus;
}

wxString CommException::GetErrorDetail(){
	return _errorDetail;
}

wxString CommException::GetErrorMessage(){

	switch (_errorStatus){
		case OPEN_PORT_FAILED:
			return "Could not open port";
		case DATA_ERROR:
			return "A data error occured";
		case COMM_TIMEOUT:
			return "A communications timeout occured: (" + _errorDetail + ")";
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
		if (_serialPort->isOpen()) _serialPort->closePort();
		delete _serialPort;
		_serialPort = NULL;
	}
}

CComm* RaceAnalyzerComm::GetSerialPort(){

	if ( _serialPort){
		if ( ! _serialPort->isOpen()){
			delete _serialPort;
			_serialPort = NULL;
		}
	}


	if (! _serialPort){
		_serialPort = OpenSerialPort();
	}

	return _serialPort;
}

CComm* RaceAnalyzerComm::OpenSerialPort(){

	wxLogMessage("Open Serial port %d" ,_serialPortNumber + 1);
	CComm* comPort = new CComm();
	if (!comPort->openPort(comPort->getPortName(_serialPortNumber + 1))){
			delete ( comPort );
			throw CommException(CommException::OPEN_PORT_FAILED);
		}
	return comPort;
}

void RaceAnalyzerComm::CheckThrowResult(wxString &result){
	if (GetParam(result,"result") != "ok") throw CommException(CommException::CMD_ERROR,result);
}

void RaceAnalyzerComm::Unescape(wxString &data){

	wxString working = data;
	data = "";
	const char *d = working.ToAscii();
	while (*d){
		if (*d =='\\'){
			switch(*(d + 1)){
				case '_':
					data+=" ";
					break;
				case 'n':
					data+="\n";
					break;
				case 'r':
					data+="\r";
					break;
				case '\\':
					data+="\\";
					break;
				case '"':
					data+="\"";
					break;
				case '\0': //this should *NOT* happen
					return;
			}
			d++;
		}
		else{
			data+=*d;
		}
		d++;
	}
}

void RaceAnalyzerComm::Escape(wxString &data){

	wxString working = data;
	data="";
	const char *d = working.ToAscii();
	while(*d){
		switch(*d){
		case ' ':
			data+="\\_";
			break;
		case '\n':
			data+="\\n";
			break;
		case '\r':
			data+="\\r";
			break;
		case '\"':
			data+="\\\"";
			break;
		default:
			data+=*d;
			break;
		}
		d++;
	}
}


void RaceAnalyzerComm::SetSerialPort(int port){

	wxMutexLocker lock(_commMutex);

	if (_serialPortNumber != port){

		if (_serialPort){
			if (_serialPort->isOpen()) _serialPort->closePort();
			delete _serialPort;
			_serialPort = NULL;
		}
		_serialPortNumber = port;
	}
}

int RaceAnalyzerComm::FlushReceiveBuffer(CComm* comPort){
	comPort->drainInput();
}

wxString RaceAnalyzerComm::SendCommand(CComm *comPort, const wxString &buffer, int timeout){

	wxLogMessage("Send Cmd (%d): %s",buffer.Len(), buffer.ToAscii());
	wxString response;
	size_t bufferSize = 8192;
	comPort->sendCommand(buffer.ToAscii(),wxStringBuffer(response,bufferSize),bufferSize,DEFAULT_TIMEOUT,true);
	wxLogMessage("Cmd Response: %s", response.ToAscii());
	return response;
}

int RaceAnalyzerComm::WriteLine(CComm * comPort, wxString &buffer, int timeout){

	//wxLogMessage("writeLine: %s", buffer.ToAscii());
	char *tempBuff = (char*)malloc(buffer.Len() + 10);
	strcpy(tempBuff,buffer.ToAscii());
	strcat(tempBuff,"\r");
	char *buffPtr = tempBuff;

	size_t len = strlen(buffPtr);
	bool result = comPort->writeBuffer(buffPtr,len);
	free(tempBuff);

	return 0;
}

int RaceAnalyzerComm::ReadLine(CComm * comPort, wxString &buffer, int timeout){
	int count = comPort->readLine(wxStringBuffer(buffer,1024), 1024, timeout);
	buffer.Trim(true).Trim(false);
	return count;
}

void RaceAnalyzerComm::SwapCharsInsideQuotes(wxString &data, char find,char replace){
	bool insideQuotes = false;
	int len = data.Len();
	for (int index = 0; index < len; index++){
		if (data[index] == '"') insideQuotes = !insideQuotes;
		else if (data[index] == find && insideQuotes) data[index] = replace;
	}
}

void RaceAnalyzerComm::HideInnerTokens(wxString &data){
	SwapCharsInsideQuotes(data,'=',EQUALS_HIDE_CHAR);
	SwapCharsInsideQuotes(data,';',SEMICOLON_HIDE_CHAR);
}

void RaceAnalyzerComm::UnhideInnerTokens(wxString &data){
	SwapCharsInsideQuotes(data,EQUALS_HIDE_CHAR,'=');
	SwapCharsInsideQuotes(data,SEMICOLON_HIDE_CHAR,';');
}

int RaceAnalyzerComm::GetIntParam(wxString &data, const wxString &name){
	return atoi(GetParam(data,name));
}

float RaceAnalyzerComm::GetFloatParam(wxString &data,const wxString &name){
	return atof(GetParam(data,name));
}
wxString RaceAnalyzerComm::GetParam(wxString &data, const wxString &name){

	HideInnerTokens(data);
	wxStringTokenizer tokenizer(data,";");

	while (tokenizer.HasMoreTokens()){
		wxString token = tokenizer.GetNextToken();
		wxStringTokenizer subTokenizer(token,"=");
		if (subTokenizer.CountTokens() >= 2){
			wxString paramName = subTokenizer.GetNextToken();
			if (paramName == name){
				wxString value = subTokenizer.GetNextToken();
				UnhideInnerTokens(value);
				if (value.StartsWith("\"") && value.EndsWith("\"")){
					value = value.Left(value.Len() - 1);
					value = value.Right(value.Len() - 1);
				}
				return value;
			}
		}
	}
	return "";
}

void RaceAnalyzerComm::reloadScript(void){

	CComm *serialPort = GetSerialPort();
	if (NULL==serialPort) throw CommException(CommException::OPEN_PORT_FAILED);

	wxString reloadCmd = "reloadScript";
	wxString result = SendCommand(serialPort, reloadCmd);
	CheckThrowResult(result);
}

wxString RaceAnalyzerComm::readScript(){

	wxString script = "";
	int scriptPage = 0;
	int to = 0;
	wxLogMessage("readScript");
	CComm *serialPort = GetSerialPort();
	if (NULL==serialPort) throw CommException(CommException::OPEN_PORT_FAILED);

	FlushReceiveBuffer(serialPort);
	while(!to){

		//wxString cmd = wxString::Format("println(getScriptPage(%d))",scriptPage++);
		wxString cmd = wxString::Format("readScriptPage %d",scriptPage++);
		wxString buffer = SendCommand(serialPort, cmd);
		wxLogMessage("read: %s",buffer.ToAscii());
		buffer.Trim(false);
		buffer.Trim(true);

		size_t scriptFragmentLen = buffer.Length();
		wxString scriptFrag = GetParam(buffer,"script");
		Unescape(scriptFrag);

		wxLogMessage("unescaped script'%s",scriptFrag.ToAscii());

		if (scriptFragmentLen > 0 ) script+=scriptFrag;
		//the last page is a 'partial page'
		if (scriptFragmentLen < SCRIPT_PAGE_LENGTH ) break;
	}
	CloseSerialPort();
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

	CComm *serialPort = GetSerialPort();
	if (NULL==serialPort) throw CommException(CommException::OPEN_PORT_FAILED);

	while(index < length && page < SCRIPT_PAGES && !to){
		wxString scriptFragment;
		if (index + SCRIPT_PAGE_LENGTH > length){
			scriptFragment = script.Mid(index);
		}else{
			scriptFragment = script.Mid(index, SCRIPT_PAGE_LENGTH);
		}
		wxLogMessage("script fragment: '%s'", scriptFragment.ToAscii());
		Escape(scriptFragment);
		wxLogMessage("esc script fragment: '%s'",scriptFragment.ToAscii());
		//wxString cmd = wxString::Format("updateScriptPage(%d,\"%s\")", page,data.ToAscii());
		wxString cmd = wxString::Format("writeScriptPage %d %s",page,scriptFragment.ToAscii());
		wxString result = SendCommand(serialPort, cmd);
		CheckThrowResult(result);
		page++;
		index += SCRIPT_PAGE_LENGTH;
	}
	//did we write fewer than the max number of script pages?
	//note we're subtracting script pages by one to account for integer divide truncation
	if ((length / SCRIPT_PAGE_LENGTH) < SCRIPT_PAGES - 1 ){
		//write a null to the next page
		wxString cmd = wxString::Format("writeScriptPage %d",page);
		SendCommand(serialPort, cmd);
	}

	CloseSerialPort();
	if (to){
		throw CommException(CommException::COMM_TIMEOUT);
	}
}

void RaceAnalyzerComm::populateChannelConfig(ChannelConfig &cfg, wxString suffix, wxString &data){
	const char *suf = suffix.ToAscii();
	cfg.label = GetParam(data,wxString::Format("label_%s",suf));
	cfg.units = GetParam(data,wxString::Format("units_%s",suf));
	cfg.sampleRate = (sample_rate_t)GetIntParam(data,wxString::Format("sampleRate_%s",suf));
}

void RaceAnalyzerComm::populateChannelConfig(ChannelConfig &cfg, wxString &data){
	cfg.label = GetParam(data,"label");
	cfg.units = GetParam(data,"units");
	cfg.sampleRate = (sample_rate_t)GetIntParam(data,"sampleRate");
}

void RaceAnalyzerComm::readRuntime(RuntimeValues *values){
	try{
		wxDateTime start = wxDateTime::UNow();
		CComm *serialPort = GetSerialPort();
		if (NULL==serialPort) throw CommException(CommException::OPEN_PORT_FAILED);

		wxString cmd = "sample";
		wxString rsp = SendCommand(serialPort, "sample");

		wxStringTokenizer tokenizer(rsp,";");
		while (tokenizer.HasMoreTokens()){
			wxString token = tokenizer.GetNextToken();
			wxStringTokenizer subTokenizer(token,"=");
			if (subTokenizer.CountTokens() >= 2){
				wxString name = subTokenizer.GetNextToken();
				wxString value = subTokenizer.GetNextToken();
				UnhideInnerTokens(value);
				if (value.StartsWith("\"") && value.EndsWith("\"")){
					value = value.Left(value.Len() - 1);
					value = value.Right(value.Len() - 1);
				}
				(*values)[name] = atof(value);
			}
		}

		wxTimeSpan dur = wxDateTime::UNow() - start;
		wxLogMessage("sample in %f",dur.GetMilliseconds().ToDouble());
	}
	catch(CommException e){

	}

}

void RaceAnalyzerComm::readConfig(RaceCaptureConfig *config,RaceAnalyzerCommCallback *callback){
	try{
		wxDateTime start = wxDateTime::UNow();
		int updateCount = 0;
		CComm *serialPort = GetSerialPort();
		if (NULL==serialPort) throw CommException(CommException::OPEN_PORT_FAILED);

		for (int i = 0; i < CONFIG_ANALOG_CHANNELS; i++){
			AnalogConfig &analogConfig = (config->analogConfigs[i]);
			wxString cmd = wxString::Format("getAnalogCfg %d",i);
			wxString rsp = SendCommand(serialPort,cmd);
			populateChannelConfig(analogConfig.channelConfig,rsp);
			analogConfig.loggingPrecision = GetIntParam(rsp,"loggingPrecision");
			analogConfig.scalingMode = (scaling_mode_t)GetIntParam(rsp,"scalingMode");
			analogConfig.linearScaling = GetFloatParam(rsp,"scaling");
			for (int ii=0; ii < CONFIG_ANALOG_SCALING_BINS;ii++){
				analogConfig.scalingMap.rawValues[ii]=GetIntParam(rsp,wxString::Format("mapRaw_%d",ii));
			}
			for (int ii=0; ii < CONFIG_ANALOG_SCALING_BINS;ii++){
				analogConfig.scalingMap.scaledValue[ii]=GetFloatParam(rsp,wxString::Format("mapScaled_%d",ii));
			}
			updateWriteConfigPct(++updateCount,callback);
		}

		for (int i = 0; i < CONFIG_TIMER_CHANNELS; i++){
			TimerConfig &timerConfig = config->timerConfigs[i];
			wxString cmd = wxString::Format("getTimerCfg %d",i);
			wxString rsp = SendCommand(serialPort,cmd);
			populateChannelConfig(timerConfig.channelConfig,rsp);
			timerConfig.loggingPrecision = GetIntParam(rsp,"loggingPrecision");
			timerConfig.slowTimerEnabled = GetIntParam(rsp,"slowTimer") == 1;
			timerConfig.mode = (timer_mode_t)GetIntParam(rsp,"mode");
			timerConfig.pulsePerRev = GetIntParam(rsp,"pulsePerRev");
			timerConfig.timerDivider = GetIntParam(rsp,"divider");
			timerConfig.scaling = GetIntParam(rsp,"scaling");
			updateWriteConfigPct(++updateCount,callback);
		}

		for (int i = 0; i < CONFIG_ACCEL_CHANNELS; i++){
			AccelConfig &accelConfig = (config->accelConfigs[i]);
			wxString cmd = wxString::Format("getAccelCfg %d",i);
			wxString rsp = SendCommand(serialPort,cmd);
			populateChannelConfig(accelConfig.channelConfig,rsp);
			accelConfig.mode = (accel_mode_t)GetIntParam(rsp,"mode");
			accelConfig.channel = (accel_channel_t)GetIntParam(rsp,"channel");
			accelConfig.zeroValue = GetIntParam(rsp,"zeroValue");
			updateWriteConfigPct(++updateCount,callback);
		}

		for (int i = 0; i < CONFIG_ANALOG_PULSE_CHANNELS; i++){
			PwmConfig &pwmConfig = (config->pwmConfigs[i]);
			wxString cmd = wxString::Format("getPwmCfg %d",i);
			wxString rsp = SendCommand(serialPort,cmd);
			populateChannelConfig(pwmConfig.channelConfig,rsp);
			pwmConfig.loggingPrecision = GetIntParam(rsp,"loggingPrecision");
			pwmConfig.outputMode = (pwm_output_mode_t)GetIntParam(rsp,"outputMode");
			pwmConfig.loggingMode = (pwm_logging_mode_t)GetIntParam(rsp,"loggingMode");
			pwmConfig.startupDutyCycle = GetIntParam(rsp,"startupDutyCycle");
			pwmConfig.startupPeriod = GetIntParam(rsp,"startupPeriod");
			pwmConfig.voltageScaling = GetFloatParam(rsp,"voltageScaling");
			updateWriteConfigPct(++updateCount,callback);
		}

		for (int i = 0; i < CONFIG_GPIO_CHANNELS; i++){
			GpioConfig &gpioConfig = (config->gpioConfigs[i]);
			wxString cmd = wxString::Format("getGpioCfg %d",i);
			wxString rsp = SendCommand(serialPort,cmd);
			populateChannelConfig(gpioConfig.channelConfig,rsp);
			gpioConfig.mode = (gpio_mode_t)GetIntParam(rsp,"mode");
			updateWriteConfigPct(++updateCount,callback);
		}

		{
			GpsConfig &gpsConfig = (config->gpsConfig);
			wxString cmd = "getGpsCfg";
			wxString rsp = SendCommand(serialPort,cmd);
			gpsConfig.gpsInstalled = GetIntParam(rsp,"installed") != 0;
			populateChannelConfig(gpsConfig.latitudeCfg,"lat",rsp);
			populateChannelConfig(gpsConfig.longitudeCfg,"long",rsp);
			populateChannelConfig(gpsConfig.velocityCfg,"vel",rsp);
			populateChannelConfig(gpsConfig.timeCfg,"time",rsp);
			populateChannelConfig(gpsConfig.qualityCfg,"qual",rsp);
			populateChannelConfig(gpsConfig.satellitesCfg,"sats",rsp);
			updateWriteConfigPct(++updateCount,callback);
		}
		{
			GpsConfig &gpsConfig = (config->gpsConfig);
			wxString cmd = "getStartFinishCfg";
			wxString rsp = SendCommand(serialPort,cmd);
			populateChannelConfig(gpsConfig.lapCountCfg,"lapCount",rsp);
			populateChannelConfig(gpsConfig.lapTimeCfg,"lapTime",rsp);
			gpsConfig.startFinishLatitude = GetFloatParam(rsp,"startFinishLat");
			gpsConfig.startFinishLongitude = GetFloatParam(rsp,"startFinishLong");
			gpsConfig.startFinishRadius = GetFloatParam(rsp,"startFinishRadius");
			updateWriteConfigPct(++updateCount,callback);
		}

		{
			LoggerOutputConfig &outputConfig = (config->loggerOutputConfig);
			wxString cmd = "getOutputCfg";
			wxString rsp = SendCommand(serialPort,cmd);
			outputConfig.loggingMode = (logging_mode_t)GetIntParam(rsp,"sdLoggingMode");
			outputConfig.telemetryMode = (telemetry_mode_t)GetIntParam(rsp,"telemetryMode");
			outputConfig.p2pDestinationAddrHigh = GetIntParam(rsp,"p2pDestAddrHigh");
			outputConfig.p2pDestinationAddrLow = GetIntParam(rsp,"p2pDestAddrLow");
			updateWriteConfigPct(++updateCount,callback);
		}

		wxTimeSpan dur = wxDateTime::UNow() - start;
		wxLogMessage("get config in %f",dur.GetMilliseconds().ToDouble());
		callback->ReadConfigComplete(true,"");
	}
	catch(CommException e){
		callback->ReadConfigComplete(false, e.GetErrorMessage());
	}
}

void RaceAnalyzerComm::updateWriteConfigPct(int count,RaceAnalyzerCommCallback *callback){
	int total = CONFIG_ANALOG_CHANNELS +
			CONFIG_TIMER_CHANNELS +
			CONFIG_ACCEL_CHANNELS +
			CONFIG_ANALOG_PULSE_CHANNELS +
			CONFIG_GPIO_CHANNELS + 3; // 3 extra = gps, start/finish, logger output config

	int pct =  (count * 100) / total;
	callback->OnProgress(pct);

}

void RaceAnalyzerComm::writeConfig(RaceCaptureConfig *config, RaceAnalyzerCommCallback * callback){
	try{
		wxDateTime start = wxDateTime::UNow();
		CComm *serialPort = GetSerialPort();
		if (NULL==serialPort) throw CommException(CommException::OPEN_PORT_FAILED);

		int updateCount = 0;
		for (int i = 0; i < CONFIG_ANALOG_CHANNELS;i++){
			AnalogConfig &cfg = config->analogConfigs[i];
			wxString cmd = "setAnalogCfg";
			cmd = AppendIntParam(cmd, i);
			cmd = AppendChannelConfig(cmd, cfg.channelConfig);
			cmd = AppendIntParam(cmd, cfg.loggingPrecision);
			cmd = AppendIntParam(cmd, cfg.scalingMode);
			cmd = AppendFloatParam(cmd, cfg.linearScaling);
			ScalingMap &map = (cfg.scalingMap);
			for (int m=0; m < CONFIG_ANALOG_SCALING_BINS;m++){
				cmd = AppendIntParam(cmd, map.rawValues[m]);
			}
			for (int m=0; m < CONFIG_ANALOG_SCALING_BINS;m++){
				cmd = AppendFloatParam(cmd, map.scaledValue[m]);
			}
			wxString result = SendCommand(serialPort, cmd);
			CheckThrowResult(result);
			updateWriteConfigPct(++updateCount,callback);
		}
		for (int i = 0; i < CONFIG_TIMER_CHANNELS; i++){
			TimerConfig &cfg = config->timerConfigs[i];
			wxString cmd = "setTimerCfg";
			cmd = AppendIntParam(cmd, i);
			AppendChannelConfig(cmd, cfg.channelConfig);
			cmd = AppendIntParam(cmd, cfg.loggingPrecision);
			cmd = AppendIntParam(cmd, cfg.slowTimerEnabled);
			cmd = AppendIntParam(cmd, cfg.mode);
			cmd = AppendIntParam(cmd, cfg.pulsePerRev);
			cmd = AppendIntParam(cmd, cfg.timerDivider);
			cmd = AppendIntParam(cmd, cfg.scaling);
			wxString result = SendCommand(serialPort, cmd);
			CheckThrowResult(result);
			updateWriteConfigPct(++updateCount,callback);
		}
		for (int i = 0; i < CONFIG_ACCEL_CHANNELS; i++){
			AccelConfig &cfg = config->accelConfigs[i];
			wxString cmd = "setAccelCfg";
			cmd = AppendIntParam(cmd, i);
			cmd = AppendChannelConfig(cmd, cfg.channelConfig);
			cmd = AppendIntParam(cmd, cfg.mode);
			cmd = AppendIntParam(cmd, cfg.channel);
			cmd = AppendIntParam(cmd, cfg.zeroValue);
			wxString result = SendCommand(serialPort, cmd);
			CheckThrowResult(result);
			updateWriteConfigPct(++updateCount,callback);
		}
		for (int i = 0; i < CONFIG_ANALOG_PULSE_CHANNELS; i++){
			PwmConfig &cfg = config->pwmConfigs[i];
			wxString cmd = "setPwmCfg";
			cmd = AppendIntParam(cmd, i);
			cmd = AppendChannelConfig(cmd, cfg.channelConfig);
			cmd = AppendIntParam(cmd, cfg.loggingPrecision);
			cmd = AppendIntParam(cmd, cfg.outputMode);
			cmd = AppendIntParam(cmd, cfg.loggingMode);
			cmd = AppendIntParam(cmd, cfg.startupDutyCycle);
			cmd = AppendIntParam(cmd, cfg.startupPeriod);
			cmd = AppendFloatParam(cmd, cfg.voltageScaling);
			wxString result = SendCommand(serialPort, cmd);
			CheckThrowResult(result);
			updateWriteConfigPct(++updateCount,callback);
		}
		for (int i = 0; i < CONFIG_GPIO_CHANNELS; i++){
			GpioConfig &cfg = config->gpioConfigs[i];
			wxString cmd = "setGpioCfg";
			cmd = AppendIntParam(cmd,i);
			AppendChannelConfig(cmd, cfg.channelConfig);
			AppendIntParam(cmd, cfg.mode);
			wxString result = SendCommand(serialPort, cmd);
			CheckThrowResult(result);
			updateWriteConfigPct(++updateCount,callback);
		}
		{
			GpsConfig &cfg = config->gpsConfig;
			{
				wxString cmd = "setGpsCfg";
				cmd = AppendIntParam(cmd, cfg.gpsInstalled);
				cmd = AppendChannelConfig(cmd, cfg.latitudeCfg);
				cmd = AppendChannelConfig(cmd, cfg.longitudeCfg);
				cmd = AppendChannelConfig(cmd, cfg.velocityCfg);
				cmd = AppendChannelConfig(cmd, cfg.timeCfg);
				cmd = AppendChannelConfig(cmd, cfg.qualityCfg);
				cmd = AppendChannelConfig(cmd, cfg.satellitesCfg);
				wxString result = SendCommand(serialPort, cmd);
				CheckThrowResult(result);
				updateWriteConfigPct(++updateCount,callback);
			}
			{
				wxString cmd = "setStartFinishCfg";
				cmd = AppendChannelConfig(cmd, cfg.lapCountCfg);
				cmd = AppendChannelConfig(cmd, cfg.lapTimeCfg);
				cmd = AppendFloatParam(cmd, cfg.startFinishLatitude);
				cmd = AppendFloatParam(cmd, cfg.startFinishLongitude);
				cmd = AppendFloatParam(cmd, cfg.startFinishRadius);
				wxString result = SendCommand(serialPort, cmd);
				CheckThrowResult(result);
				updateWriteConfigPct(++updateCount,callback);
			}
		}
		{
			LoggerOutputConfig &cfg = config->loggerOutputConfig;
			wxString cmd = "setOutputCfg";
			cmd = AppendIntParam(cmd, cfg.loggingMode);
			cmd = AppendIntParam(cmd, cfg.telemetryMode);
			cmd = AppendUIntParam(cmd, cfg.p2pDestinationAddrHigh);
			cmd = AppendUIntParam(cmd, cfg.p2pDestinationAddrLow);
			wxString result = SendCommand(serialPort, cmd);
			CheckThrowResult(result);
			updateWriteConfigPct(++updateCount,callback);
		}
		wxTimeSpan dur = wxDateTime::UNow() - start;
		wxLogMessage("write config in %f",dur.GetMilliseconds().ToDouble());
		callback->WriteConfigComplete(true,"");
	}
	catch(CommException e){
		callback->WriteConfigComplete(false, e.GetErrorMessage());
	}
}

void RaceAnalyzerComm::flashCurrentConfig(){
	wxDateTime start = wxDateTime::UNow();
	CComm *serialPort = GetSerialPort();
	if (NULL==serialPort) throw CommException(CommException::OPEN_PORT_FAILED);
	wxString cmd = "flashLoggerCfg";

	wxString result = SendCommand(serialPort, cmd);
	CheckThrowResult(result);
	wxTimeSpan dur = wxDateTime::UNow() - start;
	wxLogMessage("flash config in %f",dur.GetMilliseconds().ToDouble());
}

wxString RaceAnalyzerComm::AppendStringParam(wxString &cmd, wxString param){
	param.Replace(" ","_",true);
	return cmd + wxString::Format(" \"%s\"", param.ToAscii());
}

wxString RaceAnalyzerComm::AppendFloatParam(wxString &cmd, float param){
	return cmd + wxString::Format(" %f", param);
}

wxString RaceAnalyzerComm::AppendIntParam(wxString &cmd, int param){
	return cmd + wxString::Format(" %d", param);
}

wxString RaceAnalyzerComm::AppendUIntParam(wxString &cmd, unsigned int param){
	return cmd + wxString::Format(" %u", param);
}

wxString RaceAnalyzerComm::AppendChannelConfig(wxString &cmd, ChannelConfig &channelConfig){
	cmd = AppendStringParam(cmd, channelConfig.label);
	cmd = AppendStringParam(cmd, channelConfig.units);
	cmd = AppendIntParam(cmd, channelConfig.sampleRate);
	return cmd;
}

AsyncRaceAnalyzerComm::AsyncRaceAnalyzerComm(RaceAnalyzerComm *comm, RaceCaptureConfig *config, RaceAnalyzerCommCallback *callback) :
		m_comm(comm), m_config(config), m_callback(callback){
	wxThread::Create();
}

void AsyncRaceAnalyzerComm::Run(int action){
	m_action = action;
	wxThread::Run();
}

void AsyncRaceAnalyzerComm::RunReadConfig(void){
	Run(ACTION_READ_CONFIG);
}

void AsyncRaceAnalyzerComm::RunWriteConfig(void){
	Run(ACTION_WRITE_CONFIG);
}

void AsyncRaceAnalyzerComm::RunFlashConfig(void){
	Run(ACTION_FLASH_CONFIG);
}

void * AsyncRaceAnalyzerComm::Entry(){
	switch(m_action){
	case ACTION_READ_CONFIG:
		m_comm->readConfig(m_config,m_callback);
		break;
	case ACTION_WRITE_CONFIG:
		m_comm->writeConfig(m_config,m_callback);
		break;
	case ACTION_FLASH_CONFIG:
		m_comm->flashCurrentConfig();
		break;
	default:
		break;
	}
	return NULL;
}
