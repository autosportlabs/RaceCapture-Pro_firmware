#include "sdcard.h"
#include "usb_comm.h"
#include "stdio.h"
#include "string.h"

#define MAX_LOG_FILE_INDEX 99999

int InitSDCard(){
	
		return 0;
}

EmbeddedFileSystem g_efs;


int InitEFS(){
	return efs_init( &g_efs, 0 );
}

int UnmountEFS(){
	return fs_umount(&g_efs.myFs);
}

int OpenNextLogFile(EmbeddedFile *f){

	char filename[13];
	
	int i=0;
	for (;i < MAX_LOG_FILE_INDEX;i++){
		sprintf(filename,"rc_%d.log",i);
		int rc = file_fopen( f, &g_efs.myFs , filename , 'r' );
		if ( rc !=0 ) break;
		file_fclose(f);
	}
	if (i >= MAX_LOG_FILE_INDEX) return -2;
	int rc = file_fopen( f, &g_efs.myFs , filename , 'w' ); 
	return rc;
}


void ListRootDir(void)
{
	DirList list;
	char text[300];
	SendString("Card Init...");
	
	int res = InitEFS();
	if ( res != 0 ) {
		sprintf(text,"failed with %i\r\n",res);
		SendString(text);
	}
	else{
		SendString("ok\r\n");
		sprintf(text,"\r\nDirectory of 'root':\r\n");
		SendBytes(text,strlen(text));
		ls_openDir( &list, &(g_efs.myFs) , "/");
		while ( ls_getNext( &list ) == 0 ) {
			list.currentEntry.FileName[LIST_MAXLENFILENAME-1] = '\0';
			sprintf(text,"%s (%li bytes)\n\r",
			list.currentEntry.FileName,
			list.currentEntry.FileSize
			);
			SendBytes(text,strlen(text));
			}
	fs_umount( &g_efs.myFs ) ;
	}
}
