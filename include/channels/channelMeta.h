#ifndef CHANNELMETA_H_
#define CHANNELMETA_H_
#include "stddef.h"

#define CHANNEL_COUNT			59
#define DEFAULT_LABEL_LENGTH	11
#define DEFAULT_UNITS_LENGTH	11

typedef struct _ChannelName{
	char label[DEFAULT_LABEL_LENGTH];
	char units[DEFAULT_UNITS_LENGTH];
	unsigned char precision;
} Channel;

typedef struct _Channels{
	size_t count;
	Channel channels[CHANNEL_COUNT];
} Channels;

int flash_default_channels(void);
const Channel * get_channel(size_t id);
const Channels * get_channels();
size_t filter_channel_id(size_t id);
size_t find_channel_id(const char * name);

#define DEFAULT_GPS_POSITION_PRECISION 		6
#define DEFAULT_GPS_RADIUS_PRECISION 		5
#define DEFAULT_VOLTAGE_SCALING_PRECISION	2
#define DEFAULT_ANALOG_SCALING_PRECISION	2



typedef enum{
CHANNEL_Unknown = 0,
CHANNEL_Analog1,
CHANNEL_Analog2,
CHANNEL_Analog3,
CHANNEL_Analog4,
CHANNEL_Analog5,
CHANNEL_Analog6,
CHANNEL_Analog7,
CHANNEL_Analog8,
CHANNEL_Freq1,
CHANNEL_Freq2,
CHANNEL_Freq3,
CHANNEL_GPIO1,
CHANNEL_GPIO2,
CHANNEL_GPIO3,
CHANNEL_PWM1,
CHANNEL_PWM2,
CHANNEL_PWM3,
CHANNEL_PWM4,
CHANNEL_Coolant,
CHANNEL_EngineTemp,
CHANNEL_FuelLevel,
CHANNEL_OilPress,
CHANNEL_OilTemp,
CHANNEL_AFR,
CHANNEL_IAT,
CHANNEL_MAP,
CHANNEL_Boost,
CHANNEL_EGT,
CHANNEL_TPS,
CHANNEL_Battery,
CHANNEL_FuelPress,
CHANNEL_Injector,
CHANNEL_RPM,
CHANNEL_Wheel,
CHANNEL_Gear,
CHANNEL_Steering,
CHANNEL_Brake,
CHANNEL_LF_Height,
CHANNEL_RF_Height,
CHANNEL_LR_Height,
CHANNEL_RR_Height,
CHANNEL_AccelX,
CHANNEL_AccelY,
CHANNEL_AccelZ,
CHANNEL_Yaw,
CHANNEL_Pitch,
CHANNEL_Roll,
CHANNEL_GPSSats,
CHANNEL_Time,
CHANNEL_Latitude,
CHANNEL_Longitude,
CHANNEL_Speed,
CHANNEL_SectorTime,
CHANNEL_SplitTime,
CHANNEL_LapTime,
CHANNEL_PredTime,
CHANNEL_LapCount,
CHANNEL_Distance
} ChannelIds;

#define DEFAULT_CHANNEL_META { \
CHANNEL_COUNT, \
{ \
{"Unknown", "", 0}, \
{"Analog1", "", 0}, \
{"Analog2", "", 0}, \
{"Analog3", "", 0}, \
{"Analog4", "", 0}, \
{"Analog5", "", 0}, \
{"Analog6", "", 0}, \
{"Analog7", "", 0}, \
{"Analog8", "", 0}, \
{"Freq1", "", 0}, \
{"Freq2", "", 0}, \
{"Freq3", "", 0}, \
{"GPIO1", "", 0}, \
{"GPIO2", "", 0}, \
{"GPIO3", "", 0}, \
{"PWM1", "", 0}, \
{"PWM2", "", 0}, \
{"PWM3", "", 0}, \
{"PWM4", "", 0}, \
{"Coolant", "F", 0}, \
{"EngineTemp", "F", 0}, \
{"FuelLevel", "%", 0}, \
{"OilPress", "PSI", 0}, \
{"OilTemp", "F", 0}, \
{"AFR", "AFR", 2}, \
{"IAT", "F", 0}, \
{"MAP", "kPa", 0}, \
{"Boost", "PSI", 1}, \
{"EGT", "F", 0}, \
{"TPS", "%", 0}, \
{"Battery", "Volts", 2}, \
{"FuelPress", "PSI", 0}, \
{"Injector", "ms", 0}, \
{"RPM", "RPM", 0}, \
{"Wheel", "RPM", 0}, \
{"Gear", "Count", 0}, \
{"Steering", "degrees", 0}, \
{"Brake", "PSI", 0}, \
{"LF_Height", "mm", 0}, \
{"RF_Height", "mm", 0}, \
{"LR_Height", "mm", 0}, \
{"RR_Height", "mm", 0}, \
{"AccelX", "G", 2}, \
{"AccelY", "G", 2}, \
{"AccelZ", "G", 2}, \
{"Yaw", "Deg/sec", 0}, \
{"Pitch", "Deg/sec", 0}, \
{"Roll", "Deg/sec", 0}, \
{"GPSSats", "Count", 0}, \
{"Time", "Time", 6}, \
{"Latitude", "Degrees", 6}, \
{"Longitude", "Degrees", 6}, \
{"Speed", "MPH", 2}, \
{"SectorTime", "Time", 3}, \
{"SplitTime", "Time", 3}, \
{"LapTime", "Time", 3}, \
{"PredTime", "Time", 3}, \
{"LapCount", "Count", 0}, \
{"Distance", "Miles", 3} \
} \
}

#endif /* CHANNELMETA_H_ */
