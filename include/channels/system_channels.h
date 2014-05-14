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
CHANNEL_GPSSats = 48 ,
CHANNEL_Time = 49 ,
CHANNEL_Latitude = 50 ,
CHANNEL_Longitude = 51 ,
CHANNEL_Speed = 52 ,
CHANNEL_Sector = 53 ,
CHANNEL_SectorTime = 54 ,
CHANNEL_LapTime = 55 ,
CHANNEL_PredTime = 56 ,
CHANNEL_LapCount = 57 ,
CHANNEL_Distance = 58 
} ChannelIds;

#define DEFAULT_CHANNEL_META { \
CHANNEL_COUNT, \
{ \
{"Unknown", "", 0} , \
{"Analog1", "", 2} , \
{"Analog2", "", 2} , \
{"Analog3", "", 2} , \
{"Analog4", "", 2} , \
{"Analog5", "", 2} , \
{"Analog6", "", 2} , \
{"Analog7", "", 2} , \
{"Analog8", "", 2} , \
{"Freq1", "", 0} , \
{"Freq2", "", 0} , \
{"Freq3", "", 0} , \
{"GPIO1", "", 0} , \
{"GPIO2", "", 0} , \
{"GPIO3", "", 0} , \
{"PWM1", "", 0} , \
{"PWM2", "", 0} , \
{"PWM3", "", 0} , \
{"PWM4", "", 0} , \
{"Coolant", "F", 0} , \
{"EngineTemp", "F", 0} , \
{"FuelLevel", "F", 0} , \
{"OilPress", "PSI", 0} , \
{"OilTemp", "F", 0} , \
{"AFR", "ratio", 2} , \
{"IAT", "F", 0} , \
{"MAP", "KPa", 0} , \
{"Boost", "PSI", 0} , \
{"EGT", "F", 0} , \
{"TPS", "%", 0} , \
{"Battery", "Volts", 0} , \
{"FuelPress", "PSI", 0} , \
{"InjectorPW", "ms", 2} , \
{"RPM", "", 0} , \
{"Wheel", "RPM", 0} , \
{"Gear", "", 0} , \
{"Steering", "%", 0} , \
{"Brake", "PSI", 0} , \
{"LF_Height", "mm", 0} , \
{"RF_Height", "mm", 0} , \
{"LR_Height", "mm", 0} , \
{"RR_Height", "mm", 0} , \
{"AccelX", "G", 2} , \
{"AccelY", "G", 2} , \
{"AccelZ", "G", 2} , \
{"Yaw", "G", 0} , \
{"Pitch", "G", 0} , \
{"Roll", "G", 0} , \
{"GPSSats", "", 0} , \
{"Time", "", 3} , \
{"Latitude", "Degrees", 6} , \
{"Longitude", "Degrees", 6} , \
{"Speed", "MPH", 0} , \
{"Sector", "", 0} , \
{"SectorTime", "", 3} , \
{"LapTime", "", 3} , \
{"PredTime", "", 3} , \
{"LapCount", "", 0} , \
{"Distance", "", 3}  \
} \
}
#endif
