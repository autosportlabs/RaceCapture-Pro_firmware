#ifndef CHANNELMETA_H_
#define CHANNELMETA_H_
#include "stddef.h"

#define CHANNEL_COUNT			100
#define DEFAULT_LABEL_LENGTH	11
#define DEFAULT_UNITS_LENGTH	11

typedef struct _ChannelName{
	char label[DEFAULT_LABEL_LENGTH];
	char units[DEFAULT_UNITS_LENGTH];
	unsigned char precision;
	unsigned char flags;
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

#include "system_channels.h"


#endif /* CHANNELMETA_H_ */
