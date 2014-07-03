#ifndef SYSTEMCHANNELS_H_
#define SYSTEMCHANNELS_H_

typedef enum{
CHANNEL_Unknown = 0 ,
CHANNEL_Analog1 = 1 ,
CHANNEL_Analog2 = 2 ,
CHANNEL_Analog3 = 3 ,
CHANNEL_Analog4 = 4 ,
CHANNEL_Analog5 = 5 ,
CHANNEL_Analog6 = 6 ,
CHANNEL_Analog7 = 7 ,
CHANNEL_Analog8 = 8 ,
CHANNEL_Freq1 = 9 ,
CHANNEL_Freq2 = 10 ,
CHANNEL_Freq3 = 11 ,
CHANNEL_GPIO1 = 12 ,
CHANNEL_GPIO2 = 13 ,
CHANNEL_GPIO3 = 14 ,
CHANNEL_PWM1 = 15 ,
CHANNEL_PWM2 = 16 ,
CHANNEL_PWM3 = 17 ,
CHANNEL_PWM4 = 18 ,
CHANNEL_Coolant = 19 ,
CHANNEL_EngineTemp = 20 ,
CHANNEL_FuelLevel = 21 ,
CHANNEL_OilPress = 22 ,
CHANNEL_OilTemp = 23 ,
CHANNEL_AFR = 24 ,
CHANNEL_IAT = 25 ,
CHANNEL_MAP = 26 ,
CHANNEL_Boost = 27 ,
CHANNEL_EGT = 28 ,
CHANNEL_TPS = 29 ,
CHANNEL_Battery = 30 ,
CHANNEL_FuelPress = 31 ,
CHANNEL_InjectorPW = 32 ,
CHANNEL_RPM = 33 ,
CHANNEL_Wheel = 34 ,
CHANNEL_Gear = 35 ,
CHANNEL_Steering = 36 ,
CHANNEL_Brake = 37 ,
CHANNEL_LF_Height = 38 ,
CHANNEL_RF_Height = 39 ,
CHANNEL_LR_Height = 40 ,
CHANNEL_RR_Height = 41 ,
CHANNEL_AccelX = 42 ,
CHANNEL_AccelY = 43 ,
CHANNEL_AccelZ = 44 ,
CHANNEL_Yaw = 45 ,
CHANNEL_Pitch = 46 ,
CHANNEL_Roll = 47 ,
CHANNEL_Heading = 48 ,
CHANNEL_GPSSats = 49 ,
CHANNEL_Time = 50 ,
CHANNEL_Latitude = 51 ,
CHANNEL_Longitude = 52 ,
CHANNEL_Speed = 53 ,
CHANNEL_Distance = 54 ,
CHANNEL_Sector = 55 ,
CHANNEL_SectorTime = 56 ,
CHANNEL_LapTime = 57 ,
CHANNEL_PredTime = 58 ,
CHANNEL_LapCount = 59 
} ChannelIds;

#define DEFAULT_CHANNEL_META { \
60, \
{ \
{"Unknown", "", 0, 1, 0, 1024} , \
{"Analog1", "", 2, 3, 0, 5} , \
{"Analog2", "", 2, 3, 0, 5} , \
{"Analog3", "", 2, 3, 0, 5} , \
{"Analog4", "", 2, 3, 0, 5} , \
{"Analog5", "", 2, 3, 0, 5} , \
{"Analog6", "", 2, 3, 0, 5} , \
{"Analog7", "", 2, 3, 0, 5} , \
{"Analog8", "", 2, 3, 0, 5} , \
{"Freq1", "", 0, 5, 0, 1000} , \
{"Freq2", "", 0, 5, 0, 1000} , \
{"Freq3", "", 0, 5, 0, 1000} , \
{"GPIO1", "", 0, 7, 0, 1} , \
{"GPIO2", "", 0, 7, 0, 1} , \
{"GPIO3", "", 0, 7, 0, 1} , \
{"PWM1", "", 0, 9, 0, 100} , \
{"PWM2", "", 0, 9, 0, 100} , \
{"PWM3", "", 0, 9, 0, 100} , \
{"PWM4", "", 0, 9, 0, 100} , \
{"Coolant", "F", 0, 3, 0, 300} , \
{"EngineTemp", "F", 0, 3, 0, 300} , \
{"FuelLevel", "F", 0, 3, 0, 100} , \
{"OilPress", "PSI", 0, 3, 0, 150} , \
{"OilTemp", "F", 0, 3, 0, 300} , \
{"AFR", "ratio", 2, 3, 0, 20} , \
{"IAT", "F", 0, 3, 0, 300} , \
{"MAP", "KPa", 0, 3, 0, 255} , \
{"Boost", "PSI", 0, 3, 0, 50} , \
{"EGT", "F", 0, 3, 0, 2000} , \
{"TPS", "%", 0, 3, 0, 100} , \
{"Battery", "Volts", 2, 3, 0, 20} , \
{"FuelPress", "PSI", 0, 3, 0, 150} , \
{"InjectorPW", "ms", 2, 5, 0, 100} , \
{"RPM", "", 0, 5, 0, 10000} , \
{"Wheel", "RPM", 0, 5, 0, 5000} , \
{"Gear", "", 0, 3, 0, 10} , \
{"Steering", "%", 0, 3, -100, 100} , \
{"Brake", "PSI", 0, 3, 0, 2000} , \
{"LF_Height", "mm", 0, 3, 0, 255} , \
{"RF_Height", "mm", 0, 3, 0, 255} , \
{"LR_Height", "mm", 0, 3, 0, 255} , \
{"RR_Height", "mm", 0, 3, 0, 255} , \
{"AccelX", "G", 2, 11, -3, 3} , \
{"AccelY", "G", 2, 11, -3, 3} , \
{"AccelZ", "G", 2, 11, -3, 3} , \
{"Yaw", "Deg/Sec", 1, 11, -300, 300} , \
{"Pitch", "G", 0, 11, -300, 300} , \
{"Roll", "G", 0, 11, -300, 300} , \
{"Heading", "Degrees", 0, 11, 1, 360} , \
{"GPSSats", "", 0, 13, 0, 100} , \
{"Time", "", 6, 13, 0, 0} , \
{"Latitude", "Degrees", 6, 13, -180, 180} , \
{"Longitude", "Degrees", 6, 13, -180, 180} , \
{"Speed", "MPH", 2, 13, 0, 150} , \
{"Distance", "Miles", 3, 13, 0, 0} , \
{"Sector", "Count", 0, 15, 0, 100} , \
{"SectorTime", "Min", 3, 15, 0, 0} , \
{"LapTime", "Min", 3, 15, 0, 0} , \
{"PredTime", "Min", 3, 15, 0, 0} , \
{"LapCount", "Count", 0, 15, 0, 1000}  \
} \
}
#endif
