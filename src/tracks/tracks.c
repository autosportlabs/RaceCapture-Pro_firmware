#include "tracks.h"
#include "mod_string.h"
#include "printk.h"
#include "memory.h"
#include "mem_mang.h"

#ifndef RCP_TESTING
#include "memory.h"
static const Tracks g_tracks __attribute__ ((aligned (FLASH_MEMORY_PAGE_SIZE))) __attribute__((section(".tracks\n\t#")));
#else
static const Tracks g_tracks = DEFAULT_TRACKS;
#endif

static const Tracks g_defaultTracks = DEFAULT_TRACKS;

static Tracks *g_tracksBuffer = NULL;

int flash_default_tracks(void){
	pr_info("flashing default tracks...");
	return flash_tracks(&g_defaultTracks, sizeof (g_defaultTracks));
}

int flash_tracks(const Tracks *source, size_t rawSize){
	int result = memory_flash_region(&g_tracks, source, rawSize);
	if (result == 0) pr_info("success\r\n"); else pr_info("failed\r\n");
	return result;
}

const Tracks * get_tracks(){
	return &g_tracks;
}

#define TRACK_UPDATE_RESULT_OK  		1
#define TRACK_UPDATE_RESULT_FAIL  		0

#define TRACK_UPDATE_MODE_MIDDLE 		0
#define TRACK_UPDATE_MODE_START 		1
#define TRACK_UPDATE_MODE_COMPLETE 		2

int update_track(const Track *track, size_t index, int mode){
	int result = TRACK_UPDATE_RESULT_OK;
	if (mode == TRACK_UPDATE_MODE_START){
		if (g_tracksBuffer != NULL){
			pr_info("freeing previous tracks buffer\r\n");
			portFree(g_tracksBuffer);
		}
		g_tracksBuffer = (Tracks *)portMalloc(sizeof(Tracks));
		if (g_tracksBuffer != NULL){
			memcpy(g_tracksBuffer, &g_tracks, sizeof(Tracks));
		}
		else{
			pr_error("could not allocate buffer for tracks\r\n");
			result = TRACK_UPDATE_RESULT_FAIL;
		}
	}

	if (result == TRACK_UPDATE_RESULT_OK){
		Track *trackToUpdate = g_tracksBuffer->tracks + index;
		memcpy(trackToUpdate, track, sizeof(Track));
		if (index + 1 > g_tracksBuffer->count) g_tracksBuffer->count = index + 1;
	}

	if (mode == TRACK_UPDATE_MODE_COMPLETE){
		if (g_tracksBuffer != NULL){
			if (flash_tracks(g_tracksBuffer, sizeof(Tracks))){
				pr_info("completed updating tracks buffer, flashing\r\n");
			}
			else{
				pr_error("failed to flash tracks\r\n");
				result = TRACK_UPDATE_RESULT_FAIL;
			}
			portFree(g_tracksBuffer);
			g_tracksBuffer = NULL;
		}
	}
	return result;
}
