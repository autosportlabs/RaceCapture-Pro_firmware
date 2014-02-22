/*
 * magic.h
 *
 *  Created on: Feb 21, 2014
 *      Author: brent
 */

#ifndef MAGIC_H_
#define MAGIC_H_

#define SCRIPT_IS_INIT 0xDECAFBAD
#define CONFIG_IS_INIT 0xDECAFBAD

typedef struct _VersionInfo{
	unsigned int major;
	unsigned int minor;
	unsigned int bugfix;
} VersionInfo;

typedef struct _MagicInfo {
	VersionInfo current_version;
	unsigned int script_init;
	unsigned int config_init;
} MagicInfo;


int is_script_init();

int is_config_init();

int firmware_version_matches_last();

void initialize_magic_info();

MagicInfo * get_saved_magic_info();

MagicInfo * get_working_magic_info();

int flash_magic_info();

#endif /* MAGIC_H_ */
