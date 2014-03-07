#include "channelMeta.h"
#include "mod_string.h"
#include "printk.h"
#include "memory.h"

#ifndef RCP_TESTING
#include "memory.h"
static const Channels g_channels __attribute__ ((aligned (FLASH_MEMORY_PAGE_SIZE))) __attribute__((section(".channels\n\t#")));
#else
static const Channels g_channels = DEFAULT_CHANNEL_META;
#endif

static const Channels g_defaultChannelMeta = DEFAULT_CHANNEL_META;

const Channel * get_channel(size_t id){
	if (id >= CHANNEL_COUNT) id = 0;
	return &g_channels.channels[id];
}

size_t filter_channel_id(size_t id){
	if (id >= CHANNEL_COUNT) id = 0;
	return id;
}

size_t find_channel_id(const char * name){
	for (size_t i = 0; i < CHANNEL_COUNT; i++){
		if (strcasecmp(name, g_channels.channels[i].label) == 0) return i;
	}
	return 0;
}

int flash_default_channels(void){
	pr_info("flashing default channels...");
	int result = memory_flash_region(&g_channels, &g_defaultChannelMeta, sizeof (Channels));
	if (result == 0) pr_info("success\r\n"); else pr_info("failed\r\n");
	return result;
}
