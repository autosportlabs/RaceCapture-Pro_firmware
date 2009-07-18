#include "commonEvents.h"

#include <wx/arrimpl.cpp> // this is a magic incantation which must be done!

WX_DEFINE_OBJARRAY(DatalogChannelSelectionSet);


DEFINE_EVENT_TYPE ( OPTIONS_CHANGED_EVENT )
DEFINE_EVENT_TYPE ( CONFIG_CHANGED_EVENT )
DEFINE_EVENT_TYPE ( CONFIG_STALE_EVENT )
DEFINE_EVENT_TYPE ( WRITE_CONFIG_EVENT )
DEFINE_EVENT_TYPE ( ADD_NEW_LINE_CHART_EVENT )
