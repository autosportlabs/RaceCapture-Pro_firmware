/*
 * debug.h
 *
 *  Created on: Feb 3, 2014
 *      Author: stieg
 */

#ifndef __DEBUG_H__
#define __DEBUG_H__

//#define RCP_INFO
#ifdef RCP_INFO
#include <stdio.h>
#define INFO(s, ...) printf(s, ##__VA_ARGS__)
#else
#define INFO(s, ...)
#endif // RCP_INFO

//#define RCP_DEBUG
#ifdef RCP_DEBUG
#include <stdio.h>
#define DEBUG(s, ...) printf(s, ##__VA_ARGS__)
#else
#define DEBUG(s, ...)
#endif // RCP_DEBUG

//#define RCP_DEVEL
#ifdef RCP_DEVEL
#include <stdio.h>
#define DEVEL(s, ...) printf(s, ##__VA_ARGS__)
#else
#define DEVEL(s, ...)
#endif // RCP_DEVEL

#endif // __DEBUG_H__
