#ifndef COMMON_EVENTS_H_
#define COMMON_EVENTS_H_

#include "wx/wxprec.h"

class DatalogChannelSelection {

public:
	DatalogChannelSelection(int newDatalogId, wxArrayInt newChannelIds) : datalogId(newDatalogId), channelIds(newChannelIds)
	{}

	int datalogId;
	wxArrayInt channelIds;
};


WX_DECLARE_OBJARRAY(DatalogChannelSelection,DatalogChannelSelectionSet);

DECLARE_EVENT_TYPE ( OPTIONS_CHANGED_EVENT, -1 )
DECLARE_EVENT_TYPE ( CONFIG_CHANGED_EVENT, -1 )
DECLARE_EVENT_TYPE ( CONFIG_STALE_EVENT, -1 )
DECLARE_EVENT_TYPE ( WRITE_CONFIG_EVENT, -1 )
DECLARE_EVENT_TYPE ( ADD_NEW_LINE_CHART_EVENT, -1 )
DECLARE_EVENT_TYPE ( ADD_NEW_ANALOG_GAUGE_EVENT, -1 )
DECLARE_EVENT_TYPE ( ADD_NEW_DIGITAL_GAUGE_EVENT, -1 )

enum{
		CONFIG_CHANGED 	= wxID_HIGHEST + 80000,
		OPTIONS_CHANGED,
		CONFIG_STALE,
		WRITE_CONFIG,
		ADD_NEW_LINE_CHART,
		ADD_NEW_ANALOG_GAUGE,
		ADD_NEW_DIGITAL_GAUGE
};

#endif /*EVENTS_H_*/
