#include "virtual_channel.h"
#include "mem_mang.h"
#include "mod_string.h"
#include "printk.h"
#include "channelMeta.h"

static size_t g_virtualChannelCount = 0;
static VirtualChannel g_virtualChannels[MAX_VIRTUAL_CHANNELS];

VirtualChannel * get_virtual_channel(size_t id){
	VirtualChannel *c;
	if (id < g_virtualChannelCount){
		c = g_virtualChannels + id;
	}
	else{
		c = NULL;
	}
	return c;
}

int create_virtual_channel(const char *name, unsigned short sampleRate){
	int newVirtualChannelId = -1;

	unsigned short channelId = find_channel_id(name);

	if (g_virtualChannelCount < MAX_VIRTUAL_CHANNELS){
		newVirtualChannelId = g_virtualChannelCount;
		VirtualChannel * newChannel = g_virtualChannels + newVirtualChannelId;
		newChannel->config.channeId = channelId;
		newChannel->config.sampleRate = sampleRate;
		newChannel->currentValue = 0;
		g_virtualChannelCount++;
	}
	else{
		pr_error("could not create virtual channel; limit reached\r\n");
	}
	configChanged();
	return newVirtualChannelId;
}

void set_virtual_channel_value(size_t id, float value){
	if (id < g_virtualChannelCount)	g_virtualChannels[id].currentValue = value;
}

float get_virtual_channel_value(int id){
	float value = 0;
	if (id < g_virtualChannelCount) value = g_virtualChannels[id].currentValue;
	return value;
}

size_t get_virtual_channel_count(){
	return g_virtualChannelCount;
}
