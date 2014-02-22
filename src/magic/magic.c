#include "magic.h"
#include "memory.h"
#include "mod_string.h"

MagicInfo g_saved_magic_info __attribute__ ((aligned (FLASH_MEMORY_PAGE_SIZE))) __attribute__((section(".magic\n\t#")));

static MagicInfo g_working_magic_info;
int is_script_init(){
	return g_working_magic_info.script_init == SCRIPT_IS_INIT;
}

int is_config_init(){
	return g_working_magic_info.config_init == CONFIG_IS_INIT;
}

int firmware_version_matches_last(){
	VersionInfo * version = &g_working_magic_info.current_version;
	return version->major == MAJOR_REV && version->minor == MINOR_REV && version->bugfix == BUGFIX_REV;
}

void initialize_magic_info(){
	memcpy(&g_working_magic_info,&g_saved_magic_info,sizeof(MagicInfo));
}

MagicInfo * get_saved_magic_info(){
	return &g_saved_magic_info;
}

MagicInfo * get_working_magic_info(){
	return &g_working_magic_info;
}

int flash_magic_info(){
	void *saved_magic_info = get_saved_magic_info();
	void *working_magic_info = get_working_magic_info();
	return memory_flash_region(saved_magic_info, working_magic_info, sizeof (MagicInfo));
}
