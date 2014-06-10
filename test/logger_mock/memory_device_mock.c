#include "memory_device.h"
#include "memory_mock.h"
#include "mod_string.h"
#include <stdio.h>

static int g_isFlashed = 0;

int memory_device_flash_region(const void *vAddress, const void *vData, unsigned int length){
	g_isFlashed = 1;
	void * addr = (void *)vAddress;
	memcpy(addr, vData, length);
	printf("\r\nflash: %d %d |%s|\r\n", length, strlen((const char *)vData), (const char*)vData);
	return 0;
}

void memory_mock_set_is_flashed(int isFlashed){
	g_isFlashed = isFlashed;
}

int memory_mock_get_is_flashed(){
	return g_isFlashed;
}

