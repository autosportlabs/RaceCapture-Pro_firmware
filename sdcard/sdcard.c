#include "sdcard.h"
#include "usb_comm.h"
#include "string.h"
#include "modp_numtoa.h"


#define MAX_LOG_FILE_INDEX 99999

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
		strcpy(filename,"rc_");
		char numBuf[12];
		modp_itoa10(i,numBuf);
		strcat(filename,numBuf);
		strcat(filename,".log");
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
	SendString("Card Init...");
	
	int res = InitEFS();
	if ( res != 0 ) {
		SendString("failed with ");
		SendInt(res);
		SendCrlf();
	}
	else{
		SendString("ok\r\n");
		SendString("Directory of 'root':\r\n");
		ls_openDir( &list, &(g_efs.myFs) , "/");
		while ( ls_getNext( &list ) == 0 ) {
			list.currentEntry.FileName[LIST_MAXLENFILENAME-1] = '\0';
			SendString(list.currentEntry.FileName);
			SendString(" (");
			SendInt(list.currentEntry.FileSize);
			SendString(")");
			SendCrlf();
			}
	fs_umount( &g_efs.myFs ) ;
	}
}
