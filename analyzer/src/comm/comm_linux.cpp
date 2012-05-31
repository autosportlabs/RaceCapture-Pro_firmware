/*!
 *	\file com.cpp
 *  \brief implementation file for the Linux comm class.
 *  \author Ake Hedman, CC Systems AB,
 *  \author akhe@...
 *  \date    2002-2003
 *
 *
// Copyright (C) 2000-2008 Ake Hedman, eurosource, <akhe@...>
//
// This software is placed into
// the public domain and may be used for any purpose.  However, this
// notice must not be changed or removed and no warranty is either
// expressed or implied by its publication or distribution.
 * HISTORY
 * =======
 * 020629
 * Handling error returs from readChar in drainInput.
 *
 */

#define _POSIX
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "comm_linux.h"

#ifndef WIN32

///////////////////////////////////////////////////////////////////////////////
// Constructor

Comm::Comm( void )
{
  pthread_mutex_init( &m_mutex, NULL );
  m_fd = 0;
}

///////////////////////////////////////////////////////////////////////////////
// Destructor

Comm::~Comm( void )
{
  close();
  pthread_mutex_destroy( &m_mutex );
}


///////////////////////////////////////////////////////////////////////////////
// open

bool Comm::open( char *szDevice )
{
  // Not allowed to open if already open
  if ( m_fd != 0 ) return false;

  if ( NULL != szDevice ) {
    if ( -1 == ( m_fd = ::open( szDevice, O_RDWR | O_NONBLOCK ) ) ) {
      return false;
    }
  }
  else {
    if ( -1 == ( m_fd = ::open( m_szDevice, O_RDWR | O_NONBLOCK ) ) ) {
      return false;
    }
  }
  flock( m_fd, LOCK_EX );
  return true;
}


///////////////////////////////////////////////////////////////////////////////
// comm_gets
//
// Get string (max is maximum chars to read) from COM port.
//

int Comm::comm_gets( char *Buffer, int max )
{
  unsigned char c;
  int x=0;
  Buffer = NULL;
  do {
    read( m_fd, &c, 1 );
    Buffer[ x++ ] = c;
  } while( isCharReady() && ( x<max ) );
  Buffer[x-1]=0;
  return x;
}

///////////////////////////////////////////////////////////////////////////////
// comm_gets
//
// Get a string of characters with timeout
//

int Comm::comm_gets( char *Buffer, int nChars, long timeout )
{
  fd_set filedescr;
  struct timeval tval;
  int cnt;
  int us_to = 0;
  int s_to = 0;

  if ( timeout > 1000000 ) {
    s_to = ( timeout/1000000 );

    if ( s_to > 60 ) s_to = 60; // max one minute
    us_to = timeout - ( long )s_to*1000000L;
  }
  else {
    us_to = timeout;
  }

  // loop to wait until each byte is available and read it
  for ( cnt = 0; cnt < nChars; cnt++ ) {
    // set a descriptor to wait for a character available
    FD_ZERO( &filedescr );
    FD_SET( m_fd, &filedescr );

    // set timeout
    tval.tv_sec = s_to;
    tval.tv_usec = us_to;

    // if byte available read or return bytes read
    if ( select( m_fd+1, &filedescr, NULL, NULL, &tval ) != 0 ) {
      if ( read( m_fd, &Buffer[cnt], 1 ) != 1 ) {
        return cnt;
      }
    }
    else {
      return cnt;
    }
  }

  // success, so return desired length
  return nChars;
}

///////////////////////////////////////////////////////////////////////////////
// comm_puts
//
// Put characters on COM port.
//

