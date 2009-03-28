
#include "appOptions.h"
#include "wx/config.h"


#define DEFAULT_COM_PORT 1
#define DEFAULT_AUTO_LOAD_CONFIG true

AppOptions::AppOptions()
	: 	_serialPort(DEFAULT_COM_PORT) , 
		_autoLoadConfig(DEFAULT_AUTO_LOAD_CONFIG)
{}
	
int AppOptions::GetSerialPort(){
	return _serialPort;
}

void AppOptions::SetSerialPort(int comPort){
	_serialPort = comPort;	
}

bool & AppOptions::GetAutoLoadConfig(){
	return _autoLoadConfig;	
}

void AppOptions::SetAutoLoadConfig(bool autoConnect){
	_autoLoadConfig = autoConnect;
}

void AppOptions::SaveAppOptions(){
	wxConfig config(MJLJ_CONFIG_APP_NAME);
	
	config.Write(CONFIG_KEY_COM_PORT,_serialPort);	
	config.Write(CONFIG_KEY_AUTO_LOAD, _autoLoadConfig);
}

void AppOptions::LoadAppOptions(){
	wxConfig config(MJLJ_CONFIG_APP_NAME);
	
	config.Read(CONFIG_KEY_COM_PORT,&_serialPort, DEFAULT_COM_PORT);
	config.Read(CONFIG_KEY_AUTO_LOAD, &_autoLoadConfig, DEFAULT_AUTO_LOAD_CONFIG);
}
