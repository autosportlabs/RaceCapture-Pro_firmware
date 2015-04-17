#include "virtual_channel.h"
#include "mem_mang.h"
#include "mod_string.h"
#include "printk.h"
#include "loggerTaskEx.h"
#include "capabilities.h"

static size_t g_virtualChannelCount = 0;
static VirtualChannel g_virtualChannels[MAX_VIRTUAL_CHANNELS];

VirtualChannel * get_virtual_channel(size_t id){
	if (id < g_virtualChannelCount)
		return g_virtualChannels + id;

	return NULL;
}

int find_virtual_channel(const char * channel_name){
	for (size_t i = 0; i < g_virtualChannelCount; i++){
		if (strcmp(channel_name, g_virtualChannels[i].config.label) == 0) return i;
	}
	return INVALID_VIRTUAL_CHANNEL;
}

int create_virtual_channel(const ChannelConfig chCfg) {

	int virtualChannelId = find_virtual_channel(chCfg.label);

	if (virtualChannelId == INVALID_VIRTUAL_CHANNEL){
		if (g_virtualChannelCount < MAX_VIRTUAL_CHANNELS){
			virtualChannelId = g_virtualChannelCount;
			g_virtualChannelCount++;
		}
	}
	if (virtualChannelId != INVALID_VIRTUAL_CHANNEL){
		VirtualChannel * channel = g_virtualChannels + virtualChannelId;
		channel->config = chCfg;
		channel->currentValue = 0;
		configChanged();
	} else{
		pr_error("vchan: limit exceeded\r\n");
	}
	return virtualChannelId;
}

void set_virtual_channel_value(size_t id, float value){
	if (id < g_virtualChannelCount)	g_virtualChannels[id].currentValue = value;
}

float get_virtual_channel_value(int id){
	if (((size_t) id) >= g_virtualChannelCount) return 0.0;
	return g_virtualChannels[id].currentValue;
}

size_t get_virtual_channel_count(void){
	return g_virtualChannelCount;
}

void reset_virtual_channels(void){
	g_virtualChannelCount = 0;
}
