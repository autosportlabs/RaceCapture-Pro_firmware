

/*!
 *	\file com.h
 *  \brief interface for the Linux comm class.
 *  \author Ake Hedman, CC Systems AB,
 *  \author akhe@...
 *  \date    2002-2003
 *
// Copyright (C) 2000-2008 Ake Hedman, eurosource, <akhe@...>
//
// This software is placed into
// the public domain and may be used for any purpose.  However, this
// notice must not be changed or removed and no warranty is either
// expressed or implied by its publication or distribution.
 *
 * This class wraps and extends Minicom.
 * see ftp://ftp.sunsite.unc.edu/pub/Linux/apps/serialcomm/dialout
 *
 * Some changes by akhe@... ( www.brattberg.com ) done for the
 * M.U.M.I.N. protocol.
 *
 * - Sorry no more sysdep. Works only on Linux systems ( maybe on others ).
 *

        */

#ifndef COMM_LINUX_H_
#define COMM_LINUX_H_

#ifndef WIN32
#include <termios.h>
#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>

/*! \class CComm
 *  \brief A class that handles serial port communication.
 *  \author �ke Hedman,
 *  \version 1
 *  \date    2002-2003
 */

class Comm
{

public:

  // Constructors

        /// Constructor
  Comm( void );

        /*!
                Constructor
                \param Device Open the named device
        */
  Comm(char *Device) { m_fd = 0; open( Device ); }

        /*!
                Constructor
                \param fd Open a device from a descriptor
        */
        Comm(int fd) { m_fd = fd; }

  /// Destructor
  virtual ~Comm( void );

  /*!
                Open the communication device

                \parm szDevice Name of communication device to open.
                \return True on success.
        */
  virtual bool open( char* szDevice = NULL );

  /*!
                Close the communication device
        */
  void close( void ) { ::close( m_fd ); m_fd = 0; }

  /*!
                Get file descriptor
        */
  int getFD( void ) { return m_fd; }

  /*!
                Set port parameters

                \param baud Pointer to baudrate string.
                \param parity Parity (N for none, E for even, O for odd, M for mark, S for space).
                \param bits Number of bits.
                \param HWFlow	Set to zero to disable hardware flow control.
                \param SWFlow Set to zero to disable software flow control.
        */
  void setParam( char *baud,
                                                                        char *parity,
                                                                        char *bits,
                                                                        int HWFlow=0,
                                                                        int SWFlow=0 );

  /*!
                Set hardware flow control

                \param on set to non-zeroe to enable hardware handshake.
        */
  void setHWFlow( int on = true );

  /*!
                Get maximum baud rate

                \return Return the maximum supported baudrate.
        */
  int getMaxBaud( void );

  /*!
                Is there a character waiting in the inque

                \return No-zero if there is a character to read from the communication channel.
        */
  int isCharReady( void );

  /*!
                Turn on DTR
        */
  void DtrOn( void );

  /*!
                Turn off DTR
        */
  void DtrOff( void );

  /*!
                Turn on RTS
        */
  void RtsOn( void );

  /*!
                Turn off RTS
        */
  void RtsOff( void );

  /*!
                Turn of DTR for a specific period.

                \param delay Time that DTR should be off.
        */
  void ToggleDTR( int delay );

  /*!
                Get Data Carrier Detect status
        */
  int getDCD( void );

  /*!
                Send break
        */
  void SendBreak( void );

  /*!
                Flush the send que
        */
  void Flush( void );

  /*!
                Drain the send que
        */
  void Drain( void );

  /*!
                Flush inQue
        */
  void FlushInQue( void );

  /*!
                Get charcter

                \return Get character from communication channel.
        */
  unsigned char comm_getc( void );

  /*!
                Send a character

                \param c Character to send on communication channel.
                \param bDrain Set to TRUE if we should wait for the character to be sent.
        */
  void comm_putc( unsigned char c, bool bDrain = false  );

  /*!
                Get a string of characters

                \param Buffer Buffer for received data.
                \param max Maximum number of characters for the buffer.
                \return Non zeror if OK, zeror on failure.
        */
  int comm_gets( char *Buffer, int max );

  /*!
                Get a string of characters with timeout

                \param Buffer for received data.
                \param max Max number of characters the buffer can hold.
                \param timeout Time in us we should wait.
                \return Non zero if OK, zeror on failure.
        */
  int comm_gets( char *Buffer, int nChars, long timeout );

  /*!
                Send a string of characters

                \param Buffer Data to send.
                \param bDrain Set to TRUE if we should wait for the character to be sent.
                \return Non zero if OK, zeror on failure.
        */
  int comm_puts( char *Buffer, bool bDrain=false );

  /*!
     Send an array of characters

                \param Buffer Buffer for data to send.
                \paran len Number of bytes to send.
                \param bDrain Set to TRUE if we should wait for the character to be sent.
                \return Non zero if OK, zeror on failure.
  */
  int comm_puts( char *Buffer, int len, bool bDrain=false  );

  /*!
                Get Line Status Register Transmitter Empty bit.

                \return Non zero if UART transmitter is empty.
        */
  int isTransmitterEmpty( void );

  /*!
                Get the current millisecond tick count.

                \return Current CPU tick count.
        */
  long msGettick( void );

  /*!
                Delay for at least 'len' ms

                \param len Number of milliseconds to delay.
        */
  void msDelay( int len );



        // Windows compatible methods

        /*!
                Write a character out on the communication channel
                \param b Character to write.
        */
  void writeChar( unsigned char b ) { comm_putc( b ); };


        /*!
                Read a character form the communication channel
                \param cnt 0 if no character to read, 1 if character read.
                \return Character read .
        */
        char readChar( int *cnt );


        /*!
                Drain the input queue
        */
  void drainInput( void );

 protected:

        /*!
                Set the error string (Internal)

                \param p Pointer to error string to set
        */
  void setErrorStr( char *p ) { strncpy( m_szerr, p, 256 ); };

 public:

  /// mutex for exclusive use of the Com object
  pthread_mutex_t m_mutex;

 protected:

  /// Serial device name
  char m_szDevice[64];

  /// Serial device handle
  int m_fd;

  /// Holder for error string
  char m_szerr[256];
};

#endif
#endif /* COMM_LINUX_H_ */
