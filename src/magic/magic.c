#include "magic.h"
#include "memory.h"
#include "mod_string.h"
#include "printk.h"
#include "loggerConfig.h"
#include "channelMeta.h"
#include "tracks.h"
#include "luaScript.h"


#ifndef RCP_TESTING
#include "memory.h"
const MagicInfo g_saved_magic_info __attribute__ ((aligned (FLASH_MEMORY_PAGE_SIZE))) __attribute__((section(".magic\n\t#")));
#else
const MagicInfo g_saved_magic_info = {{0,0,0},0,0,0,0};
#endif


static MagicInfo g_working_magic_info;

static void sync_magic_info_version(){
	VersionInfo *version = &g_working_magic_info.current_version;
	version->major = MAJOR_REV;
	version->minor = MINOR_REV;
	version->bugfix = BUGFIX_REV;
}

static int firmware_major_version_matches_last(){
	VersionInfo * version = &g_working_magic_info.current_version;
	return version->major == MAJOR_REV;
}

static int firmware_version_matches_last(){
	VersionInfo * version = &g_working_magic_info.current_version;
	return version->major == MAJOR_REV && version->minor == MINOR_REV && version->bugfix == BUGFIX_REV;
}

int is_script_init(){
	return g_working_magic_info.script_init == MAGIC_INFO_SCRIPT_IS_INIT;
}

int is_config_init(){
	return g_working_magic_info.config_init == MAGIC_INFO_CONFIG_IS_INIT;
}

int is_channels_init(){
	return g_working_magic_info.channels_init == MAGIC_INFO_CHANNELS_IS_INIT;
}

int is_tracks_init(){
	return g_working_magic_info.tracks_init == MAGIC_INFO_TRACKS_IS_INIT;
}

void initialize_magic_info(){
	memcpy(&g_working_magic_info,&g_saved_magic_info,sizeof(MagicInfo));

	int config_valid = is_config_init();
	int channels_valid = is_channels_init();
	int script_valid = is_script_init();
	int tracks_valid = is_tracks_init();

	int firmware_matches_last = firmware_version_matches_last();
	if (!firmware_matches_last){
		pr_info("new firmware detected\r\n");
	}

	int firmware_major_matches_last = firmware_major_version_matches_last();
	if (!firmware_major_matches_last){
		pr_info("firmware major version changed\r\n");
	}

	if (!config_valid || !firmware_major_matches_last){
		if (flash_default_logger_config() == 0) g_working_magic_info.config_init = MAGIC_INFO_CONFIG_IS_INIT;
	}

	if (!channels_valid || !firmware_major_matches_last) {
		if (flash_default_channels() == 0) g_working_magic_info.channels_init = MAGIC_INFO_CHANNELS_IS_INIT;
	}

	if (!script_valid || !firmware_major_matches_last){
		if (flash_default_script() == 0) g_working_magic_info.script_init = MAGIC_INFO_SCRIPT_IS_INIT;
	}

	if (!tracks_valid || !firmware_major_matches_last){
		if (flash_default_tracks() == 0) g_working_magic_info.tracks_init = MAGIC_INFO_TRACKS_IS_INIT;
	}

	sync_magic_info_version();
	if (!config_valid || !channels_valid || !script_valid || !tracks_valid || !firmware_matches_last){
		flash_magic_info();
	}
}

const MagicInfo * get_saved_magic_info(){
	return &g_saved_magic_info;
}

MagicInfo * get_working_magic_info(){
	return &g_working_magic_info;
}

int flash_magic_info(){
	pr_info("flashing magic info...");
	int result = memory_flash_region(&g_saved_magic_info, &g_working_magic_info, sizeof(MagicInfo));
	if (result == 0) pr_info("success\r\n"); else pr_info("failed\r\n");
	return result;
}
