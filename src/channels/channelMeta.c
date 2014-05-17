#include "channelMeta.h"
#include "mod_string.h"
#include "printk.h"
#include "memory.h"
#include "mem_mang.h"

#ifndef RCP_TESTING
#include "memory.h"
static const Channels g_channels __attribute__ ((aligned (FLASH_MEMORY_PAGE_SIZE))) __attribute__((section(".channels\n\t#")));
#else
static const Channels g_channels = DEFAULT_CHANNEL_META;
#endif

#define CHANNEL_ADD_RESULT_OK  		 	1
#define CHANNEL_ADD_RESULT_FAIL  		0

#define CHANNEL_ADD_MODE_IN_PROGRESS	0
#define CHANNEL_ADD_MODE_COMPLETE 		2

static const Channels g_defaultChannelMeta = DEFAULT_CHANNEL_META;
static Channels * g_channelsMetaBuffer = NULL;

int is_system_channel(const Channel *channel){
	return channel != NULL && (channel->flags & CHANNEL_SYSTEM_CHANNEL_FLAG);
}

const Channels * get_channels(){
	return &g_channels;
}

const Channel * get_channel(size_t id){
	if (id >= MAX_CHANNEL_COUNT) id = 0;
	return &g_channels.channels[id];
}

size_t filter_channel_id(size_t id){
	if (id >= MAX_CHANNEL_COUNT) id = 0;
	return id;
}

size_t find_channel_id(const char * name){
	for (size_t i = 0; i < MAX_CHANNEL_COUNT; i++){
		if (strcasecmp(name, g_channels.channels[i].label) == 0) return i;
	}
	return 0;
}

int flash_default_channels(void){
	pr_info("flashing default channels...");
	int result = flash_channels(&g_defaultChannelMeta, sizeof (g_defaultChannelMeta));
	return result;
}

int flash_channels(const Channels *source, size_t rawSize){
	int result = memory_flash_region(source, &g_defaultChannelMeta, rawSize);
	if (result == 0) pr_info("success\r\n"); else pr_info("failed\r\n");
	return result;
}

int add_channel(const Channel *channel, int mode,  size_t index){
	int result = CHANNEL_ADD_RESULT_OK;

	if (index < MAX_CHANNEL_COUNT){
		if (mode == CHANNEL_ADD_MODE_IN_PROGRESS || mode == CHANNEL_ADD_MODE_COMPLETE){
			if (g_channelsMetaBuffer == NULL){
				pr_info("allocating new channels buffer\r\n");
				g_channelsMetaBuffer = (Channels *)portMalloc(sizeof(Channels));
				memcpy(g_channelsMetaBuffer, &g_channels, sizeof(Channels));
			}

			if (g_channelsMetaBuffer != NULL){
				Channel *channelToAdd = g_channelsMetaBuffer->channels + index;
				memcpy(channelToAdd, channel, sizeof(Channel));
				g_channelsMetaBuffer->count = index + 1;

				if (mode == CHANNEL_ADD_MODE_COMPLETE){
					pr_info("completed updating channels, flashing: ");
					if (flash_channels(g_channelsMetaBuffer, sizeof(Channels)) == 0){
						pr_info("success\r\n");
					}
					else{
						pr_error("error\r\n");
						result = CHANNEL_ADD_RESULT_FAIL;
					}
					portFree(g_channelsMetaBuffer);
					g_channelsMetaBuffer = NULL;
				}
			}
			else{
				pr_error("could not allocate buffer for tracks\r\n");
				result = CHANNEL_ADD_RESULT_FAIL;
			}
		}
	}
	else{
		pr_error("invalid channel index\r\n");
		result = CHANNEL_ADD_RESULT_FAIL;
	}
	return result;
}
