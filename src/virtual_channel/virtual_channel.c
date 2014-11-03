#include "virtual_channel.h"
#include "mem_mang.h"
#include "mod_string.h"
#include "printk.h"

static size_t g_virtualChannelCount = 0;
static VirtualChannel g_virtualChannels[MAX_VIRTUAL_CHANNELS];

VirtualChannel * get_virtual_channel(size_t id){
	if (id < g_virtualChannelCount)
		return g_virtualChannels + id;

	return NULL;
}

int create_virtual_channel(const ChannelConfig chCfg) {
	int newVirtualChannelId = -1;

	if (g_virtualChannelCount < MAX_VIRTUAL_CHANNELS){
		newVirtualChannelId = g_virtualChannelCount;
		VirtualChannel * newChannel = g_virtualChannels + newVirtualChannelId;
		newChannel->config = chCfg;
		newChannel->currentValue = 0;
		g_virtualChannelCount++;
	} else{
		pr_error("could not create virtual channel; limit reached\r\n");
	}

	configChanged();
	return newVirtualChannelId;
}

void set_virtual_channel_value(size_t id, float value){
	if (id < g_virtualChannelCount)	g_virtualChannels[id].currentValue = value;
}

float get_virtual_channel_value(int id){
	if (((size_t) id) >= g_virtualChannelCount) return 0.0;
	return g_virtualChannels[id].currentValue;
}

size_t get_virtual_channel_count(){
	return g_virtualChannelCount;
}
