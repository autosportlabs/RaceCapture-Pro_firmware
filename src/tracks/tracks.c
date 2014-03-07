#include "tracks.h"
#include "mod_string.h"
#include "printk.h"
#include "memory.h"

#ifndef RCP_TESTING
#include "memory.h"
static const Tracks g_tracks __attribute__ ((aligned (FLASH_MEMORY_PAGE_SIZE))) __attribute__((section(".tracks\n\t#")));
#else
static const Tracks g_tracks = DEFAULT_TRACKS;
#endif

static const Tracks g_defaultTracks = DEFAULT_TRACKS;

int flash_default_tracks(void){
	pr_info("flashing default tracks...");
	int result = memory_flash_region(&g_tracks, &g_defaultTracks, sizeof (g_defaultTracks));
	if (result == 0) pr_info("success\r\n"); else pr_info("failed\r\n");
	return result;
}

const Tracks * get_tracks(){
	return &g_tracks;
}
