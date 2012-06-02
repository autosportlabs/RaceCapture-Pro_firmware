/*!
 *	\file Comm.h
 *  \brief interface for the Linux comm class.
 *  \author Ake Hedman, eurosource,
 *  \author akhe@...
 *  \date   2002-2005
 *
*/

///////////////////////////////////////////////////////////////////////////////
// Comm.h
//
//
// This file is part is part of CANAL (CAN Abstraction Layer)
// http://can.sourceforge.net)
//
// Copyright (C) 2000-2008 Ake Hedman, eurosource, <akhe@...>
//
// This software is placed into
// the public domain and may be used for any purpose.  However, this
// notice must not be changed or removed and no warranty is either
// expressed or implied by its publication or distribution.
//
// $RCSfile: com_win32.h,v $
// $Date: 2005/08/30 11:00:13 $
// $Author: akhe $
// $Revision: 1.4 $
///////////////////////////////////////////////////////////////////////////////

#ifndef COMM_WIN32_H_
#define COMM_WIN32_H_

#if !defined(AFX_COMCOMM_H__E2F5A961_FF34_437B_91E1_8D7DEA1E94C9__INCLUDED_)
#define AFX_COMCOMM_H__E2F5A961_FF34_437B_91E1_8D7DEA1E94C9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define HANDSHAKE_NONE	 0
#define HANDSHAKE_HARDWARE	1
#define HANDSHAKE_SOFTWARE	2

#ifdef WIN32
#include <windows.h>
#endif

/*!
 *	\class CCommParams
 *  \brief This class hat encapsulate communication parameters
 *	\brief for a serial communication port.
 *  \author Ake Hedman, eurosource, akhe@...
 *  \version 1
 *  \date    2002-2005
 */

class CCommParams
{

public:

        /// The port number to use
        unsigned char m_nPort;

        /// The baudrate to use
        DWORD m_nBaudRate;

        /// # of stopbits to use
        unsigned char m_nDatabits;

        /// Patity to use
        unsigned char m_nParity;

        /// # of stopbits to use
        unsigned char m_nStopbits;

};

/*! \class CComm
 *  \brief A class that handles serial port communication.
 *  \author Ake Hedman, eurosource, akhe@...
 *  \version 1
 *  \date    2002-2005
 */


class CComm
{

public:

        /// Constructor
        CComm();

        /// Destructor
        virtual ~CComm();

        /*!
                Initialise the serial communication port

                \param nComPort Index for communication port (default to COM1).
                \param nBaudRate Baudrate for the selected port (defaults to 9600).
                \param nDatabits Number of databits ( defeaults to 8 ).
                \param nParity Parity (defaults to no parity ).
                \param nStopbits Number of stopbits (defaults to one stopbit).
                \return True on success.
        */
        bool openPort( const char * comPort,
                                                        DWORD nBaudRate = CBR_115200,
                                                        unsigned char nDatabits = 8,
                                                        unsigned char nParity= NOPARITY,
                                                        unsigned char nStopbits=ONESTOPBIT,
                                                        unsigned char nHandshake = HANDSHAKE_NONE );


        const char * getPortName(unsigned char portNumber);

        void closePort( void );

        void drainInput( void );

        char readChar( int* cnt );

        size_t readBuffer( char *p, size_t size, size_t timeout = 500 );

        size_t readLine(char *buf, size_t bufSize, size_t timeout);

        size_t readLine2(char *buf, size_t bufSize, size_t timeout, size_t lines);

        size_t writeBuffer(const char * p, unsigned short cnt );

        size_t writeChar(char b );

        size_t sendCommand(const char *cmd, char *rsp, size_t rspSize, size_t timeout = 500, bool absorbEcho = true);

        size_t sendCommand2(const char *cmd, char *rsp, size_t rspSize, size_t timeout = 500, bool absorbEcho = true);

        bool isOpen( void ) { return  ( ( NULL != getHandle() ) ? true : false ); };

private :
        HANDLE getHandle( void ) { return m_hCommPort; };

        /*!
                Handle to the communication port
        */
        HANDLE m_hCommPort;

};

#endif // !defined(AFX_COMCOMM_H__E2F5A961_FF34_437B_91E1_8D7DEA1E94C9__INCLUDED_)


#endif /* COMM_WIN32_H_ */
