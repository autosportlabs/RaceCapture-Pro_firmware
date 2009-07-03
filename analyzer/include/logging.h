/*
 * logging.h
 *
 *  Created on: May 20, 2009
 *      Author: brent
 */
#include "wx/wxprec.h"
#include <string.h>

#ifndef LOGGING_H_
#define LOGGING_H_

#define __SFILE__ \
(strrchr(__FILE__,'/') \
? strrchr(__FILE__,'/')+1 \
: __FILE__ \
)

extern bool loggingVerbose;

#define VERBOSE_ENABLED loggingVerbose
#define VERBOSE(MSG) do { if (loggingVerbose){ \
		wxLogMessage("%s (%d): %s",__SFILE__, __LINE__, MSG.ToAscii()); \
		} \
	} while(0)

#define FMT(FORMAT, ARGS...) wxString::Format(FORMAT,ARGS)
#define ERROR(MSG) wxLogError(MSG)
#define INFO(MSG) wxLogMessage(MSG)

void EnableVerbose(bool enabled);

#endif /* LOGGING_H_ */
