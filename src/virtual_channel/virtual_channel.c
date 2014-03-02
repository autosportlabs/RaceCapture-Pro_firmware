#include "virtual_channel.h"
#include "mem_mang.h"
#include "mod_string.h"
#include "printk.h"

static size_t g_virtualChannelCount = 0;
static VirtualChannel *g_virtualChannels = NULL;


static int init_or_expand_virtual_channels(){
	size_t newCount = g_virtualChannelCount + 1;
	size_t oldSize = sizeof(VirtualChannel) * g_virtualChannelCount;
	size_t newSize = sizeof(VirtualChannel) * newCount;

	VirtualChannel *newVirtualChannels = (VirtualChannel *) portMalloc(newSize);

	int result = 0;
	if (newVirtualChannels != NULL){
		memset(newVirtualChannels, 0, newSize);
		if (g_virtualChannelCount > 0){
			memcpy(newVirtualChannels, g_virtualChannels, oldSize);
			portFree(g_virtualChannels);
		}
		g_virtualChannelCount = newCount;
		g_virtualChannels = newVirtualChannels;
		result = 1;
	}
	return result;
}

int create_virtual_channel(int channelNameId, unsigned char sampleRate){

	int newChannelId = -1;
	if (init_or_expand_virtual_channels() && g_virtualChannelCount > 0){
		newChannelId = g_virtualChannelCount - 1;
		VirtualChannel * newChannel = g_virtualChannels + newChannelId;
		newChannel->config.channeNameId = channelNameId;
		newChannel->config.sampleRate = sampleRate;
	}
	else{
		pr_error("could not allocate new channel\r\n");
	}
	return newChannelId;
}

void set_virtual_channel_value(size_t id, float value){
	if (id < g_virtualChannelCount)	g_virtualChannels[id].currentValue = value;
}

float get_virtual_channel_value(size_t id){
	float value = 0;
	if (id < g_virtualChannelCount) value = g_virtualChannels[id].currentValue;
	return value;
}


