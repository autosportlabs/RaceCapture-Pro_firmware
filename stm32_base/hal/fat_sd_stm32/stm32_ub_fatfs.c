//--------------------------------------------------------------
// File     : stm32_ub_fatfs.c
// Datum    : 01.05.2014
// Version  : 1.4
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F4
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : FATFS (GPIO, MISC, SDIO, DMA)
// Funktion : File-Funktionen per FatFS-Library
//            an MMC-Devices
//
// Hinweis  : Anzahl der Devices : 1
//            Typ vom Device     : MMC
//            (für USB oder MMC+USB gibt es andere LIBs) 
//
//            Doku aller FATFS-Funktionen "doc/00index_e.html"
//
// Speed-MMC: ReadBlock  = 2,38MByte/sec (Puffer=512 Bytes)
//            WriteBlock = 499kByte/sec  (Puffer=512 Bytes)
//--------------------------------------------------------------
// Warning  : The STM32F4 in Revision-A generates a
//            "SDIO_IT_DCRCFAIL" in function "SD_ProcessIRQSrc"
//            To avoid this set the Define "SDIO_STM32F4_REV" to 0
//            (check the STM-Errata-Sheet for more information)
//            [Thanks to Major for posting this BUG...UB]
//--------------------------------------------------------------

//--------------------------------------------------------------
// Pinbelegung : [MMC]
//
// 1bit-Mode :
//
//   PC8  -> SDIO_D0  = SD-Karte DAT0
//   PC12 -> SDIO_CK  = SD-Karte Clock
//   PD2  -> SDIO_CMD = SD-Karte CMD
//
// 4bit-Mode :
//
//   PC8  -> SDIO_D0  = SD-Karte DAT0
//   PC9  -> SDIO_D1  = SD-Karte DAT1
//   PC10 -> SDIO_D2  = SD-Karte DAT2
//   PC11 -> SDIO_D3  = SD-Karte DAT3/CD
//   PC12 -> SDIO_CK  = SD-Karte Clock
//   PD2  -> SDIO_CMD = SD-Karte CMD
//
// mit Detect-Pin :
//
//   PC0  -> SD_Detect-Pin (Hi=ohne SD-Karte)
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_fatfs.h"



//--------------------------------------------------------------
// Globale Variabeln
//--------------------------------------------------------------
static FATFS FileSystemObject;


//--------------------------------------------------------------
// Init-Funktion
// (init aller Systeme)
//--------------------------------------------------------------
void UB_Fatfs_Init(void)
{
  // init der Hardware für die SDCard-Funktionen
  UB_SDCard_Init();
  // init der Hardware für die USB-Funktionen
  UB_USBDisk_Init();
  // init der Hardware für die ATA-Funktionen
  UB_ATADrive_Init();
}

//--------------------------------------------------------------
// Status vom Medium abfragen
// dev : [MMC_0]
// Return Wert :
//     FATFS_OK    => Medium eingelegt
//  FATFS_NO_MEDIA => kein Medium eingelegt
//--------------------------------------------------------------
FATFS_t UB_Fatfs_CheckMedia(MEDIA_t dev)
{
  FATFS_t ret_wert=FATFS_NO_MEDIA;
  uint8_t check=SD_NOT_PRESENT;

  // check ob Medium eingelegt
  if(dev==MMC_0) check=UB_SDCard_CheckMedia();
  if(check==SD_PRESENT) {
    ret_wert=FATFS_OK;
  }
  else {
    ret_wert=FATFS_NO_MEDIA;
  }

  return(ret_wert);
}

