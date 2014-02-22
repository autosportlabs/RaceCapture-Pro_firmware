#include "magic.h"
#include "memory.h"
#include "mod_string.h"
#include "printk.h"


const MagicInfo g_saved_magic_info __attribute__ ((aligned (FLASH_MEMORY_PAGE_SIZE))) __attribute__((section(".magic\n\t#")));

static MagicInfo g_working_magic_info;

int is_script_init(){
	return g_working_magic_info.script_init == MAGIC_INFO_SCRIPT_IS_INIT;
}

int is_config_init(){
	return g_working_magic_info.config_init == MAGIC_INFO_CONFIG_IS_INIT;
}

int firmware_version_matches_last(){
	VersionInfo * version = &g_working_magic_info.current_version;
	return version->major == MAJOR_REV && version->minor == MINOR_REV && version->bugfix == BUGFIX_REV;
}

void initialize_magic_info(){
	memcpy(&g_working_magic_info,&g_saved_magic_info,sizeof(MagicInfo));
}

const MagicInfo * get_saved_magic_info(){
	return &g_saved_magic_info;
}

MagicInfo * get_working_magic_info(){
	return &g_working_magic_info;
}

int flash_magic_info(){
	pr_info("flashing magic info...");
	int result = memory_flash_region(&g_saved_magic_info, &g_working_magic_info, sizeof (MagicInfo) + (256 - sizeof(MagicInfo)));
	if (result == 0) pr_info("success\r\n"); else pr_info("failed\r\n");
	return result;
}
