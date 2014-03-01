#include "channelMeta.h"

static const ChannelMeta g_channelMeta = DEFAULT_CHANNEL_META;

const ChannelName * get_channel_name(size_t id){
	if (id >= CHANNEL_COUNT) id = 0;
	return &g_channelMeta.channelNames[id];
}

size_t filter_channel_name(size_t id){
	if (id >= CHANNEL_COUNT) id = 0;
	return id;
}
