#ifndef CHANNELMETA_H_
#define CHANNELMETA_H_
#include "stddef.h"

#define CHANNEL_COUNT			58
#define DEFAULT_LABEL_LENGTH	11
#define DEFAULT_UNITS_LENGTH	11

typedef struct _ChannelName{
	char label[DEFAULT_LABEL_LENGTH];
	char units[DEFAULT_UNITS_LENGTH];
} ChannelName;

typedef struct _ChannelMeta{
	size_t count;
	ChannelName channelNames[CHANNEL_COUNT];
} ChannelMeta;

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
{"Unknown", ""}, \
{"Analog1", ""}, \
{"Analog2", ""}, \
{"Analog3", ""}, \
{"Analog4", ""}, \
{"Analog5", ""}, \
{"Analog6", ""}, \
{"Analog7", ""}, \
{"Analog8", ""}, \
{"Freq1", ""}, \
{"Freq2", ""}, \
{"Freq3", ""}, \
{"GPIO1", ""}, \
{"GPIO2", ""}, \
{"GPIO3", ""}, \
{"PWM1", ""}, \
{"PWM2", ""}, \
{"PWM3", ""}, \
{"PWM4", ""}, \
{"EngineTemp", "F"}, \
{"FuelLevel", "%"}, \
{"OilPress", "PSI"}, \
{"OilTemp", "F"}, \
{"AFR", "AFR"}, \
{"IAT", "F"}, \
{"MAP", "kPa"}, \
{"Boost", "PSI"}, \
{"EGT", "F"}, \
{"TPS", "%"}, \
{"Battery", "Volts"}, \
{"FuelPress", "PSI"}, \
{"Injector", "ms"}, \
{"RPM", "RPM"}, \
{"Wheel", "RPM"}, \
{"Gear", "Count"}, \
{"Steering", "degrees"}, \
{"Brake", "PSI"}, \
{"LF_Height", "mm"}, \
{"RF_Height", "mm"}, \
{"LR_Height", "mm"}, \
{"RR_Height", "mm"}, \
{"AccelX", "G"}, \
{"AccelY", "G"}, \
{"AccelZ", "G"}, \
{"Yaw", "Deg/sec"}, \
{"Pitch", "Deg/sec"}, \
{"Roll", "Deg/sec"}, \
{"GPSSats", "Count"}, \
{"Time", "Time"}, \
{"Latitude", "Degrees"}, \
{"Longitude", "Degrees"}, \
{"Speed", "MPH"}, \
{"SectorTime", "Time"}, \
{"SplitTime", "Time"}, \
{"LapTime", "Time"}, \
{"PredTime", "Time"}, \
{"LapCount", "Count"}, \
{"Distance", "Miles"} \
} \
}

const ChannelName * get_channel_name(size_t id);
size_t filter_channel_name(size_t id);


#endif /* CHANNELMETA_H_ */
