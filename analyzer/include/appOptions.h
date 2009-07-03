#ifndef APPOPTIONS_H_
#define APPOPTIONS_H_
#include "wx/wxprec.h"
#include "datalogData.h"

#include "raceAnalyzerConfigBase.h"



class AppOptions{

	public:

		AppOptions();
		int GetSerialPort();
		void SetSerialPort(int serialPort);

		bool & GetAutoLoadConfig();
		void SetAutoLoadConfig(bool autoConnect);

		void SaveAppOptions();
		void LoadAppOptions();

		DatalogChannels & GetStandardChannels();
		DatalogChannelTypes & GetStandardChannelTypes();

		void LoadDefaultStandardChannels(DatalogChannels &channels);
		void LoadDefaultStandardChannelTypes(DatalogChannelTypes &channelTypes);

	private:
		int 				m_serialPort;
		bool 				m_autoLoadConfig;
		DatalogChannels 	m_standardChannels;
		DatalogChannelTypes	m_standardChannelTypes;


};


#endif /*APPOPTIONS_H_*/
