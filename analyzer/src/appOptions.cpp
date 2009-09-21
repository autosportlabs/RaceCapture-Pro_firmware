
#include "appOptions.h"
#include "wx/config.h"
#include "wx/tokenzr.h"

#define DEFAULT_COM_PORT 						1
#define DEFAULT_AUTO_LOAD_CONFIG 				true
#define DEFAULT_DATALOG_CHANNEL_COUNT 			-1
#define DEFAULT_DATALOG_CHANNEL_TYPES_COUNT 	-1
#define DEFAULT_ANALOG_GAUGE_TYPE_COUNT			-1
#define DEFAULT_DIGITAL_GAUGE_TYPE_COUNT		-1


#define DEFAULT_CHART_COLORS					"0x00FF00,0xFF0000,0x0000FF,0xFFFF00,0x00FFFF,0xFFFFFF,0xFF00FF"
#define CHART_COLOR_STRING_DELIMITER			","

#define CONFIG_KEY_COM_PORT 					"COM_Port"
#define CONFIG_KEY_AUTO_LOAD					"AutoLoadConfig"
#define CONFIG_KEY_DATALOG_CHANNEL				"DatalogChannel_"
#define CONFIG_KEY_DATALOG_CHANNEL_TYPE			"DatalogChannelType_"
#define CONFIG_KEY_DATALOG_CHANNEL_COUNT		"DatalogChannelCount"
#define CONFIG_KEY_DATALOG_CHANNEL_TYPE_COUNT	"DatalogChannelTypeCount"
#define CONFIG_KEY_CHART_COLORS					"ChartColors"

#define CONFIG_KEY_ANALOG_GAUGE_TYPE_COUNT		"AnalogGaugeTypeCount"
#define	CONFIG_KEY_ANALOG_GAUGE_TYPE			"AnalogGaugeType_"
#define CONFIG_KEY_ANALOG_GAUGE_MINOR_TICK		"AnalogGaugeTypeMinorTick_"
#define	CONFIG_KEY_ANALOG_GAUGE_MAJOR_TICK		"AnalogGaugeTypeMajorTick_"

#define CONFIG_KEY_DIGITAL_GAUGE_TYPE_COUNT		"DigitalGaugeTypeCount"
#define CONFIG_KEY_DIGITAL_GAUGE_TYPE			"DigitalGaugeType_"
#define CONFIG_KEY_DIGITAL_GAUGE_DIGITS			"DigitalGaugeTypeDigits_"
#define CONFIG_KEY_DIGITAL_GAUGE_PRECISION		"DigitalGaugeTypePrecision_"



#include <wx/arrimpl.cpp> // this is a magic incantation which must be done!
WX_DEFINE_OBJARRAY(ChartColors);

AppOptions::AppOptions()
	: 	m_serialPort(DEFAULT_COM_PORT) ,
		m_autoLoadConfig(DEFAULT_AUTO_LOAD_CONFIG)
{}

int AppOptions::GetSerialPort(){
	return m_serialPort;
}

void AppOptions::SetSerialPort(int comPort){
	m_serialPort = comPort;
}

bool & AppOptions::GetAutoLoadConfig(){
	return m_autoLoadConfig;
}

void AppOptions::SetAutoLoadConfig(bool autoConnect){
	m_autoLoadConfig = autoConnect;
}

ChartColors & AppOptions::GetChartColors(){
	return m_chartColors;
}

void AppOptions::SaveAppOptions(){
	wxConfig config(RACE_ANALYZER_APP_NAME);

	config.Write(CONFIG_KEY_COM_PORT,m_serialPort);
	config.Write(CONFIG_KEY_AUTO_LOAD, m_autoLoadConfig);
}

void AppOptions::LoadAppOptions(){
	wxConfig config(RACE_ANALYZER_APP_NAME);

	config.Read(CONFIG_KEY_COM_PORT,&m_serialPort, DEFAULT_COM_PORT);
	config.Read(CONFIG_KEY_AUTO_LOAD, &m_autoLoadConfig, DEFAULT_AUTO_LOAD_CONFIG);

	int standardChannelCount = 0;
	config.Read(CONFIG_KEY_DATALOG_CHANNEL_COUNT, &standardChannelCount, DEFAULT_DATALOG_CHANNEL_COUNT );

	if (DEFAULT_DATALOG_CHANNEL_COUNT == standardChannelCount){
		LoadDefaultStandardChannels(m_standardChannels);
	}

	int standardChannelTypesCount = 0;
	config.Read(CONFIG_KEY_DATALOG_CHANNEL_TYPE_COUNT, &standardChannelTypesCount, DEFAULT_DATALOG_CHANNEL_TYPES_COUNT );

	if (DEFAULT_DATALOG_CHANNEL_TYPES_COUNT == standardChannelTypesCount){
		LoadDefaultStandardChannelTypes(m_standardChannelTypes);
	}

	wxString chartColorString;
	config.Read(CONFIG_KEY_CHART_COLORS, &chartColorString, DEFAULT_CHART_COLORS);
	wxStringTokenizer colorSplitter(chartColorString,CHART_COLOR_STRING_DELIMITER);
	while (colorSplitter.HasMoreTokens()){
		wxString tok = colorSplitter.GetNextToken();
		long int color;
		tok.ToLong(&color,16);
		wxColour chartColor((color & 0xff0000) >> 16, (color & 0xff00) >> 8, color & 0xff);
		m_chartColors.Add(chartColor);
	}

	int analogGaugeTypeCount = 0;
	config.Read(CONFIG_KEY_ANALOG_GAUGE_TYPE_COUNT, &analogGaugeTypeCount, DEFAULT_ANALOG_GAUGE_TYPE_COUNT);
	if (DEFAULT_ANALOG_GAUGE_TYPE_COUNT == analogGaugeTypeCount){
		LoadDefaultAnalogGaugeTypes(m_analogGaugeTypes);
	}

	int digitalGaugeTypeCount = 0;
	config.Read(CONFIG_KEY_DIGITAL_GAUGE_TYPE_COUNT, &digitalGaugeTypeCount, DEFAULT_DIGITAL_GAUGE_TYPE_COUNT);
	if (DEFAULT_DIGITAL_GAUGE_TYPE_COUNT == digitalGaugeTypeCount){
		LoadDefaultDigitalGaugeTypes(m_digitalGaugeTypes);
	}

}

