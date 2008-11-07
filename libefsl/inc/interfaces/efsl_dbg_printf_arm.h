#ifndef debug_printf_arm_h_
#define debug_printf_arm_h_

extern void efsl_debug_devopen_arm( int(*put)(int) );
extern void efsl_debug_printf_arm( char const *format, ... );

#endif
