#include <stdio.h>
#include <string.h>
#include "ff.h"

int f_puts (const TCHAR* str, FIL* cp)
{
   return fputs(str, cp->p_File);
}


FRESULT f_open (FIL* fp, const TCHAR* path, BYTE mode)
{
   char 				_mode[3];
   unsigned int 	modeIndex = 0;
   memset(_mode, 0, sizeof(_mode));
   
   if(mode == FA_WRITE)
   {
	   printf("mode write\r\n", mode);
      _mode[0] = 'a';
   }
   else if(mode == (FA_WRITE | FA_CREATE_ALWAYS))
   {
	   printf("mode write create\r\n", mode);
      _mode[0] = 'w';
   }
   else if(mode == (FA_WRITE | FA_CREATE_NEW))
   {
	   fp->p_File = fopen(path, "r");
	   if(NULL != fp->p_File)
	   {
		  //fclose(fp->p_File);
		  printf("Can't create file as it already exists\r\n" );
		  return FR_EXIST;
	   }
	   
	   printf("mode write file must exist\r\n", mode);
      _mode[0] = 'w';
   }
   else
   {
	   printf("mode %d not supported\r\n", mode);
   }
   
   printf("f_open %s mode %c\r\n", path, _mode[0]);
   
   fp->p_File = fopen(path, _mode);
   
   if(NULL == (FILE*)fp->p_File)
   {
	  printf("f_open failed\r\n");
      return FR_DISK_ERR;
   }
   else
   {
	   printf("f_open SUCKSESS\r\n");
      return FR_OK;
   }
}

FRESULT f_close (FIL* fp)
{
   fclose(fp->p_File);
   return 0;
}

FRESULT f_sync (FIL* fp)
{
   fflush(fp->p_File);
   return 0;
}