int Comm::comm_puts(char *Buffer, bool bDrain )
{
  int rv = write( m_fd, Buffer, strlen( Buffer ) );

  if ( bDrain ) {
    Drain();
  }

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// comm_puts
//
// Send an array of characters

int Comm::comm_puts( char *Buffer, int len, bool bDrain )
{
  int rv = write( m_fd, Buffer, len );

  if ( bDrain ) {
    Drain();
  }

  return rv;
}


///////////////////////////////////////////////////////////////////////////////
// comm_getc
//
// Get Character from COM port.
//

unsigned char Comm::comm_getc(void)
{
  unsigned char c = 0;
  read( m_fd, &c, 1 );
  return c;
}

///////////////////////////////////////////////////////////////////////////////
// readChar
//
// Get Character from COM port.
//

char Comm::readChar( int *cnt )
{
  unsigned char c = 0;

  *cnt = read( m_fd, &c, 1 );
  return c;
}

///////////////////////////////////////////////////////////////////////////////
// comm_putc
//
// Put Character on COM port.
//

void Comm::comm_putc(unsigned char c, bool bDrain )
{
  write( m_fd, &c, 1 );
  //printf("Char=%2X\n", c );

  if ( bDrain ) {
    Drain();
  }
}

///////////////////////////////////////////////////////////////////////////////
// setHWFlow
//
// Set hardware flow control.
//

void Comm::setHWFlow( int on )
{
  struct termios tty;

  tcgetattr(m_fd, &tty);
  if (on) {
    tty.c_cflag |= CRTSCTS;
  }
  else {
    tty.c_cflag &= ~CRTSCTS;
  }

  tcsetattr(m_fd, TCSANOW, &tty);
}

///////////////////////////////////////////////////////////////////////////////
// ToggleDTR
//
// Toggle DTR.
//

void Comm::ToggleDTR( int delay )
{
  int f;
  f = TIOCM_DTR;
  ioctl( m_fd, TIOCMBIC, &f );
  if ( 0 < delay ) {
    sleep( delay );
  }
  ioctl( m_fd, TIOCMBIS, &f );
}

///////////////////////////////////////////////////////////////////////////////
// SendBreak
//
// Send break.
//

void Comm::SendBreak( void )
{
  tcsendbreak(m_fd, 0);
}


///////////////////////////////////////////////////////////////////////////////
// getDCD
//
// Get carrier status.
//

int Comm::getDCD( void )

{
  int mcs;

  ioctl(m_fd, TIOCMGET, &mcs);
  return(mcs & TIOCM_CAR ? 1 : 0);
}

///////////////////////////////////////////////////////////////////////////////
// Flush
//
// Flush output que.
//

void Comm::Flush( void )
{
  ioctl( m_fd, TCFLSH, 2);
}

///////////////////////////////////////////////////////////////////////////////
// Drain
//
// Drain output que.
//

void Comm::Drain( void )
{
  tcdrain( m_fd );
}

///////////////////////////////////////////////////////////////////////////////
// FlushInQue
//

void Comm::FlushInQue( void )
{
  while( isCharReady() ) {
    comm_getc();
  }
}

///////////////////////////////////////////////////////////////////////////////
// isCharReady
//
// Check if there are available characters.
//

int Comm::isCharReady( void )
{
  long i = -1;

  (void) ioctl( m_fd, FIONREAD, &i );
  return( (int)i );
}

///////////////////////////////////////////////////////////////////////////////
// getMaxBaud
//
// Get maximum baudrate.
//

int Comm::getMaxBaud( void )
{
#ifdef B115200
  return(1152);
#elif defined(B57600)
  return(576);
#elif defined(B38400)
  return(384);
#elif defined(EXTB)
  return(384);
#elif defined(B19200)
  return(192);
#elif defined(EXTA)
  return(192);
#else
  return(96);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// DtrOn
//
// Turn on DTR.
//

void Comm::DtrOn()
{
  int f;
  f = TIOCM_DTR;
  ioctl( m_fd, TIOCMBIS, &f );
}

///////////////////////////////////////////////////////////////////////////////
// DtrOff
//
// Turn off DTR.
//

void Comm::DtrOff()
{
  int f;
  f = TIOCM_DTR;
  ioctl( m_fd, TIOCMBIC, &f );
}

///////////////////////////////////////////////////////////////////////////////
// RtsOn
//
// Turn on RTS.
//

void Comm::RtsOn()
{
  int f;
  f = TIOCM_RTS;
  ioctl( m_fd, TIOCMBIS, &f );
}

///////////////////////////////////////////////////////////////////////////////
// RtsOff
//
// Turn off RTS.
//

void Comm::RtsOff()
{
  int f;
  f = TIOCM_RTS;
  ioctl( m_fd, TIOCMBIC, &f );
}

///////////////////////////////////////////////////////////////////////////////
// isTransmitterEmpty
//
// get_lsr_info - get line status register info
//
// Let user call ioctl() to get info when the UART physically
// is emptied.  On bus types like RS485, the transmitter must
// release the bus after transmitting. This must be done when
// the transmit shift register is empty, not be done when the
// transmit holding register is empty.  This functionality
// allows an RS485 driver to be written in user space.
// TIOCSER_TEMT
//
// These are the definitions for the Line Status Register
//
//#define UART_LSR_TEMT	0x40	 Transmitter empty
//#define UART_LSR_THRE	0x20	 Transmit-hold-register empty

int Comm::isTransmitterEmpty()
{
  int rv;
  ioctl( m_fd, TIOCSERGETLSR, &rv );
  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// setParam
//
// Set port parameters.
//

void Comm::setParam( char *baud,
                     	 char *parity,
                     	 char *bits,
                     	 int HWFlow,
                     	 int SWFlow )
{
  int spd = -1;
  int newbaud;
  int bit = bits[0];
  struct termios tty;

  tcgetattr(m_fd, &tty);

  // We generate mark and space parity ourself.
  if ( bit == '7' && ( parity[0] == 'M' || parity[0] == 'S' ) ) {
    bit = '8';
  }

  // Check if 'baudr' is really a number
  if ( ( newbaud = ( atol(baud) / 100)) == 0 && baud[0] != '0')  {
    newbaud = -1;
  }

  switch( newbaud )  {
  case 0:
#ifdef B0
    spd = B0;
    break;
#else
    spd = 0;
    break;
#endif
  case 3:
    spd = B300;
    break;

  case 6:
    spd = B600;
    break;

  case 12:
    spd = B1200;
    break;
  case 24:
    spd = B2400;
    break;
  case 48:
    spd = B4800;
    break;
  case 96:
    spd = B9600;
    break;
#ifdef B19200
  case 192:
    spd = B19200;
    break;
#else /* B19200 */
#  ifdef EXTA
  case 192:
    spd = EXTA;
    break;
#   else /* EXTA */
  case 192:
    spd = B9600;
    break;
#   endif /* EXTA */
#endif	 /* B19200 */
#ifdef B38400
  case 384:
    spd = B38400;
    break;
#else /* B38400 */
#  ifdef EXTB
  case 384:
    spd = EXTB;
    break;
#   else /* EXTB */
  case 384:
    spd = B9600;
    break;
#   endif /* EXTB */
#endif	 /* B38400 */
#ifdef B57600
  case 576:
    spd = B57600;
    break;
#endif
#ifdef B115200
  case 1152:
    spd = B115200;
    break;
#endif
  } // Switch


  if (spd != -1)  {
    cfsetospeed( &tty, (speed_t)spd );
    cfsetispeed( &tty, (speed_t)spd );
  }

  switch (bit)  {
  case '5':
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS5;
    break;

  case '6':
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS6;
    break;

  case '7':
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS7;
    break;

  case '8':
  default:
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    break;
  }

  // Set into raw, no echo mode
  tty.c_iflag =  IGNBRK;
  tty.c_lflag = 0;
  tty.c_oflag = 0;
  tty.c_cflag |= CLOCAL | CREAD;
#ifdef _DCDFLOW
  tty.c_cflag &= ~CRTSCTS;
#endif
  tty.c_cc[VMIN] = 1;
  tty.c_cc[VTIME] = 5;

  if ( SWFlow ) {
    tty.c_iflag |= IXON | IXOFF;

  }
  else {
    tty.c_iflag &= ~(IXON|IXOFF|IXANY);
  }

  tty.c_cflag &= ~(PARENB | PARODD);
  if (parity[0] == 'E') {
    tty.c_cflag |= PARENB;
  }
  else if (parity[0] == 'O') {
    tty.c_cflag |= PARODD;
  }

  tcsetattr( m_fd, TCSANOW, &tty );
  RtsOn();

#ifndef _DCDFLOW
  setHWFlow( HWFlow );
#endif
}

///////////////////////////////////////////////////////////////////////////////
// msGettick
//--------------------------------------------------------------------------
// Get the current millisecond tick count.  Does not have to represent
// an actual time, it just needs to be an incrementing timer.
//

long Comm::msGettick(void)
{
  struct timezone tmzone;
  struct timeval  tmval;
  long ms;

  gettimeofday(&tmval,&tmzone);
  ms = (tmval.tv_sec & 0xFFFF) * 1000 + tmval.tv_usec / 1000;
  return ms;
}

///////////////////////////////////////////////////////////////////////////////
// msDelay
//--------------------------------------------------------------------------
//  Description:
//     Delay for at least 'len' ms
//

void Comm::msDelay(int len)
{
  struct timespec s;              // Set aside memory space on the stack

  s.tv_sec = len / 1000;
  s.tv_nsec = (len - (s.tv_sec * 1000)) * 1000000;
  nanosleep(&s, NULL);
}

///////////////////////////////////////////////////////////////////////////////
// drainInput

void Comm::drainInput( void )
{
  int cnt;
  char c;

  do {
    c = readChar( &cnt );
  } while (cnt && ( -1 != cnt) );
}

#endif
