#include <stdio.h>
#include <string.h>
#include "printk.h"
#include "memory.h"
#include "memory_device.h"

char MEMORY_MOCK_CONFIG_FILE_NAME[256] = "config.txt";

void memory_mock_set_config_file(char * p_File)
{
	strncpy(MEMORY_MOCK_CONFIG_FILE_NAME, p_File, sizeof(MEMORY_MOCK_CONFIG_FILE_NAME) - 1);
}

int memory_flash_region(const void *address, const void *data, unsigned int length){
	FILE * p_File = NULL;
	
	p_File = fopen(MEMORY_MOCK_CONFIG_FILE_NAME, "wb");
	
	if( (NULL != p_File) &&
		(length == fwrite(data , sizeof(char), length, p_File))
		) {
			fclose(p_File);
			return 0;
		}
	else
		return -1;
}

void memory_read_region(const void *address, void * const data, unsigned int length){
	FILE * p_File = NULL;
	
	p_File = fopen(MEMORY_MOCK_CONFIG_FILE_NAME, "rb");
	
	if( (NULL != p_File) &&
		(length == fread(data , sizeof(char), length, p_File))
		) {
		pr_info("Reading configuration succeeded\r\n");
		fclose(p_File);
	}
	else
		pr_error("Reading configuration failed\r\n");
}