DatalogChannels & AppOptions::GetStandardChannels(){
	return m_standardChannels;
}

DatalogChannelTypes & AppOptions::GetStandardChannelTypes(){
	return m_standardChannelTypes;
}

AnalogGaugeTypes & AppOptions::GetAnalogGaugeTypes(){
	return m_analogGaugeTypes;
}

DigitalGaugeTypes & AppOptions::GetDigitalGaugeTypes(){
	return m_digitalGaugeTypes;
}

void AppOptions::LoadDefaultStandardChannelTypes(DatalogChannelTypes &types){

	types.Add( DatalogChannelType("Raw","Number", 0,0,1024) );
	types.Add( DatalogChannelType("GForce", "G", 5,-2.0,2.0) );
	types.Add( DatalogChannelType("Rotation", "Degrees/Sec", 5, -10.0, 10.0) );
	types.Add( DatalogChannelType("TimeDate", "UTC", 0, 0, 0) );
	types.Add( DatalogChannelType("Count", "Count", 0,0, 100.0) );
	types.Add( DatalogChannelSystemTypes::GetLatitudeChannelType());
	types.Add( DatalogChannelSystemTypes::GetLongitudeChannelType());
	types.Add( DatalogChannelType("Speed", "KPH", 0, 0, 300.0) );
	types.Add( DatalogChannelType("Volts", "Volts", 0, 0, 25.0) );
	types.Add( DatalogChannelType("Pressure", "PSI", 0, 0, 300.0) );
	types.Add( DatalogChannelType("Temperature", "F", 0, 0, 300.0) );
	types.Add( DatalogChannelType("Frequency", "Hz", 0, 0, 2000.0) );
	types.Add( DatalogChannelType("RPM", "RPM", 0, 0, 10000.0) );
	types.Add( DatalogChannelType("Duration", "Ms.", 0, 0, 100.0) );
	types.Add( DatalogChannelType("Percent", "%", 0, 0, 100.0) );
	types.Add( DatalogChannelType("Digital", "Off/On", 0, 0, 1) );
}

