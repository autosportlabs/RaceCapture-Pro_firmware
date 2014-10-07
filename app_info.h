#ifndef _APP_INFO_H_
#define _APP_INFO_H_

#include <stdint.h>

#define HANDSHAKE_ADDR 0x20000000
#define LOADER_KEY 0xBE57BEEF
#define INTERFACE_USB 0
#define INTERFACE_SERIAL 1

struct app_info_block {
	uint32_t magic_number;
	uint32_t start_addr;
	uint32_t app_len;
	uint32_t app_crc;
	uint32_t info_crc;
};

struct app_handshake_block {
	uint32_t loader_magic;
	uint32_t interface_type;
};

#define APP_INFO_MAGIC_NUMBER  (0xDEADFA11)
#define ALWAYS_KEEP(v) __asm__ __volatile__("" :: "m" (v))
#endif
