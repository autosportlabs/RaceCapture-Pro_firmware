///////////////////////////////////////////////////////////////////////////////
// COMComm.cpp:
//
// Copyright (C) 2000-2008 Ake Hedman, eurosource, <akhe@...>
//
// This software is placed into
// the public domain and may be used for any purpose.  However, this
// notice must not be changed or removed and no warranty is either
// expressed or implied by its publication or distribution.
//
// $RCSfile: com_win32.cpp,v $
// $Date: 2005/08/30 11:00:13 $
// $Author: akhe $
// $Revision: 1.4 $
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "stdio.h"
#include "comm_win32.h"
#include <wx/log.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// Constructor
//

CComm::CComm()
{
        m_hCommPort = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// Destructor
//

CComm::~CComm()
{
        if ( NULL != m_hCommPort ) {
                CloseHandle( m_hCommPort );
        }
}


/////////////////////////////////////////////////////////////////////////////
// init
//

bool CComm::openPort( unsigned char nComPort,
                                                DWORD nBaudRate,
                                                unsigned char nDatabits,
                                                unsigned char nParity,
                                                unsigned char nStopbits,
                                                unsigned char nHandshake )
{
        char szComPort[10];
        COMMTIMEOUTS ct;
        DCB dcbCommPort;	// ONESTOPBIT

        // Check com-port data
        if ( nComPort!= 0 ) {
                sprintf( szComPort, "\\\\.\\COM%d", nComPort );
        }
        else {
                return false;
        }

        if ( INVALID_HANDLE_VALUE ==
                        ( m_hCommPort = CreateFile( szComPort,
                                                                                GENERIC_READ | GENERIC_WRITE,
                                                                                0,
                                                                                NULL,
                                                                                OPEN_EXISTING,
                                                                                FILE_ATTRIBUTE_NORMAL,
                                                                                NULL ) ) ) {
                return false;
        }

        memset( &ct, 0, sizeof( ct ) );

        // Set timeouts
        ct.ReadIntervalTimeout = MAXDWORD;	 // AKHE Changed fron 200
        ct.ReadTotalTimeoutMultiplier = MAXDWORD;	// AKHE Changed from 0
        ct.ReadTotalTimeoutConstant = 200;
        ct.WriteTotalTimeoutMultiplier = 0;
        ct.WriteTotalTimeoutConstant = 200;
        SetCommTimeouts( m_hCommPort, &ct );

        // Comm state
        if ( !GetCommState( m_hCommPort, &dcbCommPort ) ) {
                CloseHandle( m_hCommPort );
                return false;
        }

        dcbCommPort.DCBlength = sizeof( DCB );
        dcbCommPort.BaudRate = nBaudRate;
        if ( nParity != NOPARITY ) {
                dcbCommPort.fParity = true;
        }
        else {
                dcbCommPort.fParity = false;
        }

        dcbCommPort.Parity = nParity;
        dcbCommPort.fDtrControl = DTR_CONTROL_ENABLE;
        dcbCommPort.fDtrControl = DTR_CONTROL_DISABLE;
        //dcbCommPort.fDsrSensitivity = DTR_CONTROL_DISABLE;
        dcbCommPort.ByteSize = nDatabits;
        dcbCommPort.StopBits = nStopbits;

        if (  HANDSHAKE_HARDWARE == nHandshake ) {

                // Software flow control
                dcbCommPort.fOutX = FALSE;
                dcbCommPort.fInX = FALSE;

                // Hardware flow control
                dcbCommPort.fOutxCtsFlow = TRUE;
                dcbCommPort.fOutxDsrFlow = TRUE;

        }
        else if (  HANDSHAKE_SOFTWARE == nHandshake ) {

                // Software flow control
                dcbCommPort.fOutX = TRUE;
                dcbCommPort.fInX = TRUE;

                // Hardware flow control
                dcbCommPort.fOutxCtsFlow = FALSE;
                dcbCommPort.fOutxDsrFlow = FALSE;

        }
        else { //  HANDSHAKE_NONE

                // Software flow control
                dcbCommPort.fOutX = FALSE;
                dcbCommPort.fInX = FALSE;

                // Hardware flow control
                dcbCommPort.fOutxCtsFlow = FALSE;
                dcbCommPort.fOutxDsrFlow = FALSE;
        }

        if ( !SetCommState( m_hCommPort, &dcbCommPort ) )  {
                CloseHandle( m_hCommPort );
                return false;
        }

        //COMMPROP prop;
        //GetCommProperties( m_hCommPort, &prop );

        return true;
}

/////////////////////////////////////////////////////////////////////////////
// close
//

void CComm::closePort()
{
        CloseHandle( m_hCommPort );
        m_hCommPort = NULL;
}


/////////////////////////////////////////////////////////////////////////////
// drainInput
//


void CComm::drainInput()
{
        int cnt;
        unsigned char c;

        do {
        	c = readChar( &cnt );
        } while ( cnt );
}

/////////////////////////////////////////////////////////////////////////////
// readChar
//

char CComm::readChar( int* cnt )
{
        char c = 0;
        DWORD dwCount;

        int result = 0;

        result = ReadFile( m_hCommPort, &c, 1, &dwCount, NULL );
        if (result == 0) {
                int err = GetLastError();
                //throw exception
        }

        *cnt = dwCount;
        return c;
}

/////////////////////////////////////////////////////////////////////////////
// readBuffer
//

size_t CComm::readBuffer( char *pBuf, size_t size, size_t timeout )
{
        char *p = pBuf;
        size_t tstart;
        int cnt;
        int zeroRead = 0;
        unsigned short pos = 0;

        memset( pBuf, 0, size );
        tstart = GetTickCount();
        while ( ( GetTickCount() - tstart ) < timeout ) {
                p[ pos ]  = readChar ( &cnt );
                if ( cnt ) {
                        pos++;
                        if ( pos > size ) break;
                }
                else{
                	zeroRead++;
                }
        }
        return pos;
}

/////////////////////////////////////////////////////////////////////////////
// write
//

size_t CComm::writeBuffer(const char * p, unsigned short cnt )
{
        DWORD count;
        bool result = WriteFile( m_hCommPort, p, cnt, &count, NULL );
        if (!result){
        	//handle error - throw, etc
        }
        return count;
}


/////////////////////////////////////////////////////////////////////////////
// writeChar
//

size_t CComm::writeChar(char b )
{
        DWORD count;
        WriteFile( m_hCommPort, &b, 1, &count, NULL );
        return count;
}


/////////////////////////////////////////////////////////////////////////////
// readLine
//
size_t CComm::readLine(char *buf, size_t bufSize, size_t timeout){

	memset( buf, 0, bufSize );
    size_t tstart = GetTickCount();
    size_t totalRead = 0;
	DWORD charsRead = 0;
    while ( ( GetTickCount() - tstart ) < timeout && totalRead < bufSize ) {

    	bool result = ReadFile(m_hCommPort, (buf + totalRead), 1, &charsRead, NULL);
    	if (!result){
			int err = GetLastError();
			//throw an exception
    	}
    	if ('\r' == *(buf + totalRead)) break;
    	totalRead += charsRead;
    }
    return totalRead;
}

/////////////////////////////////////////////////////////////////////////////
// sendCommand
//

size_t CComm::sendCommand(const char *cmd, char *rsp, size_t rspSize, size_t timeout, bool absorbEcho){
	//flush input buffer
	drainInput();

	//send the command
	size_t cmdLen = strlen(cmd);
	size_t written = 0;
	while (written < cmdLen){
		written += writeBuffer((cmd + written),cmdLen - written);
	}
	while(!writeChar('\r'));

	//optionally absorb the command echo
	if (absorbEcho){
		readLine(rsp,rspSize,timeout);
		if (strncmp(rsp,cmd,cmdLen) != 0){
			//throw an error if command doesn't match?
		}
	}

	//read the response
	return readLine(rsp,rspSize,timeout);
}

/////////////////////////////////////////////////////////////////////////////
// alternate readLine
//


size_t CComm::readLine2(char *buf, size_t bufSize, size_t timeout, size_t lines){

	memset( buf, 0, bufSize );
    size_t tstart = GetTickCount();
    size_t totalRead = 0;
	DWORD charsRead = 0;
	size_t linesRead = 0;
    while ( ( GetTickCount() - tstart ) < timeout && totalRead < bufSize && linesRead < lines) {
    	bool result = ReadFile(m_hCommPort, (buf + totalRead), (DWORD)(bufSize - totalRead), &charsRead, NULL);
    	if (!result){
			int err = GetLastError();
			//throw an exception
    	}
    	for (size_t i = totalRead; i < totalRead + charsRead; i++ ){
    		char * curPos = buf + i;
    		if ('\r' == *curPos){
    			linesRead++;
    		}
    	}
    	totalRead+=charsRead;
    }
    return totalRead;
}

/////////////////////////////////////////////////////////////////////////////
// alternate sendCommand
//

size_t CComm::sendCommand2(const char *cmd, char *rsp, size_t rspSize, size_t timeout, bool absorbEcho)
{
	//flush input buffer
	drainInput();

	//send the command
	size_t cmdLen = strlen(cmd);
	size_t written = 0;
	while (written < cmdLen){
		written += writeBuffer((cmd + written),cmdLen - written);
	}
	while(!writeChar('\r'));

	//read the response
	readLine2(rsp,rspSize,timeout,1 + absorbEcho);
	if (absorbEcho){
		//find the line following the command echo, this will be the response
		for (char *pos = rsp; *pos !='\0';pos++){
			if ('\r' == *pos){
				strcpy(rsp,pos + 1);
				break;
			}
		}
	}
	return strlen(rsp);
}



