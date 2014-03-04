/*
 * magic.h
 *
 *  Created on: Feb 21, 2014
 *      Author: brent
 */

#ifndef MAGIC_H_
#define MAGIC_H_

#define MAGIC_INFO_SCRIPT_IS_INIT 0xDECAFBA1
#define MAGIC_INFO_CONFIG_IS_INIT 0xDECAFBA2
#define MAGIC_INFO_CHANNELS_IS_INIT 0xDECAFBA3
#define MAGIC_INFO_TRACKS_IS_INIT 0xDECAFBA4

typedef struct _VersionInfo{
	unsigned int major;
	unsigned int minor;
	unsigned int bugfix;
} VersionInfo;

typedef struct _MagicInfo {
	VersionInfo current_version;
	unsigned int script_init;
	unsigned int config_init;
	unsigned int channels_init;
	unsigned int tracks_init;
} MagicInfo;


int is_script_init();

int is_config_init();

int is_channels_init();

int is_tracks_init();

void initialize_magic_info();

const MagicInfo * get_saved_magic_info();

MagicInfo * get_working_magic_info();

int flash_magic_info();

#endif /* MAGIC_H_ */
