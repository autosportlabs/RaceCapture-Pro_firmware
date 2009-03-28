#ifndef APPOPTIONS_H_
#define APPOPTIONS_H_
#include "wx/wx.h"

#include "raceAnalyzerConfigBase.h"

#define CONFIG_KEY_COM_PORT 					"COM_Port"
#define CONFIG_KEY_AUTO_LOAD					"AutoLoadConfig"

class AppOptions{
	
	public:
		
		AppOptions(); 	
		int GetSerialPort();
		void SetSerialPort(int serialPort);
		
		bool & GetAutoLoadConfig();
		void SetAutoLoadConfig(bool autoConnect);
		
		void SaveAppOptions();
		void LoadAppOptions();
		
	private:
		int 		_serialPort;
		bool 		_autoLoadConfig;
};
#endif /*APPOPTIONS_H_*/
