#include "memory_device.h"
#include "memory_mock.h"

static int g_isFlashed = 0;

int memory_device_flash_region(void *vAddress, void *vData, unsigned int length){
	g_isFlashed = 1;
	return 1;
}

void memory_mock_set_is_flashed(int isFlashed){
	g_isFlashed = isFlashed;
}

int memory_mock_get_is_flashed(){
	return g_isFlashed;
}

