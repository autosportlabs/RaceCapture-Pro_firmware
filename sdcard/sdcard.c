#include "sdcard.h"
#include "usb_comm.h"
#include "stdio.h"
#include "string.h"

int InitSDCard(){
	
		return 0;
}

void ListRootDir(void)
{
	EmbeddedFileSystem efs;		
	DirList list;
	char text[300];
	int res;
	
	SendString("Card Init...");
	
	if ( ( res = efs_init( &efs, 0 ) ) != 0 ) {
		sprintf(text,"failed with %i\r\n",res);
		SendString(text);
	}
	else{
		SendString("ok\r\n");
		sprintf(text,"\r\nDirectory of 'root':\r\n");
		SendBytes(text,strlen(text));
		ls_openDir( &list, &(efs.myFs) , "/");
		while ( ls_getNext( &list ) == 0 ) {
			list.currentEntry.FileName[LIST_MAXLENFILENAME-1] = '\0';
			sprintf(text,"%s (%li bytes)\n\r",
			list.currentEntry.FileName,
			list.currentEntry.FileSize
			);
			SendBytes(text,strlen(text));
			}
	fs_umount( &efs.myFs ) ;
	}
}