//--------------------------------------------------------------
// Media mounten
// dev : [MMC_0]
// Return Wert :
//     FATFS_OK       => kein Fehler
//  FATFS_MOUNT_ERR   => Fehler
//  FATFS_GETFREE_ERR => Fehler
//--------------------------------------------------------------
FATFS_t UB_Fatfs_Mount(MEDIA_t dev)
{
  FATFS_t ret_wert=FATFS_MOUNT_ERR;
  FRESULT check=FR_INVALID_PARAMETER;
  DWORD fre_clust;
  FATFS	*fs;

  if(dev==MMC_0) check=f_mount(0, &FileSystemObject);
  if(check == FR_OK) {
    if(dev==MMC_0) check=f_getfree("0:", &fre_clust, &fs);
    if(check == FR_OK) {
      ret_wert=FATFS_OK;
    }
    else {
      ret_wert=FATFS_GETFREE_ERR;
    }
  }
  else {
    ret_wert=FATFS_MOUNT_ERR;
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// Media unmounten
// dev : [MMC_0]
// Return Wert :
//     FATFS_OK     => kein Fehler
//  FATFS_MOUNT_ERR => Fehler
//--------------------------------------------------------------
FATFS_t UB_Fatfs_UnMount(MEDIA_t dev)
{
  FATFS_t ret_wert=FATFS_MOUNT_ERR;
  FRESULT check=FR_INVALID_PARAMETER;

  if(dev==MMC_0) check=f_mount(0, NULL);
  if(check == FR_OK) {
    ret_wert=FATFS_OK;
  }
  else {
    ret_wert=FATFS_MOUNT_ERR;
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// File loeschen
// File darf nicht geoeffnet sein
// Name ist der komplette Pfad z.B. "0:/Test.txt"
// Return Wert :
//     FATFS_OK      => kein Fehler
//  FATFS_UNLINK_ERR => Fehler
//--------------------------------------------------------------
FATFS_t UB_Fatfs_DelFile(const char* name)
{
  FATFS_t ret_wert=FATFS_UNLINK_ERR;
  FRESULT check=FR_INVALID_PARAMETER;

  check=f_unlink(name);
  if(check==FR_OK) {
    ret_wert=FATFS_OK;
  }
  else {
    ret_wert=FATFS_UNLINK_ERR;
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// File oeffnen (zum lesen oder schreiben)
// File per &-Operator übergeben
// Name ist der komplette Pfad z.B. "0:/Test.txt"
// mode : [F_RD, F_WR, F_WR_NEW, F_WR_CLEAR]
// Return Wert :
//     FATFS_OK    => kein Fehler
//  FATFS_OPEN_ERR => Fehler
//  FATFS_SEEK_ERR => Fehler bei WR und WR_NEW
//--------------------------------------------------------------
FATFS_t UB_Fatfs_OpenFile(FIL* fp, const char* name, FMODE_t mode)
{
  FATFS_t ret_wert=FATFS_OPEN_ERR;
  FRESULT check=FR_INVALID_PARAMETER;

  if(mode==F_RD) check = f_open(fp, name, FA_OPEN_EXISTING | FA_READ); 
  if(mode==F_WR) check = f_open(fp, name, FA_OPEN_EXISTING | FA_WRITE);
  if(mode==F_WR_NEW) check = f_open(fp, name, FA_OPEN_ALWAYS | FA_WRITE);
  if(mode==F_WR_CLEAR) check = f_open(fp, name, FA_CREATE_ALWAYS | FA_WRITE);

  if(check==FR_OK) {
    ret_wert=FATFS_OK;
    if((mode==F_WR) || (mode==F_WR_NEW)) {
      // Pointer ans Ende vom File stellen
      check = f_lseek(fp, f_size(fp));
      if(check!=FR_OK) {
        ret_wert=FATFS_SEEK_ERR;
      }
    }
  }
  else {
    ret_wert=FATFS_OPEN_ERR;
  }   

  return(ret_wert);
}

//--------------------------------------------------------------
// File schliessen
// File per &-Operator übergeben
// Return Wert :
//     FATFS_OK     => kein Fehler
//  FATFS_CLOSE_ERR => Fehler
//--------------------------------------------------------------
FATFS_t UB_Fatfs_CloseFile(FIL* fp)
{
  FATFS_t ret_wert=FATFS_CLOSE_ERR;
  FRESULT check=FR_INVALID_PARAMETER;

  check=f_close(fp);

  if(check==FR_OK) {
    ret_wert=FATFS_OK;
  }
  else {
    ret_wert=FATFS_CLOSE_ERR;
  }   

  return(ret_wert);
}


//--------------------------------------------------------------
// String in File schreiben
// File muss offen sein
// File per &-Operator übergeben
// Zeilenendekennung ('\n') wird automatisch angehängt
// Return Wert :
//     FATFS_OK    => kein Fehler
//  FATFS_PUTS_ERR => Fehler
//--------------------------------------------------------------
FATFS_t UB_Fatfs_WriteString(FIL* fp, const char* text)
{
  FATFS_t ret_wert=FATFS_PUTS_ERR;
  int check=0;

  check=f_puts(text, fp);

  if(check>=0) {
    ret_wert=FATFS_OK;
    // Zeilenendekennung hinzufügen
    f_putc('\n', fp);
  }
  else {
    ret_wert=FATFS_PUTS_ERR;
  }   

  return(ret_wert);
}


//--------------------------------------------------------------
// String aus einem File lesen
// File muss offen sein
// File per &-Operator übergeben
// text : String
// len  : Grösse des String-Puffers
//        es werden (len) Zeichen ausgelesen
//        oder bis Fileende bzw. Stringende erreicht ist
// Return Wert :
//     FATFS_OK        => kein Fehler
//    FATFS_EOF        => Fileende erreicht
// FATFS_RD_STRING_ERR => Fehler
//--------------------------------------------------------------
FATFS_t UB_Fatfs_ReadString(FIL* fp, char* text, uint32_t len)
{
  FATFS_t ret_wert=FATFS_RD_STRING_ERR;
  int check;

  f_gets(text, len, fp);
  check=f_eof(fp);
  if(check!=0) return(FATFS_EOF);
  check=f_error(fp);
  if(check!=0) return(FATFS_RD_STRING_ERR);
  ret_wert=FATFS_OK;

  return(ret_wert);
}


//--------------------------------------------------------------
// Filegröße auslesen
// File muss offen sein
// File per &-Operator übergeben
// Return Wert :
//   >0 => Filegröße in Bytes
//   0  => Fehler
//--------------------------------------------------------------
uint32_t UB_Fatfs_FileSize(FIL* fp)
{
  uint32_t ret_wert=0;
  int filesize;

  filesize=f_size(fp);
  if(filesize>=0) ret_wert=(uint32_t)(filesize);

  return(ret_wert);
}


//--------------------------------------------------------------
// Datenblock aus einem File lesen
// File muss offen sein
// File per &-Operator übergeben
// buf  : Puffer für die Daten
// len  : Grösse des Daten-Puffers (max 512 Bytes)
//        es werden (len) Zeichen ausgelesen
//        oder bis Fileende erreicht ist
// read : Anzahl der Zeichen die ausgelesen wurden (bei err=>0)
// Return Wert :
//     FATFS_OK        => kein Fehler
//    FATFS_EOF        => Fileende erreicht
//  FATFS_RD_BLOCK_ERR => Fehler
//--------------------------------------------------------------
FATFS_t UB_Fatfs_ReadBlock(FIL* fp, unsigned char* buf, uint32_t len, uint32_t* read)
{
  FATFS_t ret_wert=FATFS_RD_BLOCK_ERR;
  FRESULT check=FR_INVALID_PARAMETER;
  UINT ulen,uread;

  ulen=(UINT)(len);
  if(ulen>_MAX_SS) {
    ret_wert=FATFS_RD_BLOCK_ERR;
    *read=0;
  }
  else {
    check=f_read(fp, buf, ulen, &uread);
    if(check==FR_OK) {
      *read=(uint32_t)(uread);
      if(ulen==uread) {
        ret_wert=FATFS_OK;
      }
      else {
        ret_wert=FATFS_EOF;
      }
    }
    else {
      ret_wert=FATFS_RD_BLOCK_ERR;
      *read=0;
    }
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// Datenblock in ein File schreiben
// File muss offen sein
// File per &-Operator übergeben
// buf  : Daten (in einem Puffer)
// len  : Grösse des Daten-Puffers (max 512 Bytes)
//        es werden (len) Zeichen geschrieben
// write : Anzahl der Zeichen die geschrieben wurden (bei err=>0)
// Return Wert :
//     FATFS_OK        => kein Fehler
//    FATFS_DISK_FULL  => kein Speicherplatz mehr
//  FATFS_WR_BLOCK_ERR => Fehler
//--------------------------------------------------------------
FATFS_t UB_Fatfs_WriteBlock(FIL* fp, unsigned char* buf, uint32_t len, uint32_t* write)
{
  FATFS_t ret_wert=FATFS_WR_BLOCK_ERR;
  FRESULT check=FR_INVALID_PARAMETER;
  UINT ulen,uwrite;

  ulen=(UINT)(len);
  if(ulen>_MAX_SS) {
    ret_wert=FATFS_WR_BLOCK_ERR;
    *write=0;
  }
  else {
    check=f_write(fp, buf, ulen, &uwrite);
    if(check==FR_OK) {
      *write=(uint32_t)(uwrite);
      if(ulen==uwrite) {
        ret_wert=FATFS_OK;
      }
      else {
        ret_wert=FATFS_DISK_FULL;
      }
    }
    else {
      ret_wert=FATFS_WR_BLOCK_ERR;
      *write=0;
    }
  }

  return(ret_wert);
}

