#ifndef CHANNELMETA_H_
#define CHANNELMETA_H_
#include <stdint.h>
#include "stddef.h"

#define CHANNEL_ADD_RESULT_OK  		 	1
#define CHANNEL_ADD_RESULT_FAIL  		0

#define CHANNEL_ADD_MODE_IN_PROGRESS	1
#define CHANNEL_ADD_MODE_COMPLETE 		2

#define MAX_CHANNEL_COUNT				90

/* KILL_ME
#define CHANNEL_SYSTEM_CHANNEL_FLAG 	0

#define CHANNEL_TYPE_UNKNOWN			0
#define CHANNEL_TYPE_ANALOG				1
#define CHANNEL_TYPE_FREQ				2
#define CHANNEL_TYPE_GPIO				3
#define CHANNEL_TYPE_PWM				4
#define CHANNEL_TYPE_IMU				5
#define CHANNEL_TYPE_GPS				6
#define CHANNEL_TYPE_STATISTICS			7
*/

#define MAGIC_NUMBER_CHANNEL_INIT		0xDEADBEE7

void initialize_channels();
int flash_default_channels(void);
const Channel * get_channel(size_t id);
const Channels * get_channels();
size_t filter_channel_id(size_t id);
size_t find_channel_id(const char * name);
int is_channel_type(const Channel *channel, unsigned char type);
unsigned char get_channel_type(const Channel *channel);
void set_channel_type(Channel *channel, unsigned char type);
int is_system_channel(const Channel *channel);
int flash_channels(const Channels *source, size_t rawSize);
int add_channel(const Channel *channel, int mode, size_t index);

#include "system_channels.h"


#endif /* CHANNELMETA_H_ */