void AppOptions::LoadDefaultStandardChannels(DatalogChannels &channels){

	//Accelerometer inputs
	channels.Add( DatalogChannel("AccelX", 1, "Accelerometer X Axis") );
	channels.Add( DatalogChannel("AccelY", 1, "Accelerometer Y Axis") );
	channels.Add( DatalogChannel("AccelZ", 1, "Accelerometer Z Axis") );
	channels.Add( DatalogChannel("Yaw", 2, "Accelerometer Z Axis Rotation") );

	//GPS inputs
	channels.Add( DatalogChannel("GPSTime", 3, "GPS Time in UTC") );
	channels.Add( DatalogChannel("GPSQual", 4, "GPS signal quality indicator") );
	channels.Add( DatalogChannel("GPSSats", 4, "Number of Active Satellites") );
	channels.Add( DatalogChannel("GPSLatitude", 5, "GPS Latitude in Degrees") );
	channels.Add( DatalogChannel("GPSLongitude", 6,"GPS Longitude in Degrees") );
	channels.Add( DatalogChannel("GPSVelocity", 7, "GPS Velocity") );

	//Analog inputs
	channels.Add( DatalogChannel("Battery", 8, "Battery Voltage") );
	channels.Add( DatalogChannel("AnalogIn1", 8, "Analog Input 1") );
	channels.Add( DatalogChannel("AnalogIn2", 8, "Analog Input 2") );
	channels.Add( DatalogChannel("AnalogIn3", 8, "Analog Input 3") );
	channels.Add( DatalogChannel("AnalogIn4", 8, "Analog Input 4") );
	channels.Add( DatalogChannel("AnalogIn5", 8, "Analog Input 5") );
	channels.Add( DatalogChannel("AnalogIn6", 8, "Analog Input 6") );
	channels.Add( DatalogChannel("AnalogIn7", 8, "Analog Input 7") );
	channels.Add( DatalogChannel("OilPressure", 9, "Oil Pressure") );
	channels.Add( DatalogChannel("OilTemp", 10, "Engine Oil Temperature") );
	channels.Add( DatalogChannel("CoolantTemp", 10, "Engine Coolant Temperature") );

	//Frequency Inputs
	channels.Add( DatalogChannel("FrequencyIn1", 11, "Frequency Input 1") );
	channels.Add( DatalogChannel("FrequencyIn2", 11, "Frequency Input 2") );
	channels.Add( DatalogChannel("FrequencyIn3", 11, "Frequency Input 3") );
	channels.Add( DatalogChannel("EngineRPM", 12, "Engine Speed in RPM") );
	channels.Add( DatalogChannel("WheelSpeed", 12, "Wheel Speed in RPM") );
	channels.Add( DatalogChannel("PulseWidth", 13, "Signal Pulse width in Ms.") );
	channels.Add( DatalogChannel("DutyCycle", 14, "Signal Duty Cycle in %") );

	//Analog Outputs
	channels.Add( DatalogChannel("AnalogOut1", 8, "Analog Output 1") );
	channels.Add( DatalogChannel("AnalogOut2", 8, "Analog Output 2") );
	channels.Add( DatalogChannel("AnalogOut3", 8, "Analog Output 3") );
	channels.Add( DatalogChannel("AnalogOut4", 8, "Analog Output 4") );

	//Frequency Outputs
	channels.Add( DatalogChannel("FrequencyOut1", 11, "Frequency Output 1") );
	channels.Add( DatalogChannel("FrequencyOut2", 11, "Frequency Output 2") );
	channels.Add( DatalogChannel("FrequencyOut3", 11, "Frequency Output 3") );
	channels.Add( DatalogChannel("FrequencyOut4", 11, "Frequency Output 4") );

	//Digital Inputs
	channels.Add(DatalogChannel("DigitalIn1", 15, "Digital Input 1") );
	channels.Add(DatalogChannel("DigitalIn2", 15, "Digital Input 2") );
	channels.Add(DatalogChannel("DigitalIn3", 15, "Digital Input 3") );

	//Digital Outputs
	channels.Add(DatalogChannel("DigitalOut1", 15, "Digital Output 1") );
	channels.Add(DatalogChannel("DigitalOut2", 15, "Digital Output 2") );
	channels.Add(DatalogChannel("DigitalOut3", 15, "Digital Output 3") );
}

void AppOptions::LoadDefaultAnalogGaugeTypes(AnalogGaugeTypes &analogGaugeTypes){

	analogGaugeTypes["Raw"] = AnalogGaugeType(100, 50, 0);
	analogGaugeTypes["GForce"] = AnalogGaugeType(0.5, 0.1,2);
	analogGaugeTypes["Rotation"] = AnalogGaugeType(2.0, 1.0, 2);
	analogGaugeTypes["Count"] = AnalogGaugeType(10.0, 5.0, 1);
	analogGaugeTypes["Speed"] = AnalogGaugeType(20.0, 10.0, 0);
	analogGaugeTypes["Volts"] = AnalogGaugeType(5.0, 1.0, 2);
	analogGaugeTypes["Pressure"] = AnalogGaugeType(10.0, 5.0, 0);
	analogGaugeTypes["Temperature"] = AnalogGaugeType(10.0, 5.0, 0);
	analogGaugeTypes["Frequency"] = AnalogGaugeType(100.0, 50.0, 0);
	analogGaugeTypes["RPM"] = AnalogGaugeType(100.0, 50.0, 0);
	analogGaugeTypes["Duration"] = AnalogGaugeType(10.0, 5.0, 0);
	analogGaugeTypes["Percent"] = AnalogGaugeType(10.0, 5.0, 0);
}

void AppOptions::LoadDefaultDigitalGaugeTypes(DigitalGaugeTypes &digitalGaugeTypes){

	digitalGaugeTypes["Raw"] = DigitalGaugeType(3,0);
	digitalGaugeTypes["GForce"] = DigitalGaugeType(1,2);
	digitalGaugeTypes["Rotation"] = DigitalGaugeType(2,2);
	digitalGaugeTypes["Count"] = DigitalGaugeType(3,0);
	digitalGaugeTypes["Speed"] = DigitalGaugeType(3,0);
	digitalGaugeTypes["Volts"] = DigitalGaugeType(2,2);
	digitalGaugeTypes["Pressure"] = DigitalGaugeType(3,0);
	digitalGaugeTypes["Temperature"] = DigitalGaugeType(3,0);
	digitalGaugeTypes["Frequency"] = DigitalGaugeType(4,0);
	digitalGaugeTypes["RPM"] = DigitalGaugeType(4,0);
	digitalGaugeTypes["Duration"] = DigitalGaugeType(3,1);
	digitalGaugeTypes["Percent"] = DigitalGaugeType(2,0);
}
