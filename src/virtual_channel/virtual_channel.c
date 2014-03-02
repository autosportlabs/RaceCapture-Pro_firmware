#include "virtual_channel.h"
#include "mem_mang.h"
#include "mod_string.h"
#include "printk.h"
#include "virtual_channel_lock.h"

static size_t g_virtualChannelCount = 0;
static VirtualChannel *g_virtualChannels = NULL;

static int init_or_expand_virtual_channels(){
	size_t oldCount = g_virtualChannelCount;
	size_t newCount = oldCount + 1;
	size_t newSize = sizeof(VirtualChannel) * newCount;

	VirtualChannel *newVirtualChannels = (VirtualChannel *)portRealloc(g_virtualChannels, newSize);

	int result = 0;
	if (newVirtualChannels != NULL){
		g_virtualChannels = newVirtualChannels;
		g_virtualChannelCount++;
		result = 1;
	}
	return result;
}

int init_virtual_channels(){
	return init_virtual_channel_lock();
}

VirtualChannel * get_virtual_channel(size_t id){
	lock_virtual_channel();
	VirtualChannel *c;
	if (id < g_virtualChannelCount){
		c = g_virtualChannels + id;
	}
	else{
		c = NULL;
	}
	unlock_virtual_channel();
	return c;
}

int create_virtual_channel(int channelNameId, unsigned char precision, unsigned short sampleRate){

	lock_virtual_channel();
	int newChannelId = -1;
	if (init_or_expand_virtual_channels() && g_virtualChannelCount > 0){
		newChannelId = g_virtualChannelCount - 1;
		VirtualChannel * newChannel = g_virtualChannels + newChannelId;
		newChannel->config.channeNameId = channelNameId;
		newChannel->config.sampleRate = sampleRate;
		newChannel->precision = precision;
	}
	else{
		pr_error("could not allocate new channel\r\n");
	}
	configChanged();
	unlock_virtual_channel();
	return newChannelId;
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
